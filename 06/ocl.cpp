#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

struct OCL_Error
{
	const char *call;
	cl_int error;
};

struct OCL_Context
{
	cl_context ctx;
	cl_command_queue queue;

	cl_device_id *devices;
	size_t device_count;

	OCL_Error *errors;
	size_t error_count;
	size_t error_capacity;
};

inline void ocl_error(OCL_Context *ocl, const char *call, cl_int error)
{
	if (error) {
		if (ocl->error_count < ocl->error_capacity) {
			OCL_Error err = { call, error };
			ocl->errors[ocl->error_count++] = err;
		}
	}
}

OCL_Context *ocl_make_context()
{
	cl_int error;

	cl_platform_id platforms[16];
	cl_uint platform_count;

	clGetPlatformIDs(16, platforms, &platform_count);
	for (cl_uint platform_i = 0; platform_i < platform_count; platform_i++) {
		cl_platform_id platform_id = platforms[platform_i];

		cl_device_id *devices;
		cl_uint device_count;

		clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 0, 0, &device_count);
		devices = (cl_device_id*)malloc(sizeof(cl_device_id) * device_count);
		clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, device_count, devices, 0);

		cl_context_properties properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
			0, 0,
		};

		cl_context context = clCreateContext(properties, device_count, devices, 0, 0, 0);
		if (context) {
			OCL_Context *oc = (OCL_Context*)malloc(sizeof(OCL_Context));

			oc->error_count = 0;
			oc->error_capacity = 256;
			oc->errors = (OCL_Error*)malloc(sizeof(OCL_Error) * oc->error_capacity);

			oc->ctx = context;

			oc->devices = devices;
			oc->device_count = device_count;

			oc->queue = clCreateCommandQueue(oc->ctx, oc->devices[0], 0, &error);
			ocl_error(oc, "clCreateCommandQueue", error);
			return oc;
		}
	}

	return 0;
}

cl_program ocl_compile(OCL_Context *ocl, const char *source, size_t length, const char *options,
		char *log, size_t log_size)
{
	cl_int error;

	cl_program program = clCreateProgramWithSource(ocl->ctx, 1, (const char**)&source, &length, &error);
	ocl_error(ocl, "clCreateProgramWithSource", error);
	if (!program) return program;

	error = clBuildProgram(program, 0, 0, options, 0, 0);
	ocl_error(ocl, "clBuildProgram", error);

	// TODO: Multiple device logging
	if (log) {
		error = clGetProgramBuildInfo(program, ocl->devices[0], CL_PROGRAM_BUILD_LOG,
			log_size, log, 0);
		ocl_error(ocl, "clGetProgramBuildInfo", error);
	}

	return program;
}

cl_program ocl_compile_file(OCL_Context *ocl, const char *filename, const char *options,
		char *log, size_t log_size)
{
	FILE *file;
	size_t size;
	char *source;

	file = fopen(filename, "rb");
	if (!file) {
		ocl_error(ocl, "fopen", errno);
		return 0;
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);

	source = (char*)malloc(size);
	fread(source, 1, size, file);
	fclose(file);

	cl_program program = ocl_compile(ocl, source, size, options, log, log_size);
	free(source);

	return program;
}

cl_kernel ocl_create_kernel(OCL_Context *ocl, cl_program program, const char *kernel_name)
{
	cl_int error;
	cl_kernel kernel = clCreateKernel(program, kernel_name, &error);
	ocl_error(ocl, "clCreateKernel", error);
	return kernel;
}

void ocl_enqueue_kernel_1d(OCL_Context *ocl, cl_kernel kernel, size_t offset, size_t size)
{
	cl_int error = clEnqueueNDRangeKernel(ocl->queue, kernel, 1, &offset, &size, 0, 0, 0, 0);
	ocl_error(ocl, "clEnqueueNDRangeKernel", error);
}

void ocl_enqueue_kernel_2d(OCL_Context *ocl, cl_kernel kernel,
		size_t offsetX, size_t offsetY, size_t sizeX, size_t sizeY)
{
	size_t offsets[] = { offsetX, offsetY };
	size_t sizes[] = { sizeX, sizeY };

	cl_int error = clEnqueueNDRangeKernel(ocl->queue, kernel, 2, offsets, sizes, 0, 0, 0, 0);
	ocl_error(ocl, "clEnqueueNDRangeKernel", error);
}

void ocl_blocking_read(OCL_Context *ocl, cl_mem buffer, size_t offset, size_t size, void *data)
{
	cl_int error = clEnqueueReadBuffer(ocl->queue, buffer, 1, offset, size, data, 0, 0, 0);
	ocl_error(ocl, "clEnqueueReadBuffer", error);
}

void ocl_arg_buffer(OCL_Context *ocl, cl_kernel kernel, int argument, cl_mem buffer)
{
	cl_int error = clSetKernelArg(kernel, argument, sizeof(cl_mem), &buffer);
	ocl_error(ocl, "clSetKernelArg", error);
}

void ocl_arg_int(OCL_Context *ocl, cl_kernel kernel, int argument, cl_int value)
{
	cl_int error = clSetKernelArg(kernel, argument, sizeof(cl_int), &value);
	ocl_error(ocl, "clSetKernelArg", error);
}

cl_mem ocl_create_buffer(OCL_Context *ocl, cl_mem_flags flags, size_t size, void *host_ptr)
{
	cl_int error;
	cl_mem buffer = clCreateBuffer(ocl->ctx, flags, size, host_ptr, &error);
	ocl_error(ocl, "clCreateBuffer", error);
	return buffer;
}

