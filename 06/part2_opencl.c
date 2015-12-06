__kernel void set_zero(__global unsigned short *states)
{
	int i = get_global_id(0);
	states[i] = 0;
}

__kernel void turn_on(__global unsigned int *states, int width)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	states[x + y * width] += 1;
}

__kernel void turn_off(__global unsigned int *states, int width)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	int index = x + y * width;
	states[index] = sub_sat(states[index], (unsigned int)1);
}

__kernel void toggle(__global unsigned int *states, int width)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	states[x + y * width] += 2;
}


