#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "parse.h"
#include "status.h"
#include "util.h"

/* Prints the help page */
void phelp(void);

int main(int argc, char *argv[]) {
	bool help_only = false;
	char *expr, *swap, chr;
	double result, ndec = 6;	// Number of decimal places; Default is 6 (same as printf)

	if (initstat())
		return EXIT_FAILURE;
	maxdec = nplaces(SSIZE_MAX);	// # of accurate digits retained when converting double to integral
	maxln = SSIZE_MAX;	// Equal to or lower than SIZE_MAX; Alleviates conversion issues

	/* Interactive */
	if (argc == 1) {
		cmdln = false;
		while (true) {
			printf("> ");
			if (!(expr = getln(maxln - 1))) {	// Subtract 1 because string length does not count null character
				pstatus();
				continue;
			}
			if (!strcmp(expr, "\n")) {
				free(expr);
				break;
			}
			expr[strlen(expr) - 1] = '\0';	// Remove newline
			swap = expr;	// Swap causes 'still reachable' error in valgrind
			expr = parse(expr, NULL);
			free(swap);
			if (expr) {
				puts(expr);
				free(expr);
			} else
				pstatus();
		}
		return EXIT_SUCCESS;
	}

	/* Command line */
	cmdln = true;
	if (strlen(expr = argv[argc - 1]) >= maxln) {	
		errstat(ERR_INPUTSIZE);
		pstatus();
		return EXIT_FAILURE;
	}
	if (argv[argc - 1])
	if (argc == 2 && !strcmp(argv[argc - 1], "-h"))
		help_only = true;
	if (*argv[argc - 1] == '-' &&  !help_only) {	// Final argument cannot be flag, unless it is '-h' flag
		errstat(ERR_INVFLAG);
		pstatus();
		return EXIT_FAILURE;
	}
	for (size_t nstr = 1; nstr < argc; nstr++) {	// Get flags
		for (size_t index = 1; *argv[nstr] == '-' && (chr = *(argv[nstr] + index)); index++) {
			switch (chr) {
			case 'h':
				flags.help = true;
				break;
			case 'd':
				flags.round = true;
				if (nstr != argc - 2) {	// '-d' place takes argument afterward
					if ((ndec = stod(argv[nstr + 1])) < 0 || ndec > maxdec || !isequal(ndec, (ssize_t) ndec)) {
						errstat(ERR_INVDEC);
						pstatus();
						return EXIT_FAILURE;
					}
				} else {
					errstat(ERR_INVFLAG);
					pstatus();
					return EXIT_FAILURE;
				}
				break;
			case 'r':
				flags.radian = true;
				break;
			default:
				errstat(ERR_INVFLAG);
				pstatus();
				return EXIT_FAILURE;
			}
		}
	}
	if (flags.help) {
		phelp();
		if (help_only)
			return EXIT_SUCCESS;
		putchar('\n');	// Seperate help page from answer
	}
	if (!(expr = parse(expr, NULL))) {
		putchar('\n');	// Seperate error message from terminal cursor
		return EXIT_FAILURE;
	}
	puts(expr);
	free(expr);
	return EXIT_SUCCESS;
}

void phelp(void) {
	puts("Usage: parse [FLAGS] [EXPRESSION]    Command-line");
	puts("       parse                         Interactive ");
	puts("High-accuracy terminal calculator");
	puts("This software falls under the GNU Public License v3.0\n");

	puts("Flags");
	puts("-h         Show help page");
	puts("-d [INT]   Round to # of decimals");
	puts("-r         Radian mode\n");

	puts( "Operators" );
	puts("++, --     ++x, --x         Increment, decrement");
	puts("!, !!      !x, y!!x         Square root, other root        ↑ Higher precedence");
	puts("^          x^y              Exponent");
	puts("*, /, %    x*y, x/y, x%y    Multiply, divide, remainder    ↓ Lower Precedence");
	puts("+, -       x+y, x-y         Add, subtract\n");

	puts("           (x + y)          Control precedence");
	puts("           x(y)             Multiply terms\n");

	puts("GitHub repository: https://github.com/crypticcu/eval");
	puts("Report bugs to:    cryptic.cu@protonmail.com");
}