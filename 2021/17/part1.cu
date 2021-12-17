#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>

struct Target
{
	int2 min;
	int2 max;

	__device__ bool contains(int2 pos) const
	{
		return pos.x >= min.x && pos.x <= max.x
			&& pos.y >= min.y && pos.y <= max.y;
	}

};

__device__ int simulate(Target target, int2 init_vel)
{
	int2 vel = init_vel;
	int2 pos = make_int2(0, 0);
	int max_y = 0;
	while (pos.y >= target.min.y) {
		pos.x += vel.x;
		pos.y += vel.y;
		vel.x += vel.x != 0 ? (vel.x > 0 ? -1 : 1) : 0;
		vel.y -= 1;
		max_y = max(pos.y, max_y);
		if (target.contains(pos)) return max_y;
	}
	return -1;
}

__global__ void find_max_y(Target target, int *p_max_y)
{
	int x = blockDim.x*blockIdx.x + threadIdx.x;
	int y = blockDim.y*blockIdx.y + threadIdx.y;
	int max_y = simulate(target, make_int2(x, y));
	atomicMax(p_max_y, max_y);
}

int main(int argc, char **argv)
{
	Target target;
	scanf("target area: x=%d..%d, y=%d..%d", &target.min.x, &target.max.x, &target.min.y, &target.max.y);

	int mx = abs(target.min.x);
	int my = abs(target.min.y);

	int *p_max_y;
	cudaMallocManaged(&p_max_y, sizeof(int));
	*p_max_y = 0;

	dim3 block_threads = { 8, 8 };
	dim3 num_blocks = { (mx*4+block_threads.x-1)/block_threads.x, (my*4+block_threads.y-1)/block_threads.y };
	find_max_y<<<num_blocks, block_threads>>>(target, p_max_y);

	int max_y = 0;
	cudaMemcpy(&max_y, p_max_y, sizeof(int), cudaMemcpyDefault);
	cudaFree(p_max_y);

	printf("%d\n", max_y);
	return 0;
}
