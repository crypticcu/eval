#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <unistd.h>
#include "parse.h"

/* Leading underscore signifies parameter */
/* nchr ~ index position				  */

void fail(const char *_desc) {
	printf("%s: %s\n", PROG_NAME, _desc);
	exit(EXIT_FAILURE);
}

void printh(const char *_s, unsigned _hpos) {
	for (int i = 0; i < strlen(_s); i++) {
		if (i == _hpos)
			printf("\033[31m");	// Red char color escape code
		else if (i == _hpos + 1)
			printf("\033[0m");
		putchar(_s[i]);
	}
	printf("\033[0m"); // Reset formatting in case character is at end of string
}

char *dtos(double _x, unsigned _sig) {	// Dynamic memory: numstr
	bool is_negative = _x < 0, is_decimal = _sig, only_decimal = _x < 1 && _x > -1, only_whole = isequal(_x, (int) _x);
	unsigned nwhole = nplaces(_x), reqsize = nwhole + _sig + is_negative + is_decimal + only_decimal;
	char *numstr = (char *) calloc(reqsize + 1, sizeof(char));

	if (_sig > DBL_DIG || numstr == NULL)	// Decimal place exceeds accurate number allotted by system || Allocation fails
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
	for (int i = is_negative + only_decimal * 2, place = nplaces(_x) - 1; i < reqsize; i++, place--) {	// 'i' skips characters reserved for negative sign and decimal point, if present
		numstr[i] = getdigit(_x, place) + 48; // '0' = 48
		if (place == 0) {
			if (only_whole)
				break;
			else if (i + 1 != reqsize)
				numstr[++i] = '.';
		}
	}
	return numstr;
}

char *popsub(const char *_s, unsigned _low, unsigned _high) { // Dynamic memory: sub
	char *sub;	// Substring to be 'popped' from string
	if (_low >= strlen(_s) || _high >= strlen(_s) || _low > _high ||	// Low/high indices exceed range of string || Low index is greater than high || Allocation fails
	(sub = (char *) calloc(_high - _low + 2, sizeof(char))) == NULL)
		return NULL;
	for (unsigned nchr_old = _low, nchr_new = 0; nchr_old <= _high; nchr_old++, nchr_new++)
		sub[nchr_new] = _s[nchr_old];
	return sub;
}

char *pushsub(char *_s, char *_sub, unsigned _low, unsigned _high) { // Dynamic memory: newstr
	char *newstr = (char *) calloc(
		strlen(_s)				// Original length
		- (_high - _low + 1)	// Take away number of characters being removed
		+ strlen(_sub)			// Add size of substring
		+ 1						// Add space for null character
	, sizeof(char));
	unsigned nchr_new;

	if (_low >= strlen(_s) || _high >= strlen(_s) || _low > _high || newstr == NULL) // Low/high indices exceed range of string || Low index is greater than high || Allocation fails
		return NULL;								 
	for (nchr_new = 0; nchr_new < _low; nchr_new++)
		newstr[nchr_new] = _s[nchr_new];	// Add contents of old string up to point of integration
	for (unsigned nchr_sub = 0; nchr_sub < strlen(_sub); nchr_sub++, nchr_new++)
		newstr[nchr_new] = _sub[nchr_sub];	// Integrate substring
	for (unsigned nchr_old = _high + 1; nchr_old < strlen(_s); nchr_old++, nchr_new++)
		newstr[nchr_new] = _s[nchr_old];	// Add rest of old string
	free(_s);
	free(_sub);
	return newstr;
}

