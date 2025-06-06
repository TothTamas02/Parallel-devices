#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <time.h>

#define CHECK_CL_ERROR(cmd) { \
		cl_int error = cmd; \
		if (error != CL_SUCCESS) { \
				fprintf(stderr, "OpenCL error %d at %s:%d\n", error, __FILE__, __LINE__); \
				exit(1); \
		} \
}

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

int* generate_random_input(int size, float missing_prob) {
	int* arr = (int*)malloc(size * sizeof(int));
	srand(time(NULL));

	for (int i = 0; i < size; i++) {
		arr[i] = rand() % 100 + 1;
	}

	for (int i = 1; i < size-1; i++) {
		if ((float)rand() / (float)RAND_MAX < missing_prob) {
			if(arr[i-1] > 0 && arr[i+1] > 0) {
				arr[i] = 0;
				i++;
			}
		}
	}

	return arr;
}

int main() {
	const int n = 20;
	int* input = generate_random_input(n, 0.2);
	int* output = (int*)malloc(n * sizeof(int));

	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem input_buffer, output_buffer;

	CHECK_CL_ERROR(clGetPlatformIDs(1, &platform, NULL));
	CHECK_CL_ERROR(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));

	context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
	queue = clCreateCommandQueue(context, device, 0, NULL);

	program = load_kernel(context, device, "fix_missing.cl");
	kernel = clCreateKernel(program, "fix_missing", NULL);

	input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, n * sizeof(int), NULL, NULL);
	output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, n * sizeof(int), NULL, NULL);

	CHECK_CL_ERROR(clEnqueueWriteBuffer(queue, input_buffer, CL_TRUE, 0, n * sizeof(int), input, 0, NULL, NULL));

	CHECK_CL_ERROR(clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer));
	CHECK_CL_ERROR(clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer));
	CHECK_CL_ERROR(clSetKernelArg(kernel, 2, sizeof(int), &n));

	size_t global_size = n;
	CHECK_CL_ERROR(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL));

	CHECK_CL_ERROR(clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, n * sizeof(int), output, 0, NULL, NULL));

	printf("Input: ");
	for (int i = 0; i < n; i++) {
		printf("%d ", input[i]);
	}
	printf("\nOutput: ");
	for (int i = 0; i < n; i++) {
		printf("%d ", output[i]);
	}

	free(input);
	free(output);

	clReleaseMemObject(input_buffer);
	clReleaseMemObject(output_buffer);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return 0;
}
