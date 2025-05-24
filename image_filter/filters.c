#include "filters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include <time.h>
#include <sys/time.h>

#define MAX_SOURCE_SIZE (0x100000)

void to_grayscale(const unsigned char *input, unsigned char *output, int width, int height, int channels)
{
    for (int i = 0; i < width * height; i++)
    {
        int r = input[i * channels];
        int g = input[i * channels + 1];
        int b = input[i * channels + 2];
        output[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
    }
}

void run_filter(unsigned char *input, unsigned char *output, int width, int height, int channels, const char *filter, const int platform_id, const char *input_file)
{
    const char *KERNEL_FILE = "image_filter/kernels.cl";
    FILE *fp = fopen(KERNEL_FILE, "r");
    if (!fp)
    {
        KERNEL_FILE = "../image_filter/kernels.cl";
        fp = fopen(KERNEL_FILE, "r");
    }
    if (!fp)
    {
        fprintf(stderr, "Failed to load kernel: %s\n", KERNEL_FILE);
        perror("fopen");
        exit(1);
    }

    char *source_str = (char *)malloc(MAX_SOURCE_SIZE);
    size_t source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem input_buffer, output_buffer;

    cl_int used_platform;
    cl_int deviceID;
    cl_int ret;
    cl_int found_device = 0;
    char device_name[256] = "Unknown";
    cl_device_id device_id = NULL;

    // Get available platforms
    cl_uint num_platforms;
    cl_platform_id platform_ids[10];
    ret = clGetPlatformIDs(10, platform_ids, &num_platforms);
    if (ret != CL_SUCCESS || num_platforms == 0)
    {
        fprintf(stderr, "Failed to get OpenCL platforms\n");
        free(source_str);
        exit(1);
    }

    printf("Available platforms: %d\n", num_platforms);
    for (cl_uint i = 0; i < num_platforms; i++)
    {
        char platform_name[256];
        clGetPlatformInfo(platform_ids[i], CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
        printf("Platform %d: %s\n", i, platform_name);
    }

    // Use requested platform if valid
    cl_platform_id selected_platform;
    if (platform_id >= 0 && platform_id < num_platforms)
    {
        selected_platform = platform_ids[platform_id];

        char platform_name[256];
        clGetPlatformInfo(selected_platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
        printf("Using platform %d: %s\n", platform_id, platform_name);

        // Get devices on this platform
        cl_uint num_devices;
        cl_device_id devices[10];
        ret = clGetDeviceIDs(selected_platform, CL_DEVICE_TYPE_GPU, 10, devices, &num_devices);

        if (ret == CL_SUCCESS && num_devices > 0)
        {
            device_id = devices[0]; // Use first GPU on this platform
            found_device = 1;
            clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
            printf("Using device: %s\n", device_name);
        }
        else
        {
            fprintf(stderr, "No GPU devices found on platform %d\n", platform_id);
        }
    }
    else
    {
        fprintf(stderr, "Invalid platform ID: %d (must be 0-%d)\n", platform_id, num_platforms - 1);
        free(source_str);
        exit(1);
    }
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to create OpenCL context\n");
        exit(1);
    }
    queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to create command queue\n");
        clReleaseContext(context);
        exit(1);
    }

    program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to create program with source\n");
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        free(source_str);
        exit(1);
    }

    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to build program\n");
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        free(source_str);
        exit(1);
    }

    kernel = clCreateKernel(program, filter, &ret);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to create kernel\n");
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        free(source_str);
        exit(1);
    }

    size_t img_size = width * height * sizeof(unsigned char);
    size_t size = width * height * channels * sizeof(unsigned char);

    unsigned char *gray_input = NULL;
    int is_sobel = !strcmp(filter, "sobel");

    if (is_sobel)
    {
        gray_input = (unsigned char *)malloc(img_size * 3);
        to_grayscale(input, gray_input, width, height, channels);
        input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, img_size * sizeof(unsigned char), gray_input, &ret);
        if (ret != CL_SUCCESS)
        {
            fprintf(stderr, "Failed to create input buffer\n");
            free(gray_input);
            clReleaseKernel(kernel);
            clReleaseProgram(program);
            clReleaseCommandQueue(queue);
            clReleaseContext(context);
            free(source_str);
            exit(1);
        }
        output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, img_size * sizeof(unsigned char), NULL, &ret);
        if (ret != CL_SUCCESS)
        {
            fprintf(stderr, "Failed to create output buffer\n");
            free(gray_input);
            clReleaseMemObject(input_buffer);
            clReleaseKernel(kernel);
            clReleaseProgram(program);
            clReleaseCommandQueue(queue);
            clReleaseContext(context);
            free(source_str);
            exit(1);
        }
    }
    else
    {
        input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size * sizeof(unsigned char), input, &ret);
        if (ret != CL_SUCCESS)
        {
            fprintf(stderr, "Failed to create input buffer\n");
            clReleaseKernel(kernel);
            clReleaseProgram(program);
            clReleaseCommandQueue(queue);
            clReleaseContext(context);
            free(source_str);
            exit(1);
        }
        output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size * sizeof(unsigned char), NULL, &ret);
        if (ret != CL_SUCCESS)
        {
            fprintf(stderr, "Failed to create output buffer\n");
            clReleaseMemObject(input_buffer);
            clReleaseKernel(kernel);
            clReleaseProgram(program);
            clReleaseCommandQueue(queue);
            clReleaseContext(context);
            free(source_str);
            exit(1);
        }
    }

    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to set kernel argument 0\n");
        clReleaseMemObject(input_buffer);
        clReleaseMemObject(output_buffer);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        free(source_str);
        exit(1);
    }
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to set kernel argument 1\n");
        clReleaseMemObject(input_buffer);
        clReleaseMemObject(output_buffer);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        free(source_str);
        exit(1);
    }
    ret |= clSetKernelArg(kernel, 2, sizeof(int), &width);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to set kernel argument 2\n");
        clReleaseMemObject(input_buffer);
        clReleaseMemObject(output_buffer);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        free(source_str);
        exit(1);
    }
    ret |= clSetKernelArg(kernel, 3, sizeof(int), &height);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to set kernel argument 3\n");
        clReleaseMemObject(input_buffer);
        clReleaseMemObject(output_buffer);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        free(source_str);
        exit(1);
    }

    size_t global_work_size[2] = {width, height};

    struct timeval start, end;
    gettimeofday(&start, NULL);

    ret = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to enqueue kernel\n");
        clReleaseMemObject(input_buffer);
        clReleaseMemObject(output_buffer);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        free(source_str);
        exit(1);
    }
    clFinish(queue);

    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    if (is_sobel)
    {
        ret = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, img_size, gray_input, 0, NULL, NULL);
        if (ret != CL_SUCCESS)
        {
            fprintf(stderr, "Failed to read output buffer\n");
            free(gray_input);
            clReleaseMemObject(input_buffer);
            clReleaseMemObject(output_buffer);
            clReleaseKernel(kernel);
            clReleaseProgram(program);
            clReleaseCommandQueue(queue);
            clReleaseContext(context);
            free(source_str);
            exit(1);
        }
        for (int i = 0; i < img_size; i++)
        {
            output[i * 3] = gray_input[i];
            output[i * 3 + 1] = gray_input[i];
            output[i * 3 + 2] = gray_input[i];
        }
        free(gray_input);
    }
    else
    {
        ret = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, size, output, 0, NULL, NULL);
        if (ret != CL_SUCCESS)
        {
            fprintf(stderr, "Failed to read output buffer\n");
            clReleaseMemObject(input_buffer);
            clReleaseMemObject(output_buffer);
            clReleaseKernel(kernel);
            clReleaseProgram(program);
            clReleaseCommandQueue(queue);
            clReleaseContext(context);
            free(source_str);
            exit(1);
        }
    }

    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(source_str);

    FILE *log_check = fopen("measurements.csv", "r");
    if (!log_check)
    {
        log_check = fopen("../measurements.csv", "r");
    }
    int file_exists = (log_check != NULL);
    if (log_check)
    {
        fclose(log_check);
    }

#ifdef __linux__
#define delimeter '/'
#elif __APPLE__
#define delimeter '/'
#else
#define delimeter '\\'
#endif

    const char *filename_only = strrchr(input_file, delimeter);
    if (filename_only)
    {
        filename_only++;
    }
    else
    {
        filename_only = input_file;
    }

    const char *log_path = "measurements.csv";
    FILE *log = fopen(log_path, "a");
    if (!log)
    {
        log_path = "../measurements.csv";
        log = fopen(log_path, "a");
    }
    if (log)
    {
        if (!file_exists)
        {
            fprintf(log, "input_file,filter,device_name,width,height,elapsed_time(ms)\n");
        }
        fprintf(log, "%s,%s,%s,%d,%d,%.10f\n", filename_only, filter, device_name, width, height, elapsed_time);
        fclose(log);
    }
}
