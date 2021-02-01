#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

bool cmd_line = true;	// Using command-line interface?

int main(int argc, char *argv[]) {
	bool help_only = false;
	char chr, *expr = NULL, *swap = NULL;
	size_t bufsize = 999;		// Maximum input size
	double result, ndec = 6;	// Number of decimal places; Default is 6 (same as printf)

	if (argc == 1) {	// Interactive
		cmd_line = false;
		for (;;) {
			printf("> ");
			expr = calloc(bufsize + 1, sizeof(char));
			getline(&expr, &bufsize, stdin);
			if (!strcmp(expr, "\n")) {
				free(expr);
				break;
			}
			expr[strlen(expr) - 1] = 0;	// Ignore newline
			swap = expr;	// Swap causes 'still reachable' error in valgrind
			expr = simplify(expr, 0);
			free(swap);
			if (expr != NULL) {
				result = stod(expr);
				free(expr);
				puts(expr = dtos(result, 6));
				free(expr);
			}
		}
		return EXIT_SUCCESS;
	} else {	// Command-line
		if (argc == 2 && !strcmp(argv[argc - 1], "-h"))
			help_only = true;
		if (*argv[argc - 1] == '-' &&  !help_only)	// Final argument cannot be flag, unless it is '-h' flag
			goto invflag_err;
		for (size_t nstr = 1; nstr < argc; nstr++) {	// Get flags
			for (size_t nchr = 1; *argv[nstr] == '-' && (chr = *(argv[nstr] + nchr)); nchr++) {
				switch (chr) {
				case 'h':
					flags.help = true;
					break;
				case 'd':
					flags.round = true;
					if (nstr != argc - 2) {	// '-d' place takes argument afterward
						if ((ndec = stod(argv[nstr + 1])) < 0 || ndec > DBL_DIG || !isequal(ndec, (int) ndec))
							goto invdec_err;
					} else
						goto invflag_err;
					break;
				case 'r':
					flags.radian = true;
					break;
				default:
					goto invflag_err;
				}
			}
		}
		if (flags.help) {
			print_help();
			if (help_only)
				return EXIT_SUCCESS;
			putchar('\n');	// Seperate help page from answer
		}
		if ((expr = simplify(argv[argc - 1], 0)) == NULL) {
			putchar('\n');	// Seperate error message from terminal cursor
			return EXIT_FAILURE;
		}
		#if DEBUG
		puts("\n\e[4mresult\e[24m");
		#endif
		result = stod(expr);
		free(expr);
		puts(expr = dtos(result, ndec));	// 6 = default number of decimals shown
		free(expr);
		return EXIT_SUCCESS;
	}

	invflag_err:	fail("Invalid flag usage");
	invdec_err:		fail("Invalid # of decimals");
}