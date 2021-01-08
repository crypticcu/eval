#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <err.h>
#include <unistd.h>
#include "parse.h"

char *dtos(double _x, unsigned int _sig) {
	//// Dynamic memory: numstr ////

	bool isNegative = _x < 0, isDecimal = _sig != 0, onlyDecimal = _x < 1 && _x > -1;
	unsigned int nwhole = nplaces(_x), reqsize = nwhole + _sig + isNegative + isDecimal + onlyDecimal;
	char *numstr = (char *) calloc(reqsize + 1, sizeof(char));

	if (_sig > DBL_DIG || numstr == NULL) // Decimal place exceeds accurate number or allocation fails
		return NULL;

	/* Negative and decimal prerequisites */
	if (isNegative) {
		numstr[0] = '-';
		if (onlyDecimal) {
			numstr[1] = '0';
			numstr[2] = '.';
		}	
	} else if (onlyDecimal) {
		numstr[0] = '0';
		numstr[1] = '.';
	}
	/* Parse double */
	for (int i = isNegative + 2 * onlyDecimal, place = nplaces(_x) - 1; i < reqsize; i++, place--) {
		numstr[i] = getdigit(_x, place) + 48; // '0' = 48
		if (place == 0 && i + 1 != reqsize) {
			numstr[i + 1] = '.';
			i++;
		}
	}
	return numstr;
}

char *popsub(const char *_s, unsigned int _low, unsigned int _high) {
	//// Dynamic memory: sub ////

	char *sub; /* Number of characters being taken away
																   * Size of new substring plus null character */

	if (_low >= strlen(_s) || _high >= strlen(_s) || _low > _high || (sub = (char *) calloc(_high - _low + 2, sizeof(char))) == NULL) /* Low or high values outside of string
																    * range of values would cause overflow, allocation fails*/
		return NULL;
	for (int nchr_old = _low, nchr_new = 0; nchr_old <= _high; nchr_old++, nchr_new++)
		sub[nchr_new] = _s[nchr_old];
	return sub;
}

char *pushsub(char *_s, char *_sub, unsigned int _low, unsigned int _high) {
	//// Dynamic memory: newstr ////

	char *newstr = (char *) calloc(
		strlen(_s)				// Original length
		- (_high - _low + 1)	// Take away number of characters being taken away
		+ strlen(_sub)			// Add size of substring being added
		+ 1						// Add extra space for null character
	, sizeof(char));
	int nchr_new;

	if (_low >= strlen(_s) || _high >= strlen(_s) || _low > _high || newstr == NULL) /* Low or high values outside of string
																    * range of values would cause overflow or allocation fails */
		return NULL;
	
	/* Add contents of old string up to point of integration */
	for (nchr_new = 0; nchr_new < _low; nchr_new++)
		newstr[nchr_new] = _s[nchr_new];
	
	/* Add substring */
	for (int nchr_sub = 0; nchr_sub < strlen(_sub); nchr_sub++, nchr_new++)
		newstr[nchr_new] = _sub[nchr_sub];
	
	/* Add the rest of old string */
	for (int nchr_old = _high + 1; nchr_old < strlen(_s); nchr_old++, nchr_new++)
		newstr[nchr_new] = _s[nchr_old];

	free(_s);
	free(_sub);
	return newstr;
}

