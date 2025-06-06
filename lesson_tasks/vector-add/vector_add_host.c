#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define VECTOR_SIZE 1024

void check_error(cl_int err, const char *name) {
  if (err != CL_SUCCESS) {
    fprintf(stderr, "%s failed with error code %d\n", name, err);
    exit(1);
  }
}

int main() {
  cl_int err;

  float A[VECTOR_SIZE], B[VECTOR_SIZE], C[VECTOR_SIZE];

  for (int i = 0; i < VECTOR_SIZE; i++) {
    A[i] = (float)i;
    B[i] = (float)(VECTOR_SIZE - i);
  }

  cl_platform_id platform;
  clGetPlatformIDs(1, &platform, NULL);

  cl_device_id device;
  clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  check_error(err, "clCreateContext");

  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
  check_error(err, "clCreateCommandQueue");

  FILE* fp = fopen("vector_add.cl", "r");
  if(!fp) {
    perror("Failed to open OpenCL source file");
    exit(1);
  }

  fseek(fp, 0, SEEK_END);
  size_t program_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* program_source = (char*)malloc(program_size + 1);
  fread(program_source, 1, program_size, fp);
  fclose(fp);
  program_source[program_size] = '\0';

  cl_program program = clCreateProgramWithSource(context, 1, (const char**)&program_source, &program_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    size_t log_size;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char* build_log = (char*)malloc(log_size + 1);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
    build_log[log_size] = '\0';
    printf("Build log:\n%s\n", build_log);
    free(build_log);
    exit(1);
  }

  cl_kernel kernel = clCreateKernel(program, "vector_add", &err);
  check_error(err, "clCreateKernel");

  cl_mem buffer_A = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * VECTOR_SIZE, NULL, &err);
  check_error(err, "clCreateBuffer A");

  cl_mem buffer_B = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * VECTOR_SIZE, NULL, &err);
  check_error(err, "clCreateBuffer B");

  cl_mem buffer_C = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * VECTOR_SIZE, NULL, &err);
  check_error(err, "clCreateBuffer C");

  err = clEnqueueWriteBuffer(queue, buffer_A, CL_TRUE, 0, sizeof(float) * VECTOR_SIZE, A, 0, NULL, NULL);
  check_error(err, "clEnqueueWriteBuffer A");

  err = clEnqueueWriteBuffer(queue, buffer_B, CL_TRUE, 0, sizeof(float) * VECTOR_SIZE, A, 0, NULL, NULL);
  check_error(err, "clEnqueueWriteBuffer B");

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_A);
  check_error(err, "clSetKernelArg A");

  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_B);
  check_error(err, "clSetKernelArg B");

  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_C);
  check_error(err, "clSetKernelArg C");

  unsigned int vec_size = VECTOR_SIZE;
  err = clSetKernelArg(kernel, 3, sizeof(unsigned int), (void*)&vec_size);
  check_error(err, "clSetKernelArg N");

  size_t global_work_size = VECTOR_SIZE;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
  check_error(err, "clEnqueueNDRangeKernel");

  err = clEnqueueReadBuffer(queue, buffer_C, CL_TRUE, 0, sizeof(float) * VECTOR_SIZE, C, 0, NULL, NULL);
  check_error(err, "clEnqueueReadBuffer");

  for (int i = 0; i < VECTOR_SIZE; i++){
    printf("C[%d] = %f\n", i, C[i]);
  }

  clReleaseMemObject(buffer_A);
  clReleaseMemObject(buffer_B);
  clReleaseMemObject(buffer_C);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  return 0;
}
