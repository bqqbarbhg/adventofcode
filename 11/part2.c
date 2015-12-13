
#define main(p_argc, p_argv) main(int argc, char **argv) { \
	extern int main_wrapper(int argc, char **argv); \
	return main_wrapper(argc, argv); \
	} \
	int main_original(p_argc, p_argv)

#include "part1.c"

int main_wrapper(int argc, char **argv)
{
	char temp[L_tmpnam];
	tmpnam(temp);

	FILE *outfile = freopen(temp, "w", stdout);
	main_original(argc, argv);
	fclose(outfile);

#if _WIN32
	freopen("CONOUT$", "a", stdout);
#else
	freopen("/dev/tty", "a", stdout);
#endif

	char line[128];
	FILE *infile = fopen(temp, "r");
	fgets(line, sizeof(line), infile);
	fclose(infile);

	char *endline = strchr(line, '\n');
	if (endline) {
		*endline = '\0';
	}

	char *new_argv[] = { argv[0], line };
	main_original(argc, new_argv);

	remove(temp);
}

