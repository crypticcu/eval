#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eval.h"

bool CMD_LINE; // Using command-line interface?

void print_help(void);

int main(int argc, char *argv[]) {
	char *expr = NULL, *swap = NULL;
	size_t bufsize = 999,	// Maximum input size
	ndec;					// Number of decimal places
	double result;

	CMD_LINE = true;
	if (argc == 1) {
		CMD_LINE = false;
		goto interactive;
	}

	command_line:
	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {	// Interpret help flag if present
		if (argc == 2)
			print_help();
		else
			fail("Incorrect flag usage");
	}
	if ((expr = simplify(argv[1], 0)) == NULL) {
		putchar('\n');
		exit(EXIT_FAILURE);
	}
	if (argc == 3) {	// Interpret decimal count if present
		if (strspn(argv[2], VAL_CHRS + 10) == strlen(argv[2]))	/* Decimal count contains only digits	 */
			ndec = atoi(argv[2]);								/* VAL_CHRS[11->20] = '1', ..., '9', '0' */
		else
			goto invdec_err;
		if (ndec > DBL_DIG)
			goto invdec_err;
	}
	#ifdef DEBUG
	puts("\n\e[4mresult\e[24m");
	#endif
	result = stod(expr);
	free(expr);
	puts(expr = dtos(result, argc == 3 ? ndec : 6));	// 6 = default number of decimals shown
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

	invdec_err:
		fail("Invalid decimal count");
}

void print_help(void) {
	printf("Usage: %s [EXPRESSION] [ROUND]\n", PROG_NAME);
	puts("High-accuracy terminal calulator");
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
	exit(EXIT_SUCCESS);
}