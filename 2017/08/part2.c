#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

uint16_t code[64*1024];

int main(int argc, char **argv) {
	int registers[64] = { 0 };
	int max_value = INT_MIN;

	/* Read the input */
	{
		unsigned readpos = 0;
		char line[64];
		while (fgets(line, sizeof(line), stdin)) {
			unsigned int val;
			if (sscanf(line, "%x", &val) == 1) {
				code[readpos] = val;
				readpos++;
			}
		}
	}

	/* Execute */
	{
		unsigned pc = 0;
		while (code[pc] != 0xFFFF) {
			uint16_t op = code[pc];
			int cmp_imm = (int)(int16_t)code[pc + 1];
			int dst_imm = (int)(int16_t)code[pc + 2];

			int cmp_reg = registers[op >> 6 & 0x3f];

			int cmp_res;
			switch (op >> 13 & 0x3) {
				case 0x0: cmp_res = cmp_reg == cmp_imm; break;
				case 0x1: cmp_res = cmp_reg <  cmp_imm; break;
				case 0x2: cmp_res = cmp_reg >  cmp_imm; break;
			}
			if ((op >> 15) & 0x1)
				cmp_res = !cmp_res;

			int *dr = &registers[op & 0x3f];
			if (cmp_res) {
				switch (op >> 12 & 0x1) {
					case 0x0: *dr += dst_imm; break;
					case 0x1: *dr -= dst_imm; break;
				}
			}

			if (*dr > max_value)
				max_value = *dr;

			pc += 3;
		}
	}

	/* Print max register */
	printf("%d\n", max_value);

	return 0;
}


