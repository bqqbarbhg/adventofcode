#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint32_t checksum(uint32_t *values, uint32_t count);

int main(int argc, char **argv)
{
	uint32_t values[64];
	char line[512];
	uint32_t total = 0;

	while (fgets(line, sizeof(line), stdin)) {
		uint32_t count = 0;
		char *pos = line;
		for (;;) {
			char *prev = pos;
			uint32_t val = (uint32_t)strtol(pos, &pos, 0);
			if (pos == prev) break;
			values[count++] = val;
		}

		total += checksum(values, count);
	}

	printf("%u\n", total);
	return 0;
}

