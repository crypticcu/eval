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
	char *expr, *swap, chr;
	bool help_only = false, field_is_last = false;
	unsigned field = 1;
	double result;
	double ndec = 6;	// Number of decimal places, default is 6 (same as printf)

	if (atexit(clrstat))
		return EXIT_FAILURE;
	for (size_t arg = 1; arg < argc; arg++) {	// Get Flags
		if (*argv[arg] != '-') {
			if (!strchr(argv[arg - 1], 'd') && argc < argc - 1) {
				setstat(ERR_INVARG);
				setinv(NULL, arg);
				break;
			}
		} else {
			for (size_t index = 1; (chr = *(argv[arg] + index)); index++) {
				switch (chr) {
				case 'h':
					Flags.help = true;
					break;
				case 'd':
					if (arg == argc - 1) {
						setstat(ERR_INVARG);
						setinv(NULL, arg);
						break;					}
					ndec = stod(argv[arg + field]);
					if (ndec < 0 || ndec > MaxDec || !iswhole(ndec)) {
						setstat(ERR_INVDEC);
						break;
					}
					if (arg + field == argc - 1)
						field_is_last = true;
					Flags.round = true;
					field++;
					break;
				case 'r':
					Flags.radian = true;
					break;
				default:
					setstat(ERR_INVFLAG);
					setinv(argv[arg], index);
				}
			}
		}
		field = 0;
	}
	if (ErrStat > 0) {
		pstatus();
		return EXIT_FAILURE;
	}
	if (Flags.help) {
		phelp();
		if (help_only)
			return EXIT_SUCCESS;
		putchar('\n');	// Seperate help page from normal output
	}

	/* Command-line */
	if (*argv[argc - 1] != '-' && !strchr(argv[argc > 1 ? argc - 2 : argc - 1], 'd') && argc > 1) {
		CmdLn = true;
		if (strlen(expr = argv[argc - 1]) >= MaxLn) {			setstat(ERR_INPUTSIZE);
			pstatus();
			return EXIT_FAILURE;
		}
		if (!(expr = parse(expr, ndec, NULL))) {
			pstatus();
			return EXIT_FAILURE;
		}
		puts(expr);
		free(expr);
	/* Interactive */
	} else {
		CmdLn = false;
		while (true) {
			printf("> ");
			if (!(expr = getln(MaxLn))) {	// Subtract 1 because string length does not count null character
				pstatus();
				continue;
			}
			if (!strcmp(expr, "\n")) {
				free(expr);
				break;
			}
			expr[strlen(expr) - 1] = '\0';	// Remove newline
			swap = expr;	// Swap causes 'still reachable' error in valgrind
			expr = parse(expr, ndec, NULL);
			free(swap);
			if (expr) {
				puts(expr);
				free(expr);
			} else
				pstatus();
		}
	}

	return EXIT_SUCCESS;
}

void phelp(void) {
	puts("Usage: parse [FLAGS] [EXPRESSION]    Command-line");
	puts("       parse                         Interactive ");
	puts("High-accuracy terminal calculator\n");

	puts("Flags");
	puts("-d [INT]   Round to # of decimals");
	puts("-h         Show help page");
	puts("-r         Radian mode\n");

	puts("Operators");
	puts("++, --     ++x, --x         Increment, decrement");
	puts("!, !!      !x, y!!x         Square root, other root        ↑ Higher precedence\n");
	puts("^          x^y              Exponent");
	puts("*, /, %    x*y, x/y, x%y    Multiply, divide, remainder    ↓ Lower Precedence\n");
	puts("+, -       x+y, x-y         Add, subtract\n");

	puts("           (x + y)          Control precedence");
	puts("           x(y)             Multiply terms\n");

	puts("GitHub repository: https://github.com/crypticcu/eval");
	puts("Report bugs to:    cryptic.cu@protonmail.com");
}