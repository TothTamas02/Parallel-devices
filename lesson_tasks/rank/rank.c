#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CL/cl.h>

#define SIZE 10

cl_program load_kernel(cl_context context, cl_device_id device, const char* filename) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "Failed to open kernel file %s\n", filename);
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	size_t source_size = ftell(fp);
	rewind(fp);
	char *source = (char*)malloc(source_size + 1);
	source[source_size] = '\0';
	fread(source, 1, source_size, fp);
	fclose(fp);

	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&source, &source_size, NULL);
	cl_int build_status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

	if (build_status != CL_SUCCESS) {
		char build_log[4096];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, NULL);
		fprintf(stderr, "Kernel build failed: %s\n", build_log);
		exit(1);
	}

	free(source);
	return program;
}

int main() {
	int input[SIZE];
	int output[SIZE];
	int size = SIZE;

	srand(time(NULL));
	for (int i = 0; i < size; i++) {
		input[i] = rand() % 100 + 1;
	}

	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem input_buffer, output_buffer;
	cl_int err;

  err = clGetPlatformIDs(1, &platform, NULL);
  if (err != CL_SUCCESS) {
      fprintf(stderr, "Platform error: %d\n", err);
      exit(1);
  }

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if (err != CL_SUCCESS) {
      fprintf(stderr, "Device error: %d\n", err);
      exit(1);
  }

  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  if (err != CL_SUCCESS) {
      fprintf(stderr, "Context error: %d\n", err);
      exit(1);
	}

  queue = clCreateCommandQueue(context, device, 0, &err);
  if (err != CL_SUCCESS) {
      fprintf(stderr, "Command queue error: %d\n", err);
      exit(1);
  }

	input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size * sizeof(int), input, NULL);
	output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size * sizeof(int), NULL, NULL);

	program = load_kernel(context, device, "rank.cl");

	cl_int kernel_err;
	kernel = clCreateKernel(program, "rank", &kernel_err);
	if (kernel_err != CL_SUCCESS || !kernel) {
		  fprintf(stderr, "Kernel creation failed: %d\n", kernel_err);
			exit(1);
	}
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
	clSetKernelArg(kernel, 2, sizeof(int), &size);
	if (err != CL_SUCCESS) {
        fprintf(stderr, "Argument setting failed: %d\n", err);
        exit(1);
    }

	size_t global_work_size = size;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
  if (err != CL_SUCCESS) {
      fprintf(stderr, "Kernel execution failed: %d\n", err);
      exit(1);
  }

  err = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, sizeof(int) * SIZE, output, 0, NULL, NULL);
  if (err != CL_SUCCESS) {
      fprintf(stderr, "Buffer read failed: %d\n", err);
      exit(1);
  }

	printf("Generated input:\n");
	for (int i = 0; i < size; i++) {
		printf("%3d ", input[i]);
	}

	printf("\n\nCalculated ranks:\n");
	for (int i = 0; i < size; i++) {
		printf("%3d ", output[i]);
	}

	clReleaseMemObject(input_buffer);
	clReleaseMemObject(output_buffer);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return 0;
}
