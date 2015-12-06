#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

struct OCL_Context
{
	cl_context ctx;

	cl_device_id *devices;
	size_t device_count;
};

OCL_Context *ocl_make_context()
{
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
			oc->ctx = context;
			oc->devices = devices;
			oc->device_count = device_count;
			return oc;
		}
	}

	return 0;
}

cl_program compile_file(OCL_Context *ocl, const char *filename,
		const char *options, cl_int *error)
{
	FILE *file;
	size_t size;
	char *source;
	cl_program program;
	cl_int build_error;

	file = fopen(filename, "rb");
	if (!file) {
		*error = 1;
		return 0;
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);

	source = (char*)malloc(size);
	fread(source, 1, size, file);
	fclose(file);

	program = clCreateProgramWithSource(ocl->ctx, 1, (const char**)&source, &size, error);
	if (!program) return 0;

	build_error = clBuildProgram(program, 0, 0, options, 0, 0);

	if (build_error) {
		char *log = (char*)malloc(1024*1024);
		for (size_t device_i = 0; device_i < ocl->device_count; device_i++) {
			clGetProgramBuildInfo(program, ocl->devices[device_i], CL_PROGRAM_BUILD_LOG,
				1024*1024, log, 0);
			puts(log);
		}
		free(log);

		if (error) *error = build_error;
		return 0;
	}

	return program;
}

void CL_CALLBACK print_cl_error(const char *error, const void *data,
		size_t cb, void *user)
{
	printf("CL error: %s\n", error);
}

void log_start(const char *section)
{
	printf("%s\n", section);
}

int log_error(int error)
{
	if (error) {
		printf("> failed %d\n", error);
		exit(1);
	} else {
	}
	return error;
}

cl_int enqueue_light_kernel(cl_command_queue queue, cl_kernel kernel,
		int minX, int minY, int maxX, int maxY)
{
	clSetKernelArg(kernel, 1, sizeof(cl_int), &minX);
	clSetKernelArg(kernel, 2, sizeof(cl_int), &minY);
	clSetKernelArg(kernel, 3, sizeof(cl_int), &maxX);
	clSetKernelArg(kernel, 4, sizeof(cl_int), &maxY);

	const size_t global_offsets[] = { minX / 4, minY / 4 };
	const size_t global_sizes[] = {
		(maxX + 3) / 4 - global_offsets[0],
		(maxY + 3) / 4 - global_offsets[1],
	 };

	return clEnqueueNDRangeKernel(queue, kernel,
			2, global_offsets, global_sizes,
			0, 0, 0, 0);
}

int main(int argc, char **argv)
{
	cl_int error;
	log_start("Initializing");
	OCL_Context *ocl = ocl_make_context();
	log_error(ocl ? 0 : 1);

	for (size_t i = 0; i < ocl->device_count; i++) {
		char device_name[128];
		clGetDeviceInfo(ocl->devices[i], CL_DEVICE_NAME, 128, device_name, 0);
		printf("> %s\n", device_name);
	}

	log_start("Creating command queue");
	cl_command_queue queue = clCreateCommandQueue(ocl->ctx,
		ocl->devices[0], 0, &error);
	log_error(error);


	log_start("Compiling");
	cl_program program = compile_file(ocl, "part1_opencl.c", "", &error);
		log_error(error);

	log_start("Creating kernels");

	cl_kernel k_set_zero = clCreateKernel(program, "set_zero", &error);
	cl_kernel k_count_lights = clCreateKernel(program, "count_lights", &error);

	cl_kernel k_turn_on = clCreateKernel(program, "turn_on", &error);
	cl_kernel k_turn_off = clCreateKernel(program, "turn_off", &error);
	cl_kernel k_toggle = clCreateKernel(program, "toggle", &error);
	cl_kernel light_kernels[] = { k_turn_on, k_turn_off, k_toggle };

	log_error(error);

	int width = 1000;
	int height = 1000;

	int x_cells = (width + 3) / 4;
	int y_cells = (height + 3) / 4;

	log_start("Creating buffer");
	cl_mem buffer = clCreateBuffer(ocl->ctx,
			CL_MEM_READ_WRITE,
			sizeof(cl_ushort) * x_cells * y_cells, 0,
			&error);
	log_error(error);

	for (int i = 0; i < 3; i++) {
		clSetKernelArg(light_kernels[i], 0, sizeof(cl_mem), &buffer);
		clSetKernelArg(light_kernels[i], 5, sizeof(cl_int), &x_cells);
	}
	clSetKernelArg(k_set_zero, 0, sizeof(cl_mem), &buffer);
	clSetKernelArg(k_count_lights, 0, sizeof(cl_mem), &buffer);

	log_start("Starting kernel");

	size_t total_cells = x_cells * y_cells;
	error = clEnqueueNDRangeKernel(queue, k_set_zero,
			1, 0, &total_cells, 0, 0, 0, 0);
	log_error(error);

	while (!feof(stdin)) {
		char line[256];
		int minX, minY, maxX, maxY;
		cl_kernel kernel = 0;

		if (!fgets(line, sizeof(line), stdin))
			continue;

		if (sscanf(line, "turn on %d,%d through %d,%d",
					&minX, &minY, &maxX, &maxY)) {
			kernel = k_turn_on;
		} else if (sscanf(line, "toggle %d,%d through %d,%d",
					&minX, &minY, &maxX, &maxY)) {
			kernel = k_toggle;
		} else if (sscanf(line, "turn off %d,%d through %d,%d",
					&minX, &minY, &maxX, &maxY)) {
			kernel = k_turn_off;
		}

		if (minX < 0) minX = 0;
		if (minY < 0) minY = 0;
		if (maxX > width - 1) minX = width - 1;
		if (maxY > height - 1) minY = height - 1;

		if (kernel) {
			error = enqueue_light_kernel(queue, kernel,
					minX, minY, maxX + 1, maxY + 1);
			log_error(error);
		}
	}

	error = clEnqueueNDRangeKernel(queue, k_count_lights,
			1, 0, &total_cells, 0, 0, 0, 0);
	log_error(error);

	cl_ushort *result = (cl_ushort*)malloc(total_cells * sizeof(cl_ushort));

	log_start("Reading the result");
	clEnqueueReadBuffer(queue, buffer, 1, 0,
			total_cells * sizeof(cl_ushort), result,
			0, 0, 0);
	log_error(error);

	size_t light_count = 0;
	for (size_t i = 0; i < total_cells; i++) {
		light_count += (size_t)result[i];
	}

	printf("%u\n", light_count);

	getchar();
}

