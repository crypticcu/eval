#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "parse.h"

void print_help(void) {
	puts(" TODO ");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
	char chr, *expr = NULL;
	double result;
	bool help_shown = false;
	unsigned ndec;

	if (argc < 2 || argc > 3)
		fail("Invalid # of arguments");
	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) { // Interpret help flag if present
		if (argc == 2)	print_help();
		else			fail("Incorrect flag usage");
	}
	if ((expr = simplify(argv[1], 0)) == NULL)
		fail("Internal error (main.simplify)");
	if (argc == 3) {	// Interpret decimal count if present
		if (strspn(argv[2], VAL_CHRS + 10) == strlen(argv[2]))	ndec = atoi(argv[2]);	/* Decimal count contains only digits	 */
		else													goto invdec_err;					/* VAL_CHRS[11->20] = '1', ..., '9', '0' */
		if (ndec > DBL_DIG)										goto invdec_err;
	}
	result = stod(expr);
	free(expr);
	puts(expr = dtos(result, argc == 3 ? ndec : 6));	// 6 = default number of decimals shown
	free(expr);
	return result;
		
	invdec_err:
		fail("Invalid decimal count");
}