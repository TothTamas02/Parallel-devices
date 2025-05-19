#include "filters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include <time.h>
#include <sys/time.h>

#define KERNEL_FILE "../image_filter/kernels.cl"
#define MAX_SOURCE_SIZE (0x100000)

void to_grayscale(const unsigned char *input, unsigned char *output, int width, int height, int channels) {
    for (int i = 0; i < width * height; i++) {
        int r = input[i * channels];
        int g = input[i * channels + 1];
        int b = input[i * channels + 2];
        output[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
    }
}

void run_filter(unsigned char *input, unsigned char *output, int width, int height, int channels, const char *filter, const char *device_type_str, const char *input_file) {
    FILE *fp = fopen(KERNEL_FILE, "r");
if (!fp) {
    fprintf(stderr, "Failed to load kernel: %s\n", KERNEL_FILE);
    perror("fopen");
    exit(1);
}

    char *source_str = (char *)malloc(MAX_SOURCE_SIZE);
    size_t source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
		printf("Kernel loaded (%zu bytes): %.30s...\n", source_size, source_str);
    fclose(fp);

    cl_device_type device_type = !strcmp(device_type_str, "gpu") ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU;

    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem input_buffer, output_buffer;

    cl_int ret = clGetPlatformIDs(1, &platform_id, NULL);
		ret = clGetDeviceIDs(platform_id, device_type, 1, &device_id, NULL);
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    queue = clCreateCommandQueue(context, device_id, 0, &ret);

    program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

		ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    kernel = clCreateKernel(program, filter, &ret);

    size_t img_size = width * height * sizeof(unsigned char);
    size_t size = width * height * channels * sizeof(unsigned char);

    unsigned char *gray_input = NULL;
    int is_sobel = !strcmp(filter, "sobel");

    if (is_sobel) {
        gray_input = (unsigned char *)malloc(img_size * 3);
        to_grayscale(input, gray_input, width, height, channels);
        input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, img_size * sizeof(unsigned char), gray_input, &ret);
        output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, img_size * sizeof(unsigned char), NULL, &ret);
    } else {
        input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size * sizeof(unsigned char), input, &ret);
        output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size * sizeof(unsigned char), NULL, &ret);
    }

    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
    ret |= clSetKernelArg(kernel, 2, sizeof(int), &width);
    ret |= clSetKernelArg(kernel, 3, sizeof(int), &height);

    size_t global_work_size[2] = {width, height};

    struct timeval start, end;
    gettimeofday(&start, NULL);

    ret = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    clFinish(queue);

    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    if (is_sobel) {
        ret = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, img_size, gray_input, 0, NULL, NULL);
        for (int i = 0; i < img_size; i++) {
            output[i * 3] = gray_input[i];
            output[i * 3 + 1] = gray_input[i];
            output[i * 3 + 2] = gray_input[i];
        }
        free(gray_input);
    } else {
        ret = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, size, output, 0, NULL, NULL);
    }

    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(source_str);

	FILE *log_check = fopen("../measurements.csv", "r");
		int file_exists = (log_check != NULL);
		if (log_check) {
			fclose(log_check);
		}

	const char *filename_only = strrchr(input_file, '/');
	if (filename_only) {
    filename_only++;
	} else {
    filename_only = input_file;
	}

  FILE *log = fopen("../measurements.csv", "a");
    if (log) {
        if (!file_exists) {
            fprintf(log, "input_file,filter,device,width,height,elapsed_time(ms)\n");
        }
        fprintf(log, "%s,%s,%s,%d,%d,%.10f\n", filename_only, filter, device_type_str, width, height, elapsed_time);
        fclose(log);
    }
}
