#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

typedef struct pass_field {
	char name[3];
	bool required;
} pass_field;

#define NUM_FIELDS 8
const pass_field fields[NUM_FIELDS] = {
	{ "byr", 1 },
	{ "iyr", 1 },
	{ "eyr", 1 },
	{ "hgt", 1 },
	{ "hcl", 1 },
	{ "ecl", 1 },
	{ "pid", 1 },
	{ "cid", 0 },
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

typedef struct pass_ctx {
	pass_state state;
	char key[3];
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
		c->state = ch == ':' ? PS_VALUE : PS_ERROR;
		break;

	case PS_VALUE:
		if (ch == ' ' || ch == '\n') {
			bool found = false;
			c->state = ch == '\n' ? PS_KEYN : PS_KEY0;
			for (size_t i = 0; i < NUM_FIELDS; i++) {
				if (!memcmp(c->key, fields[i].name, 3)) {
					if (c->field_count[i] == UINT8_MAX) c->state = PS_ERROR;
					c->field_count[i]++;
					found = true;
					break;
				}
			}
			if (!found) c->state = PS_ERROR;
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
