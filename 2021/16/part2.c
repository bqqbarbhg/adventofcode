#include <inttypes.h>
#include <stdio.h>

typedef struct {
    FILE *file;
    size_t pos;
    uint32_t bits;
    uint32_t valid;
} bitstream;

uint32_t readhex(bitstream *s)
{
    char c = getc(s->file);
    if (c >= '0' && c <= '9') {
        return (uint32_t)(c - '0');
    } else if (c >= 'A' && c <= 'F') {
        return (uint32_t)(c - 'A' + 10);
    } else {
        return 0;
    }
}

uint32_t readbits(bitstream *s, uint32_t num)
{
    while (s->valid < 32-4) {
        s->bits |= readhex(s) << (32-4 - s->valid);
        s->valid += 4;
    }
    uint32_t result = s->bits >> (32 - num);
    s->bits <<= num;
    s->valid -= num;
    s->pos += num;
    return result;
}

uint64_t evaluate_packet(bitstream *s)
{
    uint32_t version = readbits(s, 3);
    uint32_t type = readbits(s, 3);

	uint64_t value = 0;
    if (type == 4) {
        for (;;) {
            uint32_t cont = readbits(s, 1);
            value = (value << 4u) | (uint64_t)readbits(s, 4);
            if (!cont) break;
        }
    } else {
        uint32_t length_type = readbits(s, 1);
        size_t limit = readbits(s, length_type == 1 ? 11 : 15);
        size_t begin = s->pos;
        for (uint32_t num = 0; ; num++) {
            if (length_type == 1) {
                if (num >= limit) break;
            } else {
                if (s->pos - begin >= limit) break;
            }

            uint64_t arg = evaluate_packet(s);
            if (num == 0) {
                value = arg;
            } else {
                switch (type) {
                case 0: value += arg; break;
                case 1: value *= arg; break;
                case 2: value = arg < value ? arg : value; break;
                case 3: value = arg > value ? arg : value; break;
                case 5: value = value > arg ? 1 : 0; break;
                case 6: value = value < arg ? 1 : 0; break;
                case 7: value = value == arg ? 1 : 0; break;
                }
            }
        }
    }
	return value;
}

int main(int argc, char **argv)
{
    bitstream s = { stdin };
    uint64_t result = evaluate_packet(&s);
    printf("%" PRIu64 "\n", result);
    return 0;
}