char *simplify(const char *_expr, unsigned _from) {	// Dynamic memory: subA, subB, expr
	char chr, *subA = NULL, *subB = NULL;
	char *expr = (char *) calloc(strlen(_expr) + 1, sizeof(char)); // Modifiable expression
	bool read_parenth = false;
	unsigned par_low, par_high, invpos;
	double result;
	if (expr == NULL)
		return NULL;
	if (!_from) { //Only use on first function call, in which '_from' is always zero
		if (chk_syntax(_expr) != CHK_PASS) { // Check for syntax errors; Ignore if '-s' is passed
			invpos = chk_syntax(_expr);
			goto syntax_err;
		}
		if (chk_parenth(_expr) != CHK_PASS) { // Check for parenthetical errors; Ignore if '-p' is passed
			invpos = chk_parenth(_expr);
			goto syntax_err;
		}
	}
	strcpy(expr, _expr); // Copy constant expression to modifiable one
	for (unsigned nchr = _from; (chr = expr[nchr]); nchr++) {	// Go straight to evaluate() if '-p' is passed
		if (chr == ')') {
			par_high = nchr;
			read_parenth = false;
			if ((subA = popsub(expr, par_low, par_high)) == NULL) {
				free(expr);
				return NULL;
			}
			expr[par_low] = (toast (expr, par_low)) ? '*' : ' ';
			expr[par_high] = (toast (expr, par_high)) ? '*' : ' ';
			result = evaluate(subA);	// Value passed to result to increase efficiency and improve debuggiing mode clarity
			if (isequal(result, DBL_FAIL))	// evaluate() does not return heap address, so can be called without assignment
				goto evaluate_err;
			if ((subB = dtos(result, DBL_DIG)) == NULL)
				goto dtos_err;
			free(subA);
			if ((expr = pushsub(expr, subB, par_low + 1, par_high - 1)) == NULL)	// Do not overwite space where parentheses used to be
				goto pushsub_err;
			if (_from)
				return expr;
		}
		else if (chr == '(') {
			if (read_parenth) {
				subA = expr; // Swap causes 'still reachable' error in valgrind
				if ((expr = simplify(expr, nchr)) == NULL)
					goto simplify_err;
				free(subA);
			} else {
				read_parenth = true;
				par_low = nchr;
			}
		}
	}
	subA = expr; // Swap causes 'still reachable' error in valgrind
	result = evaluate(expr);
	if (isequal(result, DBL_FAIL))
		goto evaluate_err;
	if ((expr = dtos(result, DBL_DIG)) == NULL)
		goto dtos_err;
	free(subA);
	return expr;

	syntax_err:
		printf("%s: Syntax error: ", PROG_NAME);
		printh(_expr, invpos);
		putchar('\n');
		free(expr);
		exit(EXIT_FAILURE);
	evaluate_err:
		free(subA);
		free(expr);
		fail("Internal error (simplify.evaluate)");
	dtos_err:
		free(subA);
		fail("Internal error (simplify.dtos)");
	pushsub_err:
		fail("Internal error (simplify.pushsub)");
	simplify_err:
		free(subA);
		fail("Internal error (simplify.simplify)");
}

bool isequal(double _x, double _y) {
	return fabs(_x - _y) < FLT_EPSILON;
}

bool isin(const char _x, const char *_y) {
	for (int i = 0; i < strlen(_y); i++)
		if (_x == _y[i])
			return true;
	return false;
}

bool isnumer(char _c) {
	return (isdigit(_c) || _c == '-' || _c == '.');
}

unsigned nplaces(double _x) {
	_x = fabs(_x);	// log of negative is undefined
	if (_x == 0)	// log of zero is undefined
		return 1;
	return log(_x)/log(10) + 1;
}

bool toast(const char *_expr, unsigned _parpos) {
	char chr = _expr[_parpos],
		 next = _expr[_parpos + 1],
		 last = _expr[_parpos - 1];

	return (isdigit(last) && isnumer(next)	||
		chr == '(' && last == ')'			||
		chr == ')' && next == '(') ? true : false;
}

int chk_syntax(const char *_expr) {
	char chr;
	int nsingle = 0, ndouble = 0;

	for (int i = 0; (chr = _expr[i]); i++) {
		if (isdigit(chr) || chr == '(') // Reset single/double operator count
			nsingle = 0, ndouble = 0;
		else if (isin(chr, OPERS_S))
			nsingle++;
		else if (isin(chr, OPERS_D) && (chr == _expr[i - 1] || chr == _expr[i + 1]))
			ndouble++;
		if (nsingle == 2 || ndouble == 3 ||			/* Extra operator ||					*/
			!isin(chr, VAL_CHRS) && !isspace(chr))	/* Is not a valid character nor a space */
			return i;
	}
	return CHK_PASS;
}

int chk_parenth(const char *_expr) {
	char chr;
	int nopen = 0, nclosed = 0;

	for (int i = 0; (chr = _expr[i]); i++)	// Get number of closed parentheses
		if (chr == ')')
			nclosed++;
	for (int i = 0; (chr = _expr[i]); i++) {
		if (chr == '(')
			nopen++;
		else if (chr == ')')
			nopen--;
		if (nopen > nclosed) { // Extra open parenthesis?
			while ((chr = _expr[--i]) != '('); // Find last instance of open parenthesis
			return i;
		}
		if (nopen < 0)	// Extra closed?
			return i;
	}
	return CHK_PASS;
}

int getdigit(double _x, int _place) {
	int digit;

	_x = fabs(_x);
	if (abs(_place) > DBL_DIG || _x > LLONG_MAX)	// Place cannot be over/under place limit; Any 'x' over max llong causes overflow on conversion
		return 0;
	for (int i = 0; i <= abs(_place); _place > 0 ? (_x /= 10) : (_x *= 10), i++)
		digit = ((long long int) _x - (long long int) (_x / 10) * 10);
	return digit;
}

