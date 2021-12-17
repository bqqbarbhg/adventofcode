#include <stdint.h>
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

uint32_t parse_packet(bitstream *s)
{
    uint32_t version = readbits(s, 3);
    uint32_t type = readbits(s, 3);
    uint32_t result = version;

    if (type == 4) {
        uint32_t value = 0;
        for (;;) {
            uint32_t cont = readbits(s, 1);
            value = (value << 4) | readbits(s, 4);
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

            result += parse_packet(s);
        }
    }

    return result;
}

int main(int argc, char **argv)
{
    bitstream s = { stdin };
    uint32_t result = parse_packet(&s);
    printf("%u\n", result);
    return 0;
}
