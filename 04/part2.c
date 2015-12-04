#define _CRT_SECURE_NO_WARNINGS
#include "md5.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char **argv)
{
	int count = 0;
	uint32_t hash[MD5_HASH_BYTES];
	char hexdump[MD5_DUMP_LENGTH];
	char input[128];
	size_t input_length;

	do {
		count++;
		input_length = sprintf(input, "%s%d", argv[1], count);
		md5_hash(hash, input, input_length);
	} while (hash[0] & 0xFFFFFF);

	md5_dump(hexdump, hash);
	puts(hexdump);
	printf("%d\n", count);
}

