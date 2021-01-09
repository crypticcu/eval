#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include "parse.h"

int main(int argc, char *argv[]) {
	char *expr;
	double result;

	if (argc < 2) {
		printf("%s: Invalid # of arguments\nTry '%s -h for more information\n'", PROG_NAME, PROG_NAME);
		exit(EXIT_FAILURE);
	}
	if ((expr = simplify(argv[argc - 1], 0)) == NULL)
		errx(EXIT_FAILURE, "Internal error (main.simplify)");
	result = stod(expr);
	printf("%f\n", result);
	free(expr);
	return result;
}