int getlim(char *_expr, unsigned _operpos, char _dir) {
	int lim = -1, i;
	char chr;
	bool reading = false, read_digit;

	if (_dir != 'l' && _dir != 'r') // Left and right directions only
		return INT_FAIL;
	for (i = _dir == 'r' ? _operpos + 1 : _operpos - 1; (chr = _expr[i]) && i >= 0; _dir == 'r' ? i++ : i--) {
		if (isnumer(chr) && !reading)
			reading = true;
		else if (!isnumer(chr) && reading) {
			lim = _dir == 'r' ? i - 1 : i + 1;
			break;
		}
		if (isdigit(chr))
			read_digit = true;
	}
	if (!reading || !read_digit) // No value found
		return INT_FAIL;
	else {
		if (i == -1) // Reached beginning of expression
			lim = 0;
		else if (chr == 0) // Reached end of expression
			lim = strlen(_expr) - 1;
	}
	return lim;
}

unsigned fobst(const char *_expr, unsigned _operpos, unsigned _llim, unsigned _rlim) {
	char chr, oper = _expr[_operpos];
	int off = 0; // Offset from operator position
	int nchr = _operpos;
	bool l_obstr = false, r_obstr = false;
	if (_llim == INT_FAIL)
		_llim = 0;
	if (_rlim == INT_FAIL)
		_rlim = strlen(_expr) - 1;
	if (_llim >= strlen(_expr) || _rlim >= strlen(_expr) || _llim > _rlim)
		return INT_FAIL;
	while (off < (int) strlen(_expr)) {
		if (nchr >= _llim && nchr <= _rlim) {
			chr = _expr[nchr];
			if (isin(chr, OPERS_A) && chr != oper)
				off < 0 ? (l_obstr = true) : (r_obstr = true);
		}
		off <= 0 ? (off = -(off - 1)) : (off = -off);
		nchr = _operpos + off;
	}
	if (l_obstr && r_obstr)
		return OBS_L | OBS_R;
	else if (l_obstr && !r_obstr)
		return OBS_L;
	else if (!l_obstr && r_obstr)
		return OBS_R;
	else
		return 0;
}

