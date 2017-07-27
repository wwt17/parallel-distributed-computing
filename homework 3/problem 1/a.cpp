#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CL/cl.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

const int MAX_KERNEL_SOURCE_SIZE = 0x10000;

void read_matrix(const char *filename, float *&a, int &m, int &n) {
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		eprintf("Failed to open %s\n", filename);
		exit(1);
	}
	fread(&m, sizeof(int), 1, f);
	fread(&n, sizeof(int), 1, f);
	a = new float[m * n];
	fread(a, sizeof(float), m * n, f);
	fclose(f);
}

void print_matrix(const float *a, int m, int n) {
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			printf(" %f", a[i * n + j]);
		}
		printf("\n");
	}
}

int main() {
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;

	FILE *kernel_source_file;
	char kernel_source_file_name[] = "./kernel.cl";
	char *kernel_source_code;
	size_t kernel_source_size;

	int m, n, m_, n_;
	float *A, *b, *c;
	cl_mem mem_A, mem_b, mem_c;
	
	// Load the source code containing the kernel
	kernel_source_file = fopen(kernel_source_file_name, "r");
	if (kernel_source_file == NULL) {
		eprintf("Failed to load kernel.\n");
		exit(1);
	}
	kernel_source_code = new char[MAX_KERNEL_SOURCE_SIZE];
	kernel_source_size = fread(kernel_source_code, 1, MAX_KERNEL_SOURCE_SIZE, kernel_source_file);
	fclose(kernel_source_file);

	// Get Platform and Device Info
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

	// Create OpenCL context
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

	// Create Command Queue
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	// Read Matrices
	read_matrix("matrix_A", A, m, n);
	read_matrix("matrix_b", b, m_, n_);
	eprintf("m = %d n = %d\n", m, n);

	// Create Memory Buffer
	mem_A = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, m * n * sizeof(float), A, &ret);
	mem_b = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, n * sizeof(float), b, &ret);
	mem_c = clCreateBuffer(context, CL_MEM_READ_WRITE, m * sizeof(float), NULL, &ret);
	if (mem_A == NULL || mem_b == NULL || mem_c == NULL)
		eprintf("error in clCreateBuffer\n");

	// Create Kernel Program from the source
	program = clCreateProgramWithSource(context, 1, (const char**)&kernel_source_code, &kernel_source_size, &ret);

	// Build Kernel Program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	// Create OpenCL Kernel
	kernel = clCreateKernel(program, "matrixVectorMul", &ret);

	// Set OpenCL Kernel Parameters
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_A);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_b);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), &mem_c);
	ret = clSetKernelArg(kernel, 3, sizeof(int), &n);

	// Execute OpenCL Kernel
	const size_t globalWorkSize[] = {m, 0, 0};
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);

	// Copy results from the memory buffer
	c = new float[m];
	ret = clEnqueueReadBuffer(command_queue, mem_c, CL_TRUE, 0, m * sizeof(float), c, 0, NULL, NULL);

	// Display Result
	print_matrix(c, m, 1);

	// Finalization
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(mem_A);
	ret = clReleaseMemObject(mem_b);
	ret = clReleaseMemObject(mem_c);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	delete[] kernel_source_code;
	delete[] c;
}
