#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

struct trie {
	char ch;
	char terminal;
	uint16_t num;
	uint16_t child, next;
};

static struct trie g_trie[1024];
uint32_t g_num_trie = 1;

int main(int argc, char **argv) {
	char line[512];
	uint32_t total = 0, failed = 0;

	while (fgets(line, sizeof(line), stdin)) {
		char *c = line;

		g_num_trie = 1;
		g_trie[0].child = 0;

		while (isspace(*c)) c++;
		if (*c != '\0') total++;

		while (*c != '\0') {

			uint16_t counts[26] = { 0 };
			for (; *c >= 'a' && *c <= 'z'; c++) {
				counts[*c - 'a']++;
			}

			uint16_t ix;
			struct trie *tr = &g_trie[0];
			for (ix = 0; ix < 26; ix++) {
				uint16_t child_ix;
				char ch = ix + 'a';
				uint16_t num = counts[ix];
				if (num == 0) continue;

				child_ix = tr->child;
				while (child_ix) {
					struct trie *child = &g_trie[child_ix];
					if (child->ch == ch && child->num == num) break;
					child_ix = child->next;
				}

				if (child_ix) {
					tr = &g_trie[child_ix];
				} else {
					uint16_t ix = g_num_trie++;
					struct trie *nt = &g_trie[ix];
					nt->ch = ch;
					nt->num = num;
					nt->terminal = 0;
					nt->next = tr->child;
					nt->child = 0;
					tr->child = ix;
					tr = nt;
				}
			}

			if (tr->terminal) {
				failed++;
				break;
			}
			tr->terminal = 1;

			if (!isspace(*c) && c != '\0') {
				fprintf(stderr, "Invalid character: '%c'\n", *c);
				exit(1);
			}

			while (isspace(*c)) c++;
		} 
	}

	printf("%u\n", total - failed);
}