char *simplify(const char *_expr, unsigned int _from) {
	//// Dynamic memory: subA, subB, expr ////

	char chr, *subA = NULL, *subB = NULL,
	*expr = (char *) calloc(strlen(_expr) + 1, sizeof(char)); // Dynamic expression string
	bool rpar = false; // Reading parentheses?
	unsigned int par_low, par_high, inv;

	if (expr == NULL)
		return NULL;
	if (!_from) { //Only once needed
		if (chk_syntax(_expr) != CHK_PASS && !F_SYN) { // Check for syntax errors
			inv = chk_syntax(_expr);
			goto syntax_err;
		}
		if (chk_parenth(_expr) != CHK_PASS && !F_PAR) { // Check for parenthetical errores
			inv = chk_parenth(_expr);
			goto syntax_err;
		}
	}
	strcpy(expr, _expr); // Copy static expression string to dynamic one
	for (int nchr = _from; (chr = expr[nchr]) != 0 && !F_PAR; nchr++) {
		if (chr == ')') {
			par_high = nchr;
			rpar = false;
			if ((subA = popsub(expr, par_low, par_high)) == NULL) {
				free(expr);
				return NULL;
			}
			if (isequal(evaluate(subA), DBL_FAIL)) 
				goto evaluate_err;
			if ((subB = dtos(evaluate(subA), DBL_DIG)) == NULL)
				goto dtos_err;
			free(subA);
			if ((expr = pushsub(expr, subB, par_low, par_high)) == NULL)
				goto pushsub_err;
			if (_from)
				return expr;
		}
		else if (chr == '(') {
			if (rpar) {
				subA = expr; // Swap may cause 'still reachable' error in valgrind
				//It's not a bug... it's a feature!
				if ((expr = simplify(expr, nchr)) == NULL)
					goto simplify_err;
				free(subA);
			} else {
				rpar = true;
				par_low = nchr;
			}
		}
	}
	subA = expr; // Swap may cause 'still reachable' error in valgrind
	//It's not a bug... it's a feature!
	if (isequal(evaluate(expr), DBL_FAIL)) 
		goto evaluate_err;
	if ((expr = dtos(evaluate(expr), DBL_DIG)) == NULL)
		goto dtos_err;
	free(subA);
	return expr;

	syntax_err:
		printf("%s: Syntax error: ", PROG_NAME);
		for (int i = 0; i < strlen(_expr); i++) {
			if (i == inv)
				printf("\033[31m");
			else if (i == inv + 1)
				printf("\033[0m");
			putchar(_expr[i]);
		}
		printf("\033[0m"); // If invchr is at very end of string
		putchar('\n');
		free(expr);
		exit(EXIT_FAILURE);
	evaluate_err:
		free(subA);
		free(expr);
		errx(EXIT_FAILURE, "Internal error (simplify.evaluate)");
	dtos_err:
		free(subA);
		errx(EXIT_FAILURE, "Internal error (simplify.dtos)");
	pushsub_err:
		errx(EXIT_FAILURE, "Internal error (simplify.pushsub)");
	simplify_err:
		free(subA);
		errx(EXIT_FAILURE, "Internal error (simplify.simplify)");
}

bool isequal(double _x, double _y) {
	return fabs(_x - _y) < FLT_EPSILON;
}

bool isin(const char _x, const char *_y) {
	for (int i = 0; i < strlen(_y); i++) {
		if (_x == _y[i])
			return true;
	}
	return false;
}

unsigned int nplaces(double _x) {
	_x = fabs(_x);
	if (_x == 0)
		return 1;
	return log(_x)/log(10) + 1;
}

int chk_syntax(const char *_expr) {
	char chr, currlead = '!';
	const char *valchrs = "1234567890()^!*/%%+-\n",
	*leads = "(^!*/%%+-",
	*s_ops = "^*/%%+-",
	*d_ops = "!+-";
	int s_op = 0, d_op = 0;

	for (int i = 0; (chr = _expr[i]) != 0; i++) {
		if (isdigit(chr)) // Reset single, double operator series
			s_op = 0, d_op = 0;
		else if (isin(chr, s_ops)) // Another singleop read
			s_op++;
		else if (isin(chr, d_ops)) // Another doubleop read
			d_op++;
		if (s_op == 2 || d_op == 3 || !isin(chr, valchrs) && !isspace(chr) || chr == '!' && !isin(currlead, leads) && !isin(_expr[i + 1], leads)) // Extra single op || Extra double op || invalid character || invalid lead
			return i;
		if (isin(chr, valchrs))
			currlead = chr;
	}
	return CHK_PASS;
}