double evaluate(const char *_expr) {	// Dynamic memory: result_str, expr
	char chr,
		*result_str = NULL, // Operation result
		*expr = (char *) calloc(strlen(_expr) + 1, sizeof(char)); // Modifiable expression
	double result, // Operation result;	Final return value
		   lval, rval; // Left and right values of operation
	unsigned llim, rlim; // Left- and right-hand limits of operation

	if (expr == NULL)
		return DBL_FAIL;
	strcpy(expr, _expr); // Copy constant expression to modifiable one

	again:
	for (unsigned nchr = 0; (chr = expr[nchr]); nchr++) {	// Increment/decrement
		if (chr == '+' && expr[nchr + 1] == '+' || chr == '-' && expr[nchr + 1] == '-') {
			INIT_VALS();	// Retrieves rval, lval, rlim, and llim
			if (fobst(expr, nchr, llim, rlim) & OBS_R)	// Operation cannot continue if another operator is in the way
				continue;
			if (rlim == INT_FAIL)
				goto opermiss_err;
			result = chr == '+' ? rval + 1 : -rval - 1;					/* If increment: '++x' -> '+(x + 1)'  */
			llim = nchr;	// Left limit of operation is operator		/* If decrement: '--x' -> '-(-x + 1)' */
			INIT_EXPR();	// Retrieves new expression
			if (DEBUG)
				puts(expr);
		}
	}
	for (unsigned nchr = 0; (chr = expr[nchr]); nchr++) { // Unary root/Binary root
		if (chr == '!') {
			INIT_VALS();
			if (expr[nchr + 1] == '!') {
				if (fobst(expr, nchr, llim, rlim) & (OBS_L & OBS_R)) // Needs both sides of operator
					continue;
				if (rlim == INT_FAIL || llim == INT_FAIL)
					goto opermiss_err;
				if (rval < 0 && (int) lval % 2 == 0)
					goto evenroot_err;
				if (lval == 0)
					goto zeroroot_err;
				result = rval < 0 ? -pow(-rval, 1 / lval) : pow(rval, 1 / lval);	// Negative root workaround
			} else {
				if (fobst(expr, nchr, llim, rlim) & OBS_R)	// Needs only the right side of operator
					continue;
				if (rlim == INT_FAIL)
					goto opermiss_err;
				if (rval < 0)
					goto evenroot_err;
				llim = nchr;
				result = sqrt(rval);
			}
			INIT_EXPR();
			if (DEBUG)
				puts(expr);
		}
	}
	for (unsigned nchr = 0; (chr = expr[nchr]); nchr++) { // Exponent
		if (chr == '^') {
			INIT_VALS();
			if (fobst(expr, nchr, llim, rlim) & (OBS_L & OBS_R))
				continue;
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
			if (rlim == INT_FAIL || llim == INT_FAIL)
				goto opermiss_err;
			result = pow(lval, rval);
			INIT_EXPR();
			if (DEBUG)
				puts(expr);
		}
	}
	for (unsigned nchr = 0; (chr = expr[nchr]); nchr++) { // Multiplication/Division/Remainder
		if (chr == '*' || chr == '/' || chr == '%') {
			INIT_VALS();
			if (fobst(expr, nchr, llim, rlim) & (OBS_L & OBS_R))
				continue;
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
			if (rlim == INT_FAIL || llim == INT_FAIL)
				goto opermiss_err;
			if (rval == 0 && chr != '*')
				goto divzero_err;
			if (chr == '*')
				result = lval * rval;
			else if (chr == '/')
				result = lval / rval;
			else if (chr == '%') {
				if (isequal(lval, (int) lval) && isequal(rval, (int) rval))
					result = (int) lval % (int) rval;
				else
					goto modulus_err;
			}
			INIT_EXPR();
			if (DEBUG)
				puts(expr);
		}
	}
	for (unsigned nchr = 0; (chr = expr[nchr]); nchr++) { // Addition/Subtraction/Unary plus/Unary minus
		if (chr == '+' || chr == '-') {
			if (chr == '+' && expr[nchr + 1] == '+' || chr == '-' && expr[nchr + 1] == '-')	// Increment/Decrement found
				goto again;
			INIT_VALS();
			if (fobst(expr, nchr, llim, rlim) & OBS_R)
				continue;
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
			if (rlim == INT_FAIL)
				goto opermiss_err;
			if (llim == INT_FAIL)
				llim = nchr;
			result = chr == '+' ? lval + rval : lval - rval;
			INIT_EXPR();
			if (DEBUG)
				puts(expr);
		}
	}

	while (strcspn(expr, OPERS_A + 2) != strlen(expr)) {
		puts("gate");
		result_str = dtos(evaluate(expr), DBL_DIG);
		free(expr);
		expr = result_str;
	}
	result = stod(expr);
	free(expr);
	return result;

	opermiss_err:
		free(expr);
		fail("Missing operand");
	evenroot_err:
		free(expr);
		fail("Even root of negative number");
	zeroroot_err:
		free(expr);
		fail("Root cannot be zero");
	divzero_err:
		free(expr);
		fail("Divide by zero");
	modulus_err:
		free(expr);
		fail("Remainder takes integers only");
	getval_err:
		free(expr);
		fail("Internal error (evaluate.getval)");
}

double getval(char *_expr, unsigned _operpos, char _dir) {	// Dynamic memory: val_str
	char chr, *val_str = NULL;
	double num;
	unsigned val_low, val_high;	// Lowest 
	int i;
	bool reading = false;

	if (_dir != 'l' && _dir != 'r') // Left and right directions only
		return DBL_FAIL;
	for (i = _dir == 'r' ? _operpos + 1 : _operpos - 1; (chr = _expr[i]) && i >= 0; _dir == 'r' ? i++ : i--) {	// Get size of value string
		if (isnumer(chr) && !reading) {
			reading = true;
			_dir == 'r' ? (val_low = i) : (val_high = i);
		} else if (!isnumer(chr) && reading) {
			_dir == 'r' ? (val_high = i) : (val_low = i);
			break;
		}
	}
	if (!reading) // No value found
		return 0;
	else if (i == -1) // Reached beginning of expression
		val_low = 0;
	else if (chr == 0)	// Reached end of expression
		val_high = strlen(_expr) - 1;

	if ((val_str = popsub(_expr, val_low, val_high)) == NULL) {
		free(_expr);
		return DBL_FAIL;
	}
	num = stod(val_str);
	free(val_str);
	return num; // Convert value string to value
}
double stod(const char *_s) { // Equivalent to atof(), except that it does not print inaccurate numbers
	int i;
	char chr;
	double num = 0, placeval = 0.1;
	bool read_decim = false, is_negative = false, reading = false;

	for (i = 0; (chr = _s[i]) && i <= DBL_DIG + is_negative + read_decim; i++) { // DBL_DIG is accurate digit limit
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
	if (i > DBL_DIG && !read_decim)
		fail("Overflow error");
	if (is_negative)
		num *= -1;
	return num;
}