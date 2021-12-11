#include <stdio.h>
#include <stdint.h>
#include <intrin.h>
#include <stdbool.h>

__m128i rows[12];

__forceinline int add(__m128i *p_row, __m128i inc)
{
	__m128i limit = _mm_set1_epi8(9);
	__m128i row = _mm_add_epi8(*p_row, inc);
	*p_row = row;
	__m128i proc = _mm_cmpgt_epi8(row, limit);
	return (int)_mm_test_all_zeros(proc, proc) ^ 1;
}

__forceinline int add_nonzero(__m128i *p_row, __m128i inc)
{
	__m128i limit = _mm_set1_epi8(9);
	__m128i row = *p_row;
	__m128i zeros = _mm_cmpeq_epi8(row, _mm_setzero_si128());
	row = _mm_blendv_epi8(_mm_add_epi8(row, inc), row, zeros);
	*p_row = row;
	__m128i proc = _mm_cmpgt_epi8(row, limit);
	return (int)_mm_test_all_zeros(proc, proc) ^ 1;
}

__declspec(noinline) uint32_t step()
{
	uint32_t num = 0;
	__m128i limit = _mm_set1_epi8(9);
	__m128i ones = _mm_set1_epi8(1);
	__m128i inc_ones = _mm_srli_si128(ones, 16-10);

	uint32_t dirty = 0;
	for (size_t i = 1; i < 11; i++) {
		dirty |= add(&rows[i], inc_ones) << i;
	}

	while (dirty) {
		uint32_t i = _tzcnt_u32(dirty);
		dirty &= dirty - 1;

		__m128i row = rows[i];
		__m128i proc = _mm_cmpgt_epi8(rows[i], limit);
		row = _mm_andnot_si128(proc, row);

		num += _mm_popcnt_u32(_mm_movemask_epi8(proc));

		__m128i inc = _mm_and_si128(proc, ones);
		__m128i left_inc = _mm_slli_si128(inc, 1);
		__m128i right_inc = _mm_srli_si128(inc, 1);
		__m128i self_inc = _mm_add_epi8(left_inc, right_inc);
		__m128i nb_inc = _mm_add_epi8(self_inc, inc);

		dirty |= (add_nonzero(&rows[i - 1], nb_inc)) << (i-1);
		dirty |= (add_nonzero(&rows[i + 1], nb_inc)) << (i+1);
		dirty |= (add_nonzero(&row, self_inc)) << i;
		rows[i] = row;
	}

	return num;
}

int main(int argc, char **argv)
{
	char line[16] = { 0 };
	size_t row = 1;
	for (size_t row = 1; row < 11; row++) {
		fgets(line, sizeof(line), stdin);
		for (size_t i = 0; i < 10; i++) {
			line[i] -= '0';
		}
		for (size_t i = 10; i < 16; i++) {
			line[i] = 0;
		}
		rows[row] = _mm_loadu_si128((__m128i*)line);
	}

	size_t num_steps = 0;
	for (; ; num_steps++) {
		uint32_t procs = step();
		if (procs == 100) break;
	}

	printf("%zu\n", num_steps + 1);

	return 0;
}
