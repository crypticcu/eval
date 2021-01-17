#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eval.h"

_character_sets chrsets = {
	"+-!^*/%%.()'\n1234567890",	// Valid characters
	"1234567890",				// Digits
	"+-!^*/%%",					// Operators
	"+-!",						// Unary operators
	"^*/%%"						// Binary operators
};

_flags flags = {
	false,						// Show help				-h
	false						// Round to # of decimals	-d
};

bool cmd_line = true;	// Using command-line interface?

void print_help(void);

int main(int argc, char *argv[]) {
	bool invalid_flag = false, invalid_decplace = false, help_only = false;
	char chr, *expr = NULL, *swap = NULL;
	size_t bufsize = 999;		// Maximum input size
	double result, ndec = 6;	// Number of decimal places; Default is 6 (same as printf)

	if (argc == 1) {
		cmd_line = false;
		goto interactive;
	}

	command_line:
	if (argc == 2 && !strcmp(argv[argc - 1], "-h"))
		help_only = true;
	if (*argv[argc - 1] == '-' &&  !help_only)	// Final argument cannot be flag, unless it is '-h' flag
		goto invflag_err;
	for (size_t nstr = 1; nstr < argc && !invalid_flag; nstr++) {	// Get flags
		if (*argv[nstr] == '-') {
			for (size_t nchr = 1; (chr = *(argv[nstr] + nchr)); nchr++) {
				switch (chr) {
				case 'h':
					flags.help = true;
					break;
				case 'd':
					flags.round = true;
					if (nstr != argc - 2) {	// '-d' place takes argument afterward
						if ((ndec = stod(argv[nstr + 1])) < 0 || ndec > DBL_DIG || !isequal(ndec, (int) ndec))
							invalid_decplace = true;
					} else
						invalid_flag = true;
					break;
				default:
					invalid_flag = true;
				}
			}

		}
	}

	if (invalid_flag)
		goto invflag_err;
	if (invalid_decplace)
		goto invdec_err;
	if (flags.help) {
		print_help();
		if (help_only)
			exit(EXIT_SUCCESS);
		putchar('\n');	// Seperate help page from answer
	}
	if ((expr = simplify(argv[argc - 1], 0)) == NULL) {
		putchar('\n');	// Seperate error message from terminal cursor
		exit(EXIT_FAILURE);
	}
	#if DEBUG
	puts("\n\e[4mresult\e[24m");
	#endif
	result = stod(expr);
	free(expr);
	puts(expr = dtos(result, ndec));	// 6 = default number of decimals shown
	free(expr);
	return EXIT_SUCCESS;

	interactive:
	for (;;) {
		printf("> ");
		expr = calloc(bufsize + 1, sizeof(char));
		getline(&expr, &bufsize, stdin);
		if (!strcmp(expr, "\n")) {
			free(expr);
			break;
		}
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

	invflag_err:
		fail("Invalid flag usage");
	invdec_err:
		fail("Invalid # of decimals");

}

void print_help(void) {
	printf("Usage: %s [EXPRESSION] [ROUND]\n", PROG_NAME);
	puts("High-accuracy terminal calculator");
	puts("Encapsulation within apostrophes (') is recommended");
	puts("This software falls under the GNU Public License v3.0\n");

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