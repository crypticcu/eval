#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "parse.h"

void print_help(void);

int main(int argc, char *argv[]) {
	bool help_shown = false;
	char chr, *expr = NULL;
	unsigned ndec;
	double result;

	if (argc == 1) {
		exit(EXIT_SUCCESS); // Temporary

		/* Print query
		 * Get string
		 * Transfer to simplify()
		 * Get result
		 * Convert result to string
		 * Print result string */
	}
	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) { // Interpret help flag if present
		if (argc == 2)	print_help();
		else			fail("Incorrect flag usage");
	}
	if ((expr = simplify(argv[1], 0)) == NULL)
		fail("Internal error (main.simplify)");
	if (argc == 3) {	// Interpret decimal count if present
		if (strspn(argv[2], VAL_CHRS + 10) == strlen(argv[2]))	ndec = atoi(argv[2]);	/* Decimal count contains only digits	 */
		else													goto invdec_err;		/* VAL_CHRS[11->20] = '1', ..., '9', '0' */
		if (ndec > LDBL_DIG)									goto invdec_err;
	}
	result = stod(expr);
	free(expr);
	puts(expr = dtos(result, argc == 3 ? ndec : 6));	// 6 = default number of decimals shown
	free(expr);
	return EXIT_SUCCESS;
		
	invdec_err:
		fail("Invalid decimal count");
}

void print_help(void) {
	printf("Usage: %s [EXPRESSION] [ROUND]\n", PROG_NAME);
	puts("High-accuracy terminal calulator");
	puts("This software falls under the GNU Public Licence\n");

	puts("++, --     ++x, --x            Prefix increment, decrement");
	puts("!, !!      !x, y!!x            Square root, other root                   ↑ More precedence");
	puts("^          x^y                 Exponent");
	puts("*, /, %    x*y, x/y, x%y       Multiply, divide, remainder               ↓ Less Precedence");
	puts("+, -       +x, -x, x+y, x-y    Unary plus, Unary minus, add, subtract\n");

	puts("GitHub repository: https://github.com/crypticcu/parsr");
	puts("Report bugs to:    cryptic.cu@protonmail.com");
	exit(EXIT_SUCCESS);
}