#include <xmmintrin.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	const __m128i open = _mm_set1_epi8('(');
	const __m128i close = _mm_set1_epi8(')');

	int floor = 0;

	char input[1024] __attribute__((aligned(16)));

	while (!feof(stdin)) {
		size_t length = fread(input, 1, sizeof(input), stdin);

		size_t underflow = (16 - length % 16) % 16;
		for (size_t i = 0; i < underflow; i++) {
			input[length + i] = '\0';
		}

		for (size_t block = 0; block < length; block += 16) {
			__m128i input_block = _mm_load_si128((const __m128i*)(input + block));

			__m128i open_block = _mm_cmpeq_epi8(input_block, open);
			__m128i close_block = _mm_cmpeq_epi8(input_block, close);

			int opens = _mm_movemask_epi8(open_block);
			int closes = _mm_movemask_epi8(close_block);

			floor += __builtin_popcount(opens) - __builtin_popcount(closes);
		}
	}

	printf("%d\n", floor);
}