int chk_parenth(const char *_expr) {
	char chr;
	int open = 0, ncl = 0; // Parentheses currently open

	for (int i = 0; (chr = _expr[i]) != 0; i++) { // Get number of closed parentheses
		if (chr == ')')
			ncl++;
	}
	for (int i = 0; (chr = _expr[i]) != 0; i++) {
		if (chr == '(')
			open++;
		else if (chr == ')')
			open--;
		if (open > ncl) { // Extra open
			while ((chr = _expr[--i]) != '('); // Find last instance of extra open
			return i;
		}
		if (open < 0)	// Extra closed
			return i;
	}
	return CHK_PASS;
}

int getdigit(double _x, int _place) {
	int digit;

	_x = fabs(_x);
	if (abs(_place) > DBL_DIG || _x > LLONG_MAX)
		return 0;
	for (int i = 0; i <= abs(_place); _place > 0 ? (_x /= 10) : (_x *= 10), i++) {
		digit = ((long long int) _x - (long long int) (_x / 10) * 10);
	}
	return digit;
}

int getlim(char *_expr, unsigned int _oper_pos, char _dir) {
	int lim = -1, i;
	char chr;
	bool reading = false;

	if (_dir != 'l' && _dir != 'r') // Invalid direction
		return INT_FAIL;
	for (i = _dir == 'r' ? _oper_pos + 1 : _oper_pos - 1; (chr = _expr[i]) != 0 && i >= 0; _dir == 'r' ? i++ : i--) {
		if ((isdigit(chr) || chr == '.' || chr == '-') && !reading)
			reading = true;
		else if (!isdigit(chr) && chr != '.' && chr != '-' && reading) {
			lim = _dir == 'r' ? i - 1 : i + 1;
			break;
		}	
	}
	if (!reading) // No value found
		return INT_FAIL;
	else {
		if (i == -1) // Reached the very beginning of the expression string
			lim = 0;
		else if (chr == 0) // Reached the very end of the expression string
			lim = strlen(_expr) - 1;
	}
	return lim;
}

double evaluate(const char *_expr) {
	//// Dynamic memory: result_str, expr ////

	char chr,
	*result_str = NULL, // Operation result string
	*expr = (char *) calloc(strlen(_expr) + 1, sizeof(char)); // Dynamic expression string
	double result, // Operation result, final return value
	lval, rval; // Left and right values of operation
	unsigned int llim, rlim; // Left- and right-hand limits of operation in string

	if (expr == NULL)
		return DBL_FAIL;
	strcpy(expr, _expr); // Copy static expression string to dynamic one

	/* Prefix increment/decrement */

	/* Unary root/Binary root */
	for (int nchr = 0; (chr = expr[nchr]) != 0; nchr++) {
		if (chr == '!') {
			INIT_VALS();
			if (expr[nchr + 1] == '!') {;
				if (rlim == INT_FAIL || llim == INT_FAIL)
					goto opermiss_err;
				if (rval < 0 && (int) lval % 2 == 0 && !F_NAN)
					goto evenroot_err;
				if (lval == 0 && !F_NAN)
					goto zeroroot_err;
				result = rval < 0 ? -pow(-rval, 1 / lval) : pow(rval, 1 / lval);
			} else {
				if (rlim == INT_FAIL)
					goto opermiss_err;
				if (rval < 0)
					goto evenroot_err;
				llim = nchr;
				result = sqrt(rval);
			}
			INIT_EXPR();
			PRINT_DEBUG();
		}
	}

	/* Exponent */
	for (int nchr = 0; (chr = expr[nchr]) != 0; nchr++) {
		if (chr == '^') {
			INIT_VALS();
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
			if (rlim == INT_FAIL || llim == INT_FAIL)
				goto opermiss_err;
			result = pow(lval, rval);
			INIT_EXPR();
			PRINT_DEBUG();
		}
	}

	/* Multiplication/Division/Remainder */
	for (int nchr = 0; (chr = expr[nchr]) != 0; nchr++) {
		if (chr == '*' || chr == '/') {
			INIT_VALS();
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
			if (rlim == INT_FAIL || llim == INT_FAIL)
				goto opermiss_err;
			if (rval == 0 && chr != '*' && !F_NAN)
				goto divzero_err;
			result = chr == '*' ? lval * rval : lval / rval;
			INIT_EXPR();
			PRINT_DEBUG();
		}
	}

	/* Addition/Unary plus/Subtraction/Unary minus */
	for (int nchr = 0; (chr = expr[nchr]) != 0; nchr++) {
		if (chr == '+' || chr == '-') {
			INIT_VALS();
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
			if (rlim == INT_FAIL)
				goto opermiss_err;
			if (llim == INT_FAIL)
				llim = nchr;
			result = chr == '+' ? lval + rval : lval - rval;
			INIT_EXPR();
			PRINT_DEBUG();
		}
	}
	result = stod(expr);
	free(expr);
	return result;

	opermiss_err:
		free(expr);
		errx(EXIT_FAILURE, "Missing operand");
	evenroot_err:
		free(expr);
		errx(EXIT_FAILURE, "Even root of negative number");
	zeroroot_err:
		free(expr);
		errx(EXIT_FAILURE, "Root cannot be zero");
	divzero_err:
		free(expr);
		errx(EXIT_FAILURE, "Divide by zero");
	getval_err:
		free(expr);
		errx(EXIT_FAILURE, "Internal error (evaluate.getval)");
}

