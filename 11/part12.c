#include <string.h>
#include <stdio.h>
#include <xmmintrin.h>
#include <emmintrin.h>

int check(__m128i mpass)
{
	__m128i mi = _mm_set1_epi8('i');
	__m128i mo = _mm_set1_epi8('o');
	__m128i ml = _mm_set1_epi8('l');

	__m128i charmask = _mm_cmpgt_epi8(mpass, _mm_set1_epi8('`'));
	mpass = _mm_and_si128(mpass, charmask);

	__m128i bad = _mm_or_si128(_mm_or_si128(
				_mm_cmpeq_epi8(mpass, mi),
				_mm_cmpeq_epi8(mpass, mo)),
				_mm_cmpeq_epi8(mpass, ml));

	if (_mm_movemask_epi8(bad))
		return 0;

	__m128i diff = _mm_sub_epi8(mpass, _mm_slli_si128(mpass, 1));

	__m128i allzeros = _mm_cmpeq_epi8(diff, _mm_setzero_si128());
	__m128i zeros = _mm_and_si128(allzeros, charmask);

	int zeromask = _mm_movemask_epi8(zeros);
	if (!(zeromask && zeromask & ~(((zeromask - 1) ^ zeromask) << 1 | 1)))
		return 0;

	__m128i ones = _mm_cmpeq_epi8(diff, _mm_set1_epi8(1));
	__m128i triples = _mm_and_si128(ones, _mm_slli_si128(ones, 1));

	if (!_mm_movemask_epi8(triples))
		return 0;

	return 1;
}

#define MM_NOT_SI128(x) _mm_xor_si128((x), _mm_cmpeq_epi32( \
			_mm_setzero_si128(), _mm_setzero_si128()))

__m128i increment(__m128i password)
{
	__m128i zs = _mm_set1_epi8('z');
	__m128i addone = _mm_add_epi8(password, _mm_set1_epi8(1));
	__m128i safes = MM_NOT_SI128(_mm_cmpgt_epi8(addone, zs));
	safes = _mm_or_si128(safes, _mm_srli_si128(safes, 1));
	safes = _mm_or_si128(safes, _mm_srli_si128(safes, 2));
	safes = _mm_or_si128(safes, _mm_srli_si128(safes, 4));
	safes = _mm_or_si128(safes, _mm_srli_si128(safes, 8));
	__m128i non_overflows = _mm_srli_si128(safes, 1);

	__m128i incremented = _mm_or_si128(
			_mm_and_si128(non_overflows, password),
			_mm_andnot_si128(non_overflows, addone));

	return _mm_or_si128(
			_mm_and_si128(safes, incremented),
			_mm_andnot_si128(safes, _mm_set1_epi8('a')));
}

int main(int argc, char **argv)
{
	if (argc <= 1) return 2;

	char password[17];
	size_t len = strlen(argv[1]);
	if (len > 16) return 1;
	memset(password, '`', 16 - len);
	memcpy(password + (16 - len), argv[1], len);

	__m128i mpass = _mm_loadu_si128((__m128i*)password);
	do {
		mpass = increment(mpass);
	} while (!check(mpass));

	_mm_storeu_si128((__m128i*)password, mpass);
	size_t first_char;
	for (first_char = 0; first_char < 16; first_char++) {
		if (password[first_char] >= 'a') {
			break;
		}
	}
	password[16] = '\0';
	printf("%s\n", password + first_char);
}

