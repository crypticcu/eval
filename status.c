#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "status.h"

int errstat = 0, errln;
char *errfile, *errstr = NULL;
size_t errpos;

/* Error string destructor */
void deststat(void) {
	if (errstr)
		free(errstr);
}

void pstatus(void) {
	char *errmsg = NULL;

	if (cmdln)
		printf("parse: ");
	switch(errstat) {
	case ERR_INTERNAL:	errmsg = "Internal error";			break;
	case ERR_INVFLAG:	errmsg = "Invalid flag usage";		break;
	case ERR_INVDEC:	errmsg = "Invalid # of decimals";	break;
	case ERR_SYNTAX:	errmsg = "Invalid syntax";			break;
	case ERR_OVERFLOW:	errmsg = "Number too large";		break;
	case ERR_MISSOPER:	errmsg = "Missing operand";			break;
	case ERR_DIVZERO:	errmsg = "Divide by zero";			break;
	case ERR_MODULO:	errmsg = "Non-integer modulus";		break;
	case ERR_EVENROOT:	errmsg = "Imaginary result";		break;
	case ERR_INPUTSIZE: errmsg = "Input size too large";	break;
	}
	if (errmsg == NULL)
		return;
	printf("Error: %s", errmsg);
	switch(errstat) {
	case ERR_INTERNAL:
		printf(": %s: %d", errfile, errln);
		if (errno > 0)
			printf(": %s", strerror(errno));
		break;
	case ERR_SYNTAX:
		printf(": ");
		for (size_t index = 0; index < strlen(errstr); index++) {
			if (index == errpos)
				printf("\e[4m");	// Begin underline
			putchar(errstr[index]);
			if (index == errpos)
				printf("\e[24m");	// End underline
		}
		break;
	}
	putchar('\n');
}

int initstat(void) {
	return atexit(deststat);
}