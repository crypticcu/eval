#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

/* nchr = index position
 * obstruction = invalid character in-between operator and operand
 * limit = furthest right- or left-hand index position of operation */



void fail(const char *desc) {
	if (cmd_line)
		printf("%s: ", PROG_NAME);
	printf("%s", desc);
	putchar('\n');
	if (cmd_line)	// Exit program if using command-line
		exit(EXIT_FAILURE);
}

void print_help(void) {
	printf("Usage: %s [FLAGS] [EXPRESSION]    Command-line\n", PROG_NAME);
	printf("       %s                         Interactive \n", PROG_NAME);
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

void printu(const char *string, size_t pos) {
	for (int nchr = 0; nchr < strlen(string); nchr++) {
		if (nchr == pos)
			printf("\e[4m");	// Begin underline
		putchar(string[nchr]);
		if (nchr == pos)
			printf("\e[24m");	// End underline
	}
}

char *dtos(double x, unsigned sig) {	//~ Dynamic memory: numstr
	unsigned nwhole;

	nwhole = nplaces(x);
	if (nwhole > DBL_DIG)
		return STR_OVER;
	if (nwhole + sig > DBL_DIG)
		sig = DBL_DIG - nwhole;
	if (sig > DBL_DIG)	// Decimal place exceeds accurate number allotted by system
		sig = DBL_DIG;

	bool is_negative = x < 0, is_decimal = sig, only_decimal = x < 1 && x > -1, only_whole = isequal(x, (long long) x);
	unsigned reqsize = nwhole + sig + is_negative + is_decimal + only_decimal;
	char *numstr = (char *) calloc(reqsize + 1, sizeof(char));

	if (numstr == NULL)	// Allocation fails
		return NULL;
	if (is_negative) {	// Negative and decimal requirements
		numstr[0] = '-';
		if (only_decimal) {
			numstr[1] = '0';
			numstr[2] = '.';
		}	
	} else if (only_decimal) {
		numstr[0] = '0';
		numstr[1] = '.';
	}
	for (size_t nchr = is_negative + only_decimal * 2, place = nplaces(x) - !(nwhole == DBL_DIG && only_whole); nchr < reqsize; nchr++, place--) {	// Skip characters reserved for negative sign and decimal point, if present
		numstr[nchr] = getdigit(x, place) + 48;	// '0' = 48
		if (place == 0) {
			if (only_whole)
				break;
			else if (nchr + 1 != reqsize)
				numstr[++nchr] = '.';
		}
	}
	return numstr;
}

char *popsub(const char *string, size_t low, size_t high) { // Dynamic memory: sub
	char *sub;	// Substring to be 'popped' from string

	if (low >= strlen(string) || high >= strlen(string) || low > high ||	// Low/high indices exceed range of string || Low index is greater than high || Allocation fails
	(sub = (char *) calloc(high - low + 2, sizeof(char))) == NULL)
		return NULL;
	for (size_t nchr_old = low, nchr_new = 0; nchr_old <= high; nchr_old++, nchr_new++)
		sub[nchr_new] = string[nchr_old];
	return sub;
}

char *pushsub(char *string, char *sub, size_t low, size_t high) {	// Dynamic memory: newstr
	char *newstr = (char *) calloc(
		strlen(string)				// Original length
		- (high - low + 1)	// Take away number of characters being removed
		+ strlen(sub)			// Add size of substring
		+ 1						// Add space for null character
	, sizeof(char));
	size_t nchr_new;

	if (low >= strlen(string) || high >= strlen(string) || low > high || newstr == NULL) // Low/high indices exceed range of string || Low index is greater than high || Allocation fails
		return NULL;
	for (nchr_new = 0; nchr_new < low; nchr_new++)
		newstr[nchr_new] = string[nchr_new];	// Add contents of old string up to point of integration
	for (size_t nchr_sub = 0; nchr_sub < strlen(sub); nchr_sub++, nchr_new++)
		newstr[nchr_new] = sub[nchr_sub];	// Integrate substring
	for (size_t nchr_old = high + 1; nchr_old < strlen(string); nchr_old++, nchr_new++)
		newstr[nchr_new] = string[nchr_old];	// Add rest of old string
	free(string);
	free(sub);
	return newstr;
}

char *simplify(const char *expr, size_t from) {	// Dynamic memory: subA, subB, modexpr
	bool read_parenth = false;
	char chr, *subA = NULL, *subB = NULL,
		*modexpr = (char *) calloc(strlen(expr) + 1, sizeof(char));
	size_t par_low, par_high;
	long long invpos;
	double result;

	if (modexpr == NULL)
		return NULL;
	if (from == 0) {	// Check syntax and parenthesese only once (_from is always 0 on first call)
		if ((invpos = chk_syntax(expr)) != CHK_PASS)	// Check for syntax errors
			goto syntax_err;
		if ((invpos = chk_parenth(expr)) != CHK_PASS)	// Check for parenthetical errors
			goto syntax_err;
	}
	strcpy(modexpr, expr); // Copy constant expression to modifiable one
	for (size_t nchr = from; (chr = modexpr[nchr]); nchr++) {	// Go straight to evaluate() if '-p' is passed
		if (chr == ')') {
			par_high = nchr;
			read_parenth = false;
			if ((subA = popsub(modexpr, par_low, par_high)) == NULL)
				goto popsub_err;
			modexpr[par_low] = (toast(modexpr, par_low)) ? '*' : ' ';
			modexpr[par_high] = (toast(modexpr, par_high)) ? '*' : ' ';
			result = evaluate(subA);	// Value passed to result to increase efficiency and improve debugging mode clarity
			if (isequal(result, DBL_FAIL))	// evaluate() does not return heap address, so can be called without assignment
				goto evaluate_err;
			if ((subB = dtos(result, DBL_DIG)) == NULL)
				goto dtos_err;
			free(subA);
			if ((modexpr = pushsub(modexpr, subB, par_low + 1, par_high - 1)) == NULL)	// Do not overwite space where parentheses used to be
				goto pushsub_err;
			if (from != 0)
				return modexpr;
		}
		else if (chr == '(') {
			if (read_parenth) {
				subA = modexpr;	//~ Swap causes 'still reachable' error in valgrind
				if ((modexpr = simplify(modexpr, nchr)) == NULL)
					goto simplify_err;
				free(subA);
			} else {
				read_parenth = true;
				par_low = nchr;
			}
		}
	}
	subA = modexpr;
	result = evaluate(modexpr);
	if (isequal(result, DBL_FAIL))
		goto evaluate_err;
	if ((modexpr = dtos(result, DBL_DIG)) == NULL)
		goto dtos_err;
	#if DEBUG
	putchar('\n');
	#endif
	free(subA);
	return modexpr;

	syntax_err:
		printf("Syntax error: ");
		printu(expr, invpos);
		if (!cmd_line)
			putchar('\n');
		CLEANUP(modexpr, NULL);

	pushsub_err:	fail("Internal error (simplify.pushsub)");	CLEANUP(NULL, NULL);
	evaluate_err:	/* Error message already printed */			CLEANUP(subA, NULL);
	dtos_err:		fail("Internal error (simplify.dtos)");		CLEANUP(subA, NULL);
	simplify_err:	fail("Internal error (simplify.simplify)");	CLEANUP(subA, NULL);
	popsub_err:		fail("Internal error (simplify.dtos)");		CLEANUP(modexpr, NULL);
}

bool isequal(double x, double y) {
	return fabs(x - y) < FLT_EPSILON;
}

bool isnumer(char chr) {
	return (isdigit(chr) || chr == '.');
}

bool toast(const char *expr, size_t parpos) {	// To asterisk?
	char parenth = expr[parpos],
		 last;	// Non-space character before parenthesis

	if (parpos < strlen(expr) - 1 && parpos) {
		for (size_t i = 1; isspace(last = expr[parpos - i]); i++);
		return !strchr(chrsets.opers, last);
	} else
		return false;
}

unsigned getdigit(double x, int place) {
	x = fabs(x);
	if (abs(place) > DBL_DIG || x > LLONG_MAX)	// Place cannot be over/under place limit; Any 'x' over LLONG_MAX causes overflow on conversion
		return 0;								// Digits that cannot be printed
	return (long long) (x *= pow(10, -place)) - (long long) (x / 10) * 10;
}

unsigned nplaces(double x) {
	unsigned n = 0;

	if (isnan(x) || isinf(x))
		return UINT_MAX;
	for (n; (long long) x != 0; x /= 10, n++);
	return n;	
}

int fobst(const char *expr, size_t operpos, size_t llim, size_t rlim) {
	char chr, oper = expr[operpos];
	int nchr = operpos,
		off = 0,	// Offset from operator position
		dir = 0;	// Direction obstruction is in

	if (llim == INT_FAIL)	// Left limit of unary operation is operator position
		llim = 0;
	if (rlim == INT_FAIL)
		rlim = strlen(expr) - 1;
	if (llim >= strlen(expr) || rlim >= strlen(expr) || llim > rlim)
		return INT_FAIL;
	while (off < (int) strlen(expr)) {
		if (nchr >= llim && nchr <= rlim) {
			chr = expr[nchr];
			if (strchr(chrsets.opers, chr) && chr != oper && chr != '-')
				dir |= off < 0 ? LEFT : RIGHT;
		}
		off <= 0 ? (off = -(off - 1)) : (off = -off);
		nchr = operpos + off;
	}
	return dir;
}

long long chk_parenth(const char *expr) {
	char chr;
	size_t nopen = 0, nclosed = 0, nchr;

	for (nchr = 0; (chr = expr[nchr]); nchr++)	// Get number of closed parentheses
		if (chr == ')')
			nclosed++;
	for (nchr = 0; (chr = expr[nchr]); nchr++) {
		if (chr == '(')
			nopen++;
		else if (chr == ')')
			nopen--;
		if (nopen > nclosed ||	// Extra open parenthesis?
			nopen < 0) {		// Extra closed?
			return nchr;
		}
	}
	return CHK_PASS;
}

long long chk_syntax(const char *expr) {
	char chr,
		 lead = 0,	// Last non-space 
		 trail = 0,	// Next non-space
		 last = 0,	// Immediate last
		 next = 1,	// Immediate next
		 doubl;		// Current double oeprator
	size_t nsingle = 0,	// Single operators
		   ndouble = 0,	// Double operators
		   npoint = 0,	// Decimal points
		   pcount = 0;	// Number parsed in current expression

	#if DEBUG
	puts("\e[4mchk_syntax\e[24m");
	#endif
	for (size_t nchr = 0; (chr = expr[nchr]); nchr++) {
		#if DEBUG
		printf("single: %ld\tdouble: %ld\tchr: %c\n", nsingle, ndouble, chr);
		#endif
		if (nchr != 0)
			last = expr[nchr - 1];
		if (nchr != strlen(expr))
			next = expr[nchr + 1];
		if (next != '\0')
			for (int i = nchr + 1; expr[i]; i++)
				if(!isspace(expr[i])) {
					trail = expr[i];
					break;
				}
		if (isdigit(chr) || chr == '(' || chr == ')') {
			nsingle = 0, ndouble = 0;
			if (chr == '(')
				pcount = -1;
		}
		else if (strchr(chrsets.doubl, chr) && (chr == last || chr == next )) {
			if (chr != doubl && pcount)
				return nchr;
			doubl = chr;
			ndouble++;
		}
		else if (strchr(chrsets.opers, chr))
			nsingle++;	
		if (!isdigit(chr) && chr != '.') // CHECK DECIMAL POINTS 
			npoint = 0;
		else if (chr == '.')
			npoint++;
		if (nsingle == 2 || ndouble == 3 ||	npoint == 2  ||	/* Extra operator or comma ||			   */	// CHECK ERRORS
			!strchr(chrsets.valid, chr) && !isspace(chr) ||	/* Is not a valid character nor a space || */
			isdigit(chr) && isdigit(lead) && lead != last)	/* Two numbers side-by-side w/o operator   */
			return nchr;
		if (!isspace(chr))
			lead = chr;
		pcount++;
	}
	return CHK_PASS;
}

long long getlim(char *expr, size_t operpos, char dir) {
	bool reading = false;
	char chr;
	long nchr;	// Left intentionally signed
	long long lim = -1;

	if (dir != 'l' && dir != 'r')	// Left and right directions only
		return INT_FAIL;
	for (nchr = dir == 'r' ? operpos + 1 : operpos - 1; (chr = expr[nchr]) && nchr >= 0; dir == 'r' ? nchr++ : nchr--) {
		if (dir == 'r') {
			if (reading && !isnumer(chr)) {
				lim = nchr - 1;
				break;
			} else if (!reading && (isnumer(chr) || chr == '+' || chr == '-'))
				reading = true;
		} else {
			if (reading && !isnumer(chr)) {
				lim = nchr + !(chr == '+' || chr == '-');
				break;
			} else if (!reading && isnumer(chr))
				reading = true;
		}
	}
	if (!reading)	// No value found
		return INT_FAIL;
	else {
		if (nchr == -1)	// Reached beginning of expression
			lim = 0;
		else if (chr == 0)	// Reached end of expression
			lim = strlen(expr) ? strlen(expr) - 1 : strlen(expr);
	}
	return lim;
}

double evaluate(const char *expr) {	// Dynamic memory: result_str, modexpr
	char chr,
		*result_str = NULL,	// Operation result
		*modexpr = (char *) calloc(strlen(expr) + 1, sizeof(char));	// Modifiable expression
	size_t llim, rlim,	// Left- and right-hand limits of operation
		   nchr, ignore;
	double result,	// Operation result;	Final return value
		   lval, rval;	// Left and right values of operation

	#if DEBUG
	printf("\n\e[4mevaluate\e[24m\n%s\n", expr);
	#endif
	if (modexpr == NULL)
		return DBL_FAIL;
	strcpy(modexpr, expr);	// Copy constant expression to modifiable one
	for (nchr = 0; (chr = modexpr[nchr]); nchr++) {
		if (strchr(chrsets.opers, chr))
			goto evaluate;
	}
	goto reevaluate;	// Skip main loop if no operators are found

	evaluate:
	for (nchr = 0; (chr = modexpr[nchr]); nchr++) {	// INCREMENT/DECREMENT
		if (chr == '+' && modexpr[nchr + 1] == '+' || chr == '-' && modexpr[nchr + 1] == '-') {
			INIT_VALS();	// Retrieves rval, lval, rlim, and llim
			CHK_VALS(RIGHT);	// Checks for overflow, getval() failure, missing operand(s), and obstructions
			result = chr == '+' ? rval + 1	// '++x' -> '+(x + 1)' 
							   : -rval - 1;	// '--x' -> '-(-x + 1)'
			llim = nchr;	// Left limit of unary operation is operator position	
			INIT_EXPR();	// Retrieves new expression
			#if DEBUG
			puts(expr);
			#endif
		}
	}
	for (nchr = 0; (chr = modexpr[nchr]); nchr++) {	// SQUARE ROOT/OTHER ROOT
		if (chr == '!') {
			INIT_VALS();
			if (modexpr[nchr + 1] == '!') {
				CHK_VALS(LEFT|RIGHT);
				if (rval < 0 && (long long) lval % 2 == 0)
					goto evenroot_err;
				if (lval == 0)
					goto zeroroot_err;
				result = rval < 0 ? -pow(-rval, 1 / lval)	// Negative root workaround
									: pow(rval, 1 / lval);	
			} else {
				CHK_VALS(RIGHT);
				if (rval < 0)
					goto evenroot_err;
				llim = nchr;
				result = sqrt(rval);
			}
			INIT_EXPR();
			#if DEBUG
			puts(expr);
			#endif
		}
	}
	for (nchr = 0; (chr = modexpr[nchr]); nchr++) {	// EXPONENT
		if (chr == '^') {
			INIT_VALS();
			CHK_VALS(LEFT|RIGHT);
			result = pow(lval, rval);
			INIT_EXPR();
			#if DEBUG
			puts(expr);
			#endif
		}
	}
	for (nchr = 0; (chr = modexpr[nchr]); nchr++) {	// MULTIPLICATION/DIVISION/REMAINDER
		if (chr == '*' || chr == '/' || chr == '%') {
			INIT_VALS();
			CHK_VALS(LEFT|RIGHT);
			if (rval == 0 && chr != '*')
				goto divzero_err;
			if (chr == '*')
				result = lval * rval;
			else if (chr == '/')
				result = lval / rval;
			else if (chr == '%') {
				if (isequal(lval, (long long) lval) && isequal(rval, (long long) rval))
					result = (long long) lval % (long long) rval;
				else
					goto modulus_err;
			}
			INIT_EXPR();
			#if DEBUG
			puts(expr);
			#endif
		}
	}
	for (nchr = 0; (chr = modexpr[nchr]); nchr++) {	// ADDITION/SUBTRACTION/UNARY PLUS/UNARY MINUS
		if (chr == '+' || chr == '-') {
			if (chr == '+' && modexpr[nchr + 1] == '+' || chr == '-' && modexpr[nchr + 1] == '-')	// Increment/Decrement found
				goto evaluate;
			INIT_VALS();
			CHK_VALS(RIGHT);
			if (llim == INT_FAIL)
				llim = nchr;
			result = chr == '+' ? lval + rval : lval - rval;
			INIT_EXPR();
			#if DEBUG
			puts(expr);
			#endif
		}
	}

	reevaluate:
	ignore = strspn(modexpr, "( -");
	while (strcspn(modexpr + ignore, chrsets.opers) != strlen(modexpr + ignore)) {	// Ignore negative/positive sign
		if ((result_str = dtos(evaluate(modexpr), DBL_DIG)) == NULL)
			goto dtos_err;
		if (result_str == STR_OVER)
			goto overflow_err;
		free(modexpr);
		modexpr = result_str;
	}

	if (isequal(result = stod(modexpr), DBL_OVER))
		goto overflow_err;
	free(modexpr);
	return result;

	opermiss_err:	fail("Missing operand");					CLEANUP(modexpr, DBL_FAIL);
	evenroot_err:	fail("Even root of negative number");		CLEANUP(modexpr, DBL_FAIL);
	zeroroot_err:	fail("Root cannot be zero");				CLEANUP(modexpr, DBL_FAIL);
	divzero_err:	fail("Divide by zero");						CLEANUP(modexpr, DBL_FAIL);
	modulus_err:	fail("Remainder takes integers only");		CLEANUP(modexpr, DBL_FAIL);
	getval_err:		fail("Internal error (evaluate.getval)");	CLEANUP(modexpr, DBL_FAIL);
	dtos_err:		fail("Internal error (evaluate.dtos)");		CLEANUP(modexpr, DBL_FAIL);
	overflow_err:	fail("Number too large");					CLEANUP(modexpr, DBL_FAIL);
	pushsub_err:	fail("Internal error (evaluate.pushsub)");	CLEANUP(result_str, DBL_FAIL);
}

double getval(char *expr, size_t operpos, char dir) {	// Dynamic memory: valstr
	bool reading = false;
	char chr, *valstr = NULL;
	size_t val_low, val_high;
	long nchr;	// Left intentionally signed
	double val;

	if (dir != 'l' && dir != 'r')	// Left and right directions only
		return DBL_FAIL;
	for (nchr = dir == 'r' ? operpos + 1 : operpos - 1; (chr = expr[nchr]) && nchr >= 0; dir == 'r' ? nchr++ : nchr--) {	// Get size of value string
		if (dir == 'r') {
			if (reading && !isnumer(chr)) {
				val_high = (nchr) ? nchr - 1 : nchr;
				break;
			} else if (!reading && (isnumer(chr) || chr == '+' || chr == '-')) {
				reading = true;
				val_low = nchr;
			}
		} else {
			if (reading && !isnumer(chr)) {
				val_low = nchr + !(chr == '+' || chr == '-');
				break;
			} else if (!reading && isnumer(chr)) {
				reading = true;
				val_high = nchr;
			}
		}
	}
	if (!reading)	// No value found
		return 0;	// Must return 0 in this case to ensure proper evaluate() functionality (used in unary + and -)
	else if (nchr == -1)	// Reached beginning of expression
		val_low = 0;
	else if (chr == 0)	// Reached end of expression
		val_high = strlen(expr) - 1;
	if ((valstr = popsub(expr, val_low, val_high)) == NULL) {
		free(expr);
		return DBL_FAIL;
	}
	if ((val = stod(valstr)) == DBL_OVER) {
		free(valstr);
		return DBL_OVER;
	}
	free(valstr);
	return val;	// Convert value string to value
}

double stod(const char *string) {	// Equivalent to atof(), except that it does not print inaccurate numbers
	bool read_decim = false, is_negative = false, reading = false;
	char chr;
	size_t nchr;
	double num = 0, placeval = 0.1;

	for (nchr = 0; (chr = string[nchr]) && nchr <= DBL_DIG + is_negative + read_decim; nchr++) {	// DBL_DIG is accurate digit limit
		if (chr == '-' && !is_negative && !reading)
			is_negative = true;
		else if (chr == '.' && !read_decim && !reading)
			read_decim = true;
		if (!read_decim && isdigit(chr)) {
			num *= 10;
			num += chr - 48;
		} else if (isdigit(chr)) {
			reading = true;
			num += placeval * (chr - 48);	// '0' = 48
			placeval /= 10;
		}
	}
	if (nchr > DBL_DIG && !read_decim)
		return DBL_OVER;
	if (is_negative)
		num *= -1;
	return num;
}