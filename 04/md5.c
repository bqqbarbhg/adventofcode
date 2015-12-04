#include <string.h>
#include <stdint.h>
#include "md5.h"

static uint32_t left_rotate(uint32_t val, uint32_t shift)
{
	return val << shift | val >> (32 - shift);
}

#define ROUND_START_A() (f = (b & c) | (~b & d), g = i)
#define ROUND_START_B() (f = (d & b) | (~d & c), g = index_table[i])
#define ROUND_START_C() (f = b ^ c ^ d, g = index_table[i])
#define ROUND_START_D() (f = c ^ (b | ~d), g = index_table[i])

#define ROUND_SECOND(shift) (t = d, d = c, c = b, \
	b += left_rotate((a + f + k[i] + m[g]), shift), a = t, i++)

static const uint32_t table[] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

static const uint8_t index_table[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,

	0x01, 0x06, 0x0b, 0x00, 0x05, 0x0a, 0x0f, 0x04,
	0x09, 0x0e, 0x03, 0x08, 0x0d, 0x02, 0x07, 0x0c,

	0x05, 0x08, 0x0b, 0x0e, 0x01, 0x04, 0x07, 0x0a,
	0x0d, 0x00, 0x03, 0x06, 0x09, 0x0c, 0x0f, 0x02,

	0x00, 0x07, 0x0e, 0x05, 0x0c, 0x03, 0x0a, 0x01,
	0x08, 0x0f, 0x06, 0x0d, 0x04, 0x0b, 0x02, 0x09,
};

void md5_hash(void *result, const void *data, size_t length)
{
	const char *data_chars = (const char*)data;
	unsigned char *result_chars = (unsigned char*)result;

	uint32_t a0 = 0x67452301;
	uint32_t b0 = 0xefcdab89;
	uint32_t c0 = 0x98badcfe;
	uint32_t d0 = 0x10325476;

	uint32_t a, b, c, d, g, f, t, i;
	const uint32_t *k = table;

	char final_block[64];

	size_t length_appended = length + 5;
	size_t block;

	for (block = 0; block < length_appended; block += 64) {
		const uint32_t *m = (uint32_t*)(data_chars + block);

		if (block + 64 > length) {
			size_t left = length - block;
			size_t rest = left + 1;
			uint64_t length_bits = (uint64_t)length * 8;

			memcpy(final_block, m, left);
			final_block[left] = 0x80;
			memset(final_block + rest, 0, 56 - rest);
			
			memcpy(final_block + 56, &length_bits, 8);

			m = (const uint32_t*)(final_block);
		}

		a = a0, b = b0, c = c0, d = d0, i = 0;

		while (i < 16) {
			ROUND_START_A(); ROUND_SECOND(7);
			ROUND_START_A(); ROUND_SECOND(12);
			ROUND_START_A(); ROUND_SECOND(17);
			ROUND_START_A(); ROUND_SECOND(22);
		}

		while (i < 32) {
			ROUND_START_B(); ROUND_SECOND(5);
			ROUND_START_B(); ROUND_SECOND(9);
			ROUND_START_B(); ROUND_SECOND(14);
			ROUND_START_B(); ROUND_SECOND(20);
		}

		while (i < 48) {
			ROUND_START_C(); ROUND_SECOND(4);
			ROUND_START_C(); ROUND_SECOND(11);
			ROUND_START_C(); ROUND_SECOND(16);
			ROUND_START_C(); ROUND_SECOND(23);
		}

		while (i < 64) {
			ROUND_START_D(); ROUND_SECOND(6);
			ROUND_START_D(); ROUND_SECOND(10);
			ROUND_START_D(); ROUND_SECOND(15);
			ROUND_START_D(); ROUND_SECOND(21);
		}

		a0 += a, b0 += b, c0 += c, d0 += d;
	}

	*result_chars++ = a0;
	*result_chars++ = a0 >> 8;
	*result_chars++ = a0 >> 16;
	*result_chars++ = a0 >> 24;
	*result_chars++ = b0;
	*result_chars++ = b0 >> 8;
	*result_chars++ = b0 >> 16;
	*result_chars++ = b0 >> 24;
	*result_chars++ = c0;
	*result_chars++ = c0 >> 8;
	*result_chars++ = c0 >> 16;
	*result_chars++ = c0 >> 24;
	*result_chars++ = d0;
	*result_chars++ = d0 >> 8;
	*result_chars++ = d0 >> 16;
	*result_chars++ = d0 >> 24;
}

void md5_dump(char *buffer, const void *hash)
{
	static const char *hex_chars = "0123456789abcdef";
	unsigned char *hash_bytes = (unsigned char*)hash;
	unsigned char *byte = hash_bytes, *end = byte + MD5_HASH_BYTES;

	for (; byte < end; byte++) {
		unsigned char b = *byte;
		*buffer++ = hex_chars[b >> 4];
		*buffer++ = hex_chars[b & 0xF];
	}
	*buffer++ = '\0';
}

