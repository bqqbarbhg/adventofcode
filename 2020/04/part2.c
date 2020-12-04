#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef bool pass_validate_fn(const char *str, size_t len);

typedef struct pass_field {
    char name[3];
    bool required;
    pass_validate_fn *validate_fn;
} pass_field;

bool validate_byr(const char *str, size_t len)
{
    int val = atoi(str);
    return val >= 1920 && val <= 2002;
}

bool validate_iyr(const char *str, size_t len)
{
    int val = atoi(str);
    return val >= 2010 && val <= 2020;
}

bool validate_eyr(const char *str, size_t len)
{
    int val = atoi(str);
    return val >= 2020 && val <= 2030;
}

bool validate_hgt(const char *str, size_t len)
{
    int val = atoi(str);
    if (len > 2 && !memcmp(str + len - 2, "cm", 2)) {
        return val >= 150 && val <= 193;
    } else if (len > 2 && !memcmp(str + len - 2, "in", 2)) {
        return val >= 59 && val <= 76;
    } else {
        return false;
    }
}

bool validate_hcl(const char *str, size_t len)
{
    return len == 7 && str[0] == '#' && strspn(str + 1, "0123456789abcdef") == 6;
}

bool validate_ecl(const char *str, size_t len)
{
    static const char *opts[] = { "amb", "blu", "brn", "gry", "grn", "hzl", "oth" };
    if (len != 3) return false;
    for (size_t i = 0; i != sizeof(opts)/sizeof(*opts); i++) {
        if (!memcmp(str, opts[i], 3)) return true;
    }
    return false;
}

bool validate_pid(const char *str, size_t len)
{
    return len == 9 && strspn(str, "0123456789") == 9;
}

bool validate_cid(const char *str, size_t len)
{
    return true;
}

#define NUM_FIELDS 8
const pass_field fields[NUM_FIELDS] = {
    { "byr", 1, &validate_byr },
    { "iyr", 1, &validate_iyr },
    { "eyr", 1, &validate_eyr },
    { "hgt", 1, &validate_hgt },
    { "hcl", 1, &validate_hcl },
    { "ecl", 1, &validate_ecl },
    { "pid", 1, &validate_pid },
    { "cid", 0, &validate_cid },
};

typedef enum pass_state {
    PS_KEYN,
    PS_KEY0,
    PS_KEY1,
    PS_KEY2,
    PS_COLON,
    PS_VALUE,
    PS_ERROR,
    PS_ERRORN,
} pass_state;

#define VAL_MAX 32

typedef struct pass_ctx {
    pass_state state;
    char key[3];
    char val[VAL_MAX];
    size_t val_len;
    uint8_t field_count[NUM_FIELDS];
} pass_ctx;

bool pass_advance(pass_ctx *c, char ch)
{
    if (ch == '\r') return false;

    switch (c->state) {

    case PS_KEYN:
        if (ch == '\n') {
            bool ok = true;
            for (size_t i = 0; i < NUM_FIELDS && ok; i++) {
                uint32_t num = c->field_count[i];
                if (num == 0 && fields[i].required) {
                    ok = false;
                } else if (num > 1) {
                    ok = false;
                }
            }
            memset(c->field_count, 0, sizeof(c->field_count));
            if (ok) return true;
        } else {
			c->key[0] = ch;
			c->state = isalpha(ch) ? PS_KEY1 : PS_ERROR;
        }
        break;

    case PS_KEY0:
        c->key[0] = ch;
        c->state = isalpha(ch) ? PS_KEY1 : PS_ERROR;
        break;

    case PS_KEY1:
        c->key[1] = ch;
        c->state = isalpha(ch) ? PS_KEY2 : PS_ERROR;
        break;

    case PS_KEY2:
        c->key[2] = ch;
        c->state = PS_COLON;
        break;

    case PS_COLON:
        c->val_len = 0;
        c->state = ch == ':' ? PS_VALUE : PS_ERROR;
        break;

    case PS_VALUE:
        if (ch == ' ' || ch == '\n') {
            bool found = false;
            c->state = ch == '\n' ? PS_KEYN : PS_KEY0;
            c->val[c->val_len] = '\0';
            for (size_t i = 0; i < NUM_FIELDS; i++) {
                if (!memcmp(c->key, fields[i].name, 3)) {
                    if (c->field_count[i] == UINT8_MAX) c->state = PS_ERROR;
                    c->field_count[i]++;
                    found = true;

                    if (!fields[i].validate_fn(c->val, c->val_len)) {
                        c->state = PS_ERROR;
                    }

                    break;
                }
            }
            if (!found) c->state = PS_ERROR;
        } else {
            if (c->val_len >= VAL_MAX - 1) {
                c->state = PS_ERROR;
            } else {
                c->val[c->val_len++] = ch;
            }
        }
        break;

    case PS_ERROR:
        if (ch == '\n') c->state = PS_ERRORN;
        break;

    case PS_ERRORN:
        if (ch == '\n') {
            memset(c->field_count, 0, sizeof(c->field_count));
            c->state = PS_KEYN;
        } else {
            c->state = PS_ERROR;
        }
        break;
    }

    return false;
}

int main(int argc, char **argv) {
    int ich;
    pass_ctx ctx = { 0 };
    size_t count = 0;
    while ((ich = getc(stdin)) != EOF) {
        if (pass_advance(&ctx, ich)) count++;
    }
	if (pass_advance(&ctx, '\n')) count++;
	if (pass_advance(&ctx, '\n')) count++;

    printf("%zu\n", count);
    return 0;
}