double getval(char *_expr, unsigned int _oper_pos, char _dir) {
	//// Dynamic memory: val_str ////

	char chr, *val_str = NULL;
	double num;
	unsigned int val_low, val_high;
	int i;
	bool reading = false;

	if (_dir != 'l' && _dir != 'r')
		return 0;

	/* Get size of value string */
	for (i = _dir == 'r' ? _oper_pos + 1 : _oper_pos - 1; (chr = _expr[i]) != 0 && i >= 0; _dir == 'r' ? i++ : i--) {
		if ((isdigit(chr) || chr == '-' || chr == '.') && !reading) {
			reading = true;
			_dir == 'r' ? (val_low = i) : (val_high = i);
		} else if ((!isdigit(chr) && chr != '.' && chr != '-') && reading) {
			_dir == 'r' ? (val_high = i) : (val_low = i);
			break;
		}
	}
	if (!reading) // No value found
		return 0;
	else if (i == -1) // Reached the very beginning of the expression string
		val_low = 0;
	else if (chr == 0)
		val_high = strlen(_expr) - 1;

	if ((val_str = popsub(_expr, val_low, val_high)) == NULL) {
		free(_expr);
		return DBL_FAIL;
	}
	num = stod(val_str);
	free(val_str);
	return num; // Convert value string to value
}

double stod(const char *_s) { // Equivalent to atof()
	int i;
	char chr;
	double num = 0, dec_place = 0.1;
	bool dec = false, neg = false, reading = false; // Reading decimal?

	for (i = 0; (chr = _s[i]) != 0 && i <= DBL_DIG + neg + dec; i++) {
		if (chr == '-' && !neg && !reading)
			neg = true;
		else if (chr == '.' && !dec && !reading)
			dec = true;
		if (!dec && isdigit(chr)) {
			num *= 10;
			num += chr - 48;
		} else if (isdigit(chr)) {
			reading = true;
			num += dec_place * (chr - 48);
			dec_place /= 10;
		}
	}
	if (i > DBL_DIG && !dec)
		goto overflow_err;
	if (neg)
		num *= -1;
	return num;

	overflow_err:
		errx(EXIT_FAILURE, "Number too large");
}