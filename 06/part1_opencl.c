
inline unsigned short make_mask(int x, int y, int minX, int minY, int maxX, int maxY)
{
	int realX = x * 4;
	int realY = y * 4;

	unsigned short mask = 0xFFFF;

	int minXdiff = minX - realX;
	if (minXdiff > 0) {
		if (minXdiff == 1) {
			mask &= 0x7777;
		} else if (minXdiff == 2) {
			mask &= 0x3333;
		} else if (minXdiff == 3) {
			mask &= 0x1111;
		} else {
			mask = 0x0000;
		}
	}

	int minYdiff = minY - realY;
	if (minYdiff > 0) {
		if (minYdiff == 1) {
			mask &= 0x0FFF;
		} else if (minYdiff == 2) {
			mask &= 0x00FF;
		} else if (minYdiff == 3) {
			mask &= 0x000F;
		} else {
			mask = 0x0000;
		}
	}

	int maxXdiff = realX + 4 - maxX;
	if (maxXdiff > 0) {
		if (maxXdiff == 1) {
			mask &= 0xEEEE;
		} else if (maxXdiff == 2) {
			mask &= 0xCCCC;
		} else if (maxXdiff == 3) {
			mask &= 0x8888;
		} else {
			mask = 0x0000;
		}
	}

	int maxYdiff = realY + 4 - maxY;
	if (maxYdiff > 0) {
		if (maxYdiff == 1) {
			mask &= 0xFFF0;
		} else if (maxYdiff == 2) {
			mask &= 0xFF00;
		} else if (maxYdiff == 3) {
			mask &= 0xF000;
		} else {
			mask = 0x0000;
		}
	}

	return mask;
}

inline unsigned short popcount_impl(unsigned short val)
{
	val = (val & 0x5555) + (val >> 1 & 0x5555);
	val = (val & 0x3333) + (val >> 2 & 0x3333);
	val = (val & 0x0f0f) + (val >> 4 & 0x0f0f);
	val = (val & 0x00ff) + (val >> 8 & 0x00ff);
	return val;
}

__kernel void set_zero(__global unsigned short *states)
{
	int i = get_global_id(0);
	states[i] = 0x0000;
}

__kernel void count_lights(__global unsigned short *states)
{
	int i = get_global_id(0);
	states[i] = popcount_impl(states[i]);
}

__kernel void turn_on(__global unsigned short *states,
		int minX, int minY, int maxX, int maxY, int width)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	unsigned short mask = make_mask(x, y, minX, minY, maxX, maxY);
	states[x + y * width] |= mask;
}

__kernel void turn_off(__global unsigned short *states,
		int minX, int minY, int maxX, int maxY, int width)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	unsigned short mask = make_mask(x, y, minX, minY, maxX, maxY);
	states[x + y * width] &= ~mask;
}

__kernel void toggle(__global unsigned short *states,
		int minX, int minY, int maxX, int maxY, int width)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	unsigned short mask = make_mask(x, y, minX, minY, maxX, maxY);
	states[x + y * width] ^= mask;
}

