#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "status.h"
#include "util.h"

int ErrStat = 0, ErrLn;
char *ErrFile, *ErrStr = NULL;
size_t ErrPos = 0;

void clrstat(void) {if (ErrStr)	free(ErrStr);}

void pstatus(void) {
	char *errmsg = "Success";

	if ((ErrStat == ERR_SYNTAX || ErrStat == ERR_INVFLAG) && !ErrStr) {	// String not specified
		setstat(ERR_INTERNAL);
		pstatus();
		return;
	}
	if (CmdLn)
		printf("parse: ");
	switch(ErrStat) {
	case ERR_INTERNAL:	errmsg = "Internal error";			break;
	case ERR_INVFLAG:	errmsg = "Unknown flag";			break;
	case ERR_INVARG:	errmsg = "Invalid argument";		break;
	case ERR_INVDEC:	errmsg = "Invalid # of decimals";	break;
	case ERR_SYNTAX:	errmsg = "Invalid syntax";			break;
	case ERR_OVERFLOW:	errmsg = "Number too large";		break;
	case ERR_MISSOPER:	errmsg = "Missing operand";			break;
	case ERR_DIVZERO:	errmsg = "Divide by zero";			break;
	case ERR_MODULO:	errmsg = "Non-integer modulus";		break;
	case ERR_IMAGINARY:	errmsg = "Imaginary result";		break;
	case ERR_INPUTSIZE: errmsg = "Input size too large";	break;
	}
	printf("Error: %s", errmsg);
	switch(ErrStat) {
	case ERR_INTERNAL:
		printf(": %s: %d", ErrFile, ErrLn);
		if (errno > 0)
			printf(": %s", strerror(errno));
		break;
	case ERR_INVFLAG:
		printf(": '%c'", ErrStr[ErrPos]);
		break;
	case ERR_INVARG:
		printf(": " SIZE_FMT, ErrPos);
		break;
	case ERR_SYNTAX:
		printf(": ");
		fprint(ErrStr, ErrPos, ErrPos, F_UND);
		break;
	}
	putchar('\n');
}

void setinv(const char *str, size_t pos) {
	ErrPos = pos;
	if (str) {
		if(ErrStr)
			free(ErrStr);
		if (!(ErrStr = strdup(str)))
			setstat(ERR_INTERNAL);
	}
}