#include <stdio.h>

int main(int argc, char **argv)
{
	static char input[16 * 1024] = { 0 };
	size_t length = fread(input, 1, sizeof(input), stdin);

	int floor = 0;

	for (size_t i = 0; i < length; i++) {
		char c = input[i];

		unsigned relative = c - '(';
		if (relative < 2) {
			floor -= relative * 2 - 1;

			if (floor < 0) {
				printf("%d\n", (int)(i + 1));
				return 0;
			}
		}
	}
	return 1;
}

