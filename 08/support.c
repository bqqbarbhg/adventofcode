#include <stdio.h>

int computed_length(const char *data, int *source_len, int *data_len);

int main(int argc, char **argv)
{
        int total_source = 0;
        int total_data = 0;

        char line[256];
        while (!feof(stdin)) {
                int source_len, data_len;
                if (!fgets(line, sizeof(line), stdin))
                        break;

                if (computed_length(line, &source_len, &data_len)) {
                        total_source += source_len;
                        total_data += data_len;
                }
        }

        printf("%d\n", total_source - total_data);
}
