#include "ocl.cpp"

void enqueue_light_kernel(OCL_Context *ocl, cl_kernel kernel,
		int minX, int minY, int maxX, int maxY)
{
	ocl_arg_int(ocl, kernel, 1, minX);
	ocl_arg_int(ocl, kernel, 2, minY);
	ocl_arg_int(ocl, kernel, 3, maxX);
	ocl_arg_int(ocl, kernel, 4, maxY);

	size_t cellOffsetX = minX / 4;
	size_t cellOffsetY = minY / 4;
	size_t cellCountX = (maxX + 3) / 4 - cellOffsetX;
	size_t cellCountY = (maxY + 3) / 4 - cellOffsetY;

	ocl_enqueue_kernel_2d(ocl, kernel, cellOffsetX, cellOffsetY, cellCountX, cellCountY);
}

int main(int argc, char **argv)
{
	OCL_Context *ocl = ocl_make_context();

	static char log[1024*1024];
	cl_program program = ocl_compile_file(ocl, "part1_opencl.c", "", log, sizeof(log));
	if (!program) {
		fprintf(stderr, "Failed to compile\n%s\n", log);
		return 1;
	}

	cl_kernel k_set_zero = ocl_create_kernel(ocl, program, "set_zero");
	cl_kernel k_count_lights = ocl_create_kernel(ocl, program, "count_lights");

	cl_kernel k_turn_on = ocl_create_kernel(ocl, program, "turn_on");
	cl_kernel k_turn_off = ocl_create_kernel(ocl, program, "turn_off");
	cl_kernel k_toggle = ocl_create_kernel(ocl, program, "toggle");
	cl_kernel light_kernels[] = { k_turn_on, k_turn_off, k_toggle };

	int width = 1000;
	int height = 1000;

	int x_cells = (width + 3) / 4;
	int y_cells = (height + 3) / 4;
	size_t total_cells = x_cells * y_cells;
	size_t cells_size = total_cells * sizeof(cl_ushort);

	cl_mem buffer = ocl_create_buffer(ocl, CL_MEM_READ_WRITE, cells_size, 0);

	for (int i = 0; i < 3; i++) {
		ocl_arg_buffer(ocl, light_kernels[i], 0, buffer);
		ocl_arg_int(ocl, light_kernels[i], 5, x_cells);
	}
	ocl_arg_buffer(ocl, k_set_zero, 0, buffer);
	ocl_arg_buffer(ocl, k_count_lights, 0, buffer);

	ocl_enqueue_kernel_1d(ocl, k_set_zero, 0, total_cells);

	while (!feof(stdin)) {
		char line[256];
		int minX, minY, maxX, maxY;
		cl_kernel kernel = 0;

		if (!fgets(line, sizeof(line), stdin))
			continue;

		if (sscanf(line, "turn on %d,%d through %d,%d", &minX, &minY, &maxX, &maxY)) {
			kernel = k_turn_on;
		} else if (sscanf(line, "toggle %d,%d through %d,%d", &minX, &minY, &maxX, &maxY)) {
			kernel = k_toggle;
		} else if (sscanf(line, "turn off %d,%d through %d,%d", &minX, &minY, &maxX, &maxY)) {
			kernel = k_turn_off;
		}

		if (minX < 0) minX = 0;
		if (minY < 0) minY = 0;
		if (maxX > width - 1) minX = width - 1;
		if (maxY > height - 1) minY = height - 1;

		if (kernel) {
			enqueue_light_kernel(ocl, kernel, minX, minY, maxX + 1, maxY + 1);
		}
	}

	ocl_enqueue_kernel_1d(ocl, k_count_lights, 0, total_cells);

	cl_ushort *result = (cl_ushort*)malloc(cells_size);
	ocl_blocking_read(ocl, buffer, 0, total_cells * sizeof(cl_ushort), result);

	size_t light_count = 0;
	for (size_t i = 0; i < total_cells; i++) {
		light_count += (size_t)result[i];
	}

	printf("%u\n", light_count);
}

