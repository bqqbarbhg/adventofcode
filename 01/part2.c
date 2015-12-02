#include <stdio.h>

int main(int argc, char **argv)
{
	int floor = 0;

	char input[1024];
	int base = 1;

	while (!feof(stdin)) {
		size_t length = fread(input, 1, sizeof(input), stdin);

		for (size_t i = 0; i < length; i++) {

			unsigned relative = input[i] - '(';
			if (relative < 2) {
				floor -= relative * 2 - 1;

				if (floor < 0) {
					printf("%d\n", (int)(base + i));
					return 0;
				}
			}
		}

		base += (int)length;
	}
	return 1;
}

