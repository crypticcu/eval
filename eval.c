#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eval.h"

/* Leading underscore signifies parameter */
/* nchr ~ index position				  */

void fail(const char *_desc) {
	printf("%s\n", _desc);
	if (CMD_LINE) // Exit program if using command-line
		exit(EXIT_FAILURE);
}

void printu(const char *_s, size_t _hpos) {
	for (int nchr = 0; nchr < strlen(_s); nchr++) {
		if (nchr == _hpos)
			printf("\e[4m");	// Underline
		putchar(_s[nchr]);
		if (nchr == _hpos)
			printf("\e[24m");	// Reset underline
	}
}

char *dtos(double _x, size_t _sig) {	// Dynamic memory: numstr
	size_t nwhole = nplaces(_x);

	if (nwhole > DBL_DIG)
		return STR_OVER;
	if (nwhole + _sig > DBL_DIG)
		_sig = DBL_DIG - nwhole;
	if (_sig > DBL_DIG)	// Decimal place exceeds accurate number allotted by system
		_sig = DBL_DIG;

	bool is_negative = _x < 0, is_decimal = _sig, only_decimal = _x < 1 && _x > -1, only_whole = isequal(_x, (int) _x);
	size_t reqsize = nwhole + _sig + is_negative + is_decimal + only_decimal;
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
	for (int nchr = is_negative + only_decimal * 2, place = nplaces(_x) - !(nwhole == FLT_DIG && only_whole); nchr < reqsize; nchr++, place--) {	// Skip characters reserved for negative sign and decimal point, if present
		numstr[nchr] = getdigit(_x, place) + 48; // '0' = 48
		if (place == 0) {
			if (only_whole)
				break;
			else if (nchr + 1 != reqsize)
				numstr[++nchr] = '.';
		}
	}
	return numstr;
}

char *popsub(const char *_s, size_t _low, size_t _high) { // Dynamic memory: sub
	char *sub;	// Substring to be 'popped' from string

	if (_low >= strlen(_s) || _high >= strlen(_s) || _low > _high ||	// Low/high indices exceed range of string || Low index is greater than high || Allocation fails
	(sub = (char *) calloc(_high - _low + 2, sizeof(char))) == NULL)
		return NULL;
	for (size_t nchr_old = _low, nchr_new = 0; nchr_old <= _high; nchr_old++, nchr_new++)
		sub[nchr_new] = _s[nchr_old];
	return sub;
}

char *pushsub(char *_s, char *_sub, size_t _low, size_t _high) { // Dynamic memory: newstr
	int nchr_new;
	char *newstr = (char *) calloc(
		strlen(_s)				// Original length
		- (_high - _low + 1)	// Take away number of characters being removed
		+ strlen(_sub)			// Add size of substring
		+ 1						// Add space for null character
	, sizeof(char));
	
	if (_low >= strlen(_s) || _high >= strlen(_s) || _low > _high || newstr == NULL) // Low/high indices exceed range of string || Low index is greater than high || Allocation fails
		return NULL;								 
	for (nchr_new = 0; nchr_new < _low; nchr_new++)
		newstr[nchr_new] = _s[nchr_new];	// Add contents of old string up to point of integration
	for (int nchr_sub = 0; nchr_sub < strlen(_sub); nchr_sub++, nchr_new++)
		newstr[nchr_new] = _sub[nchr_sub];	// Integrate substring
	for (int nchr_old = _high + 1; nchr_old < strlen(_s); nchr_old++, nchr_new++)
		newstr[nchr_new] = _s[nchr_old];	// Add rest of old string
	free(_s);
	free(_sub);
	return newstr;
}

char *simplify(const char *_expr, size_t _from) {	// Dynamic memory: subA, subB, expr
	bool read_parenth = false;
	char chr, *subA = NULL, *subB = NULL,
		*expr = (char *) calloc(strlen(_expr) + 1, sizeof(char)); // Modifiable expression
	size_t par_low, par_high;
	int invpos;
	double result;

	if (expr == NULL)
		return NULL;
	if (!_from) { // Check syntax and parenthesese only once (_from is always 0 on first call)
		if ((invpos = chk_syntax(_expr)) != CHK_PASS)	// Check for syntax errors
			goto syntax_err;
		if ((invpos = chk_parenth(_expr)) != CHK_PASS)	// Check for parenthetical errors
			goto syntax_err;
	}
	strcpy(expr, _expr); // Copy constant expression to modifiable one
	for (int nchr = _from; (chr = expr[nchr]); nchr++) {	// Go straight to evaluate() if '-p' is passed
		if (chr == ')') {
			par_high = nchr;
			read_parenth = false;
			if ((subA = popsub(expr, par_low, par_high)) == NULL)
				goto popsub_err;
			expr[par_low] = (toast(expr, par_low)) ? '*' : ' ';
			expr[par_high] = (toast(expr, par_high)) ? '*' : ' ';
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
		printf("Syntax error: ");
		printu(_expr, invpos);
		free(expr);
		return NULL;
	evaluate_err:
		free(subA);
		return NULL;
	dtos_err:
		free(subA);
		fail("Internal error (simplify.dtos)"); // Exits program if using command-line
		return NULL;
	popsub_err:
		free(expr);
		fail("Internal error (simplify.dtos)");
		return NULL;
	pushsub_err:
		fail("Internal error (simplify.pushsub)");
		return NULL;
	simplify_err:
		free(subA);
		fail("Internal error (simplify.simplify)");
		return NULL;
}

bool isequal(double _x, double _y) {
	return fabs(_x - _y) < FLT_EPSILON;
}

bool isin(const char _x, const char *_y) {
	for (int nchr = 0; nchr < strlen(_y); nchr++)
		if (_x == _y[nchr])
			return true;
	return false;
}

bool isnumer(char _c) {
	return (isdigit(_c) || _c == '-' || _c == '.');
}

bool toast(const char *_expr, size_t _parpos) {	// To asterisk?
	char chr = _expr[_parpos],
		 next, // Character after parenthesis
		 last;	// Character before parenthesis
	
	next =  _parpos < strlen(_expr) - 1 ? _expr[_parpos + 1] : 0;
	last = _parpos ? _expr[_parpos - 1] : 0;
	return (isdigit(last) && isnumer(next)	||
		chr == '(' && last == ')'			||
		chr == ')' && next == '(') ? true : false;
}

size_t getdigit(double _x, int _place) {
	size_t digit;

	_x = fabs(_x);
	if (abs(_place) > DBL_DIG || _x > LLONG_MAX)	// Place cannot be over/under place limit; Any 'x' over max llong causes overflow on conversion
		return 0;	// Digits that cannot be printed
	for (int nchr = 0; nchr <= abs(_place); _place > 0 ? (_x /= 10) : (_x *= 10), nchr++)
		digit = ((long long int) _x - (long long int) (_x / 10) * 10);
	return digit;
}

size_t nplaces(double _x) {
	_x = fabs(_x);	// log of negative is undefined
	if (_x == 0)	// log of zero is undefined
		return 1;
	return log(_x)/log(10) + 1;
}

int chk_parenth(const char *_expr) {
	char chr;
	int nopen = 0, nclosed = 0, nchr;

	for (nchr = 0; (chr = _expr[nchr]); nchr++)	// Get number of closed parentheses
		if (chr == ')')
			nclosed++;
	for (nchr = 0; (chr = _expr[nchr]); nchr++) {
		if (chr == '(')
			nopen++;
		else if (chr == ')')
			nopen--;
		if (nopen > nclosed) { // Extra open parenthesis?
			while ((chr = _expr[--nchr]) != '('); // Find last instance of open parenthesis
			return nchr;
		}
		if (nopen < 0)	// Extra closed?
			return nchr;
	}
	return CHK_PASS;
}

int chk_syntax(const char *_expr) {
	char chr,
		 lead = 0,	// Last non-space 
		 trail = 0,	// Next non-space
		 last = 0,	// Immediate last
		 next = 1;	// Immediate next
	size_t nsingle = 0, ndouble = 0, npoint = 0, nerr;

	#ifdef DEBUG
	puts("\e[4mchk_syntax\e[24m");
	#endif
	for (size_t nchr = 0; (chr = _expr[nchr]); nchr++) {
		#ifdef DEBUG
		printf("single: %ld\tdouble: %ld\tchr: %c\n", nsingle, ndouble, chr);
		#endif
		if (nchr)
			last = _expr[nchr - 1];
		if (nchr != strlen(_expr))
			next = _expr[nchr + 1];
		if (next != 0)
			for (int i = nchr + 1; _expr[i]; i++)
				if(!isspace(_expr[i])) {
					trail = _expr[i];
					break;
				}
		if (isdigit(chr) || chr == '(') // Reset single/double operator count
			nsingle = 0, ndouble = 0;
		else if (isin(chr, DBLS) && (chr == last && isin(last, DBLS) || chr == next && isin(next, DBLS))) {
			if (chr != '!' && isdigit(lead) || chr == '!' && trail == '!' && !isdigit(lead) && lead != '.')	// Operator is obstruction
				return nchr;	
			if (chr == '!' && lead == '!' && !isdigit(trail) && trail != '.') {	// Operator isn't obstruction; find obstruction
				for (nerr = nchr; (chr = _expr[nerr]) != trail; nerr++);
				return nerr;
			}
			ndouble++;
		}
		else if (isin(chr, OPERS))
			nsingle++;
		if (!isdigit(chr) && chr != '.')
			npoint = 0;
		else if (chr == '.')
			npoint++;
		if (nsingle == 2 || ndouble == 3 ||	npoint == 2 ||	/* Extra operator or comma ||			   */
			!isin(chr, VAL_CHRS) && !isspace(chr) ||		/* Is not a valid character nor a space || */
			isdigit(chr) && isdigit(lead) && lead != last)	/* Two numbers side-by-side w/o operator   */
			return nchr;
		if (!isspace(chr))
			lead = chr;
	}
	return CHK_PASS;
}

int getlim(char *_expr, size_t _operpos, char _dir) {
	bool reading = false, read_digit;
	char chr;
	int lim = -1, nchr;

	if (_dir != 'l' && _dir != 'r') // Left and right directions only
		return INT_FAIL;
	for (nchr = _dir == 'r' ? _operpos + 1 : _operpos - 1; (chr = _expr[nchr]) && nchr >= 0; _dir == 'r' ? nchr++ : nchr--) {
		if (isnumer(chr) && !reading)
			reading = true;
		else if (!isnumer(chr) && reading) {
			lim = _dir == 'r' ? nchr - 1 : nchr + 1;
			break;
		}
		if (isdigit(chr))
			read_digit = true;
	}
	if (!reading || !read_digit) // No value found
		return INT_FAIL;
	else {
		if (nchr == -1) // Reached beginning of expression
			lim = 0;
		else if (chr == 0) // Reached end of expression
			lim = strlen(_expr) - 1;
	}
	return lim;
}

size_t fobst(const char *_expr, size_t _operpos, size_t _llim, size_t _rlim) {
	bool l_obstr = false, r_obstr = false;
	char chr, oper = _expr[_operpos];
	int nchr = _operpos,
		off = 0; // Offset from operator position

	if (_llim == INT_FAIL)
		_llim = 0;
	if (_rlim == INT_FAIL)
		_rlim = strlen(_expr) - 1;
	if (_llim >= strlen(_expr) || _rlim >= strlen(_expr) || _llim > _rlim)
		return INT_FAIL;
	while (off < (int) strlen(_expr)) {
		if (nchr >= _llim && nchr <= _rlim) {
			chr = _expr[nchr];
			if (isin(chr, OPERS) && chr != oper && chr != '-')
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
	size_t llim, rlim; // Left- and right-hand limits of operation
	int nchr;
	double result, // Operation result;	Final return value
		   lval, rval; // Left and right values of operation

	#ifdef DEBUG
	printf("\n\e[4mevaluate\e[24m\n%s\n", _expr);
	#endif
	if (expr == NULL)
		return DBL_FAIL;
	strcpy(expr, _expr); // Copy constant expression to modifiable one

	loop:
	for (nchr = 0; (chr = expr[nchr]); nchr++) {	// Increment/decrement
		if (chr == '+' && expr[nchr + 1] == '+' || chr == '-' && expr[nchr + 1] == '-') {
			INIT_VALS();	// Retrieves rval, lval, rlim, and llim
			if (fobst(expr, nchr, llim, rlim) & OBS_R)	// Operation cannot continue if another operator is in the way
				continue;
			if (isequal(rval, DBL_OVER) || isequal(lval, DBL_OVER))
				goto overflow_err;
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
			if (rlim == INT_FAIL)
				goto opermiss_err;
			result = chr == '+' ? rval + 1 : -rval - 1;					/* If increment: '++x' -> '+(x + 1)'  */
			llim = nchr;	// Left limit of operation is operator		/* If decrement: '--x' -> '-(-x + 1)' */
			INIT_EXPR();	// Retrieves new expression
			#ifdef DEBUG
				puts(expr);
			#endif
		}
	}
	for (nchr = 0; (chr = expr[nchr]); nchr++) { // Unary root/Binary root
		if (chr == '!') {
			INIT_VALS();
			if (expr[nchr + 1] == '!') {
				if (fobst(expr, nchr, llim, rlim) & (OBS_L & OBS_R)) // Needs both sides of operator
					continue;
			if (isequal(rval, DBL_OVER) || isequal(lval, DBL_OVER))
				goto overflow_err;
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
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
			#ifdef DEBUG
			puts(expr);
			#endif
		}
	}
	for (nchr = 0; (chr = expr[nchr]); nchr++) { // Exponent
		if (chr == '^') {
			INIT_VALS();
			if (fobst(expr, nchr, llim, rlim) & (OBS_L & OBS_R))
				continue;
			if (isequal(rval, DBL_OVER) || isequal(lval, DBL_OVER))
				goto overflow_err;
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
			if (rlim == INT_FAIL || llim == INT_FAIL)
				goto opermiss_err;
			result = pow(lval, rval);
			INIT_EXPR();
			#ifdef DEBUG
			puts(expr);
			#endif
		}
	}
	for (nchr = 0; (chr = expr[nchr]); nchr++) { // Multiplication/Division/Remainder
		if (chr == '*' || chr == '/' || chr == '%') {
			INIT_VALS();
			if (fobst(expr, nchr, llim, rlim) & (OBS_L & OBS_R))
				continue;
			if (isequal(rval, DBL_OVER) || isequal(lval, DBL_OVER))
				goto overflow_err;
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
			#ifdef DEBUG
			puts(expr);
			#endif
		}
	}
	for (nchr = 0; (chr = expr[nchr]); nchr++) { // Addition/Subtraction/Unary plus/Unary minus
		if (chr == '+' || chr == '-') {
			if (chr == '+' && expr[nchr + 1] == '+' || chr == '-' && expr[nchr + 1] == '-')	// Increment/Decrement found
				goto loop;
			INIT_VALS();
			if (fobst(expr, nchr, llim, rlim) & OBS_R)
				continue;
			if (isequal(rval, DBL_OVER) || isequal(lval, DBL_OVER))
				goto overflow_err;
			if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))
				goto getval_err;
			if (rlim == INT_FAIL)
				goto opermiss_err;
			if (llim == INT_FAIL)
				llim = nchr;
			result = chr == '+' ? lval + rval : lval - rval;
			INIT_EXPR();
			#ifdef DEBUG
			puts(expr);
			#endif
		}
	}
	while (strcspn(expr, OPERS + 2) != strlen(expr)) {
		if ((result_str = dtos(evaluate(expr), DBL_DIG)) == NULL)
			goto dtos_err;
		if ((result_str = dtos(evaluate(expr), DBL_DIG)) == STR_OVER)
			goto overflow_err;
		free(expr);
		expr = result_str;
	}
	if (isequal(result = stod(expr), DBL_OVER))
		goto overflow_err;
	free(expr);
	return result;

	opermiss_err:
		free(expr);
		fail("Missing operand"); // Exits program if using command-line
		return DBL_FAIL;
	evenroot_err:
		free(expr);
		fail("Even root of negative number");
		return DBL_FAIL;
	zeroroot_err:
		free(expr);
		fail("Root cannot be zero");
		return DBL_FAIL;
	divzero_err:
		free(expr);
		fail("Divide by zero");
		return DBL_FAIL;
	modulus_err:
		free(expr);
		fail("Remainder takes integers only");
		return DBL_FAIL;
	getval_err:
		free(expr);
		fail("Internal error (evaluate.getval)");
		return DBL_FAIL;
	dtos_err:
		free(expr);
		fail("Internal error (evaluate.dtos)");
		return DBL_FAIL;
	overflow_err:
		free(expr);
		fail("Number too large");
		return DBL_FAIL;
}

double getval(char *_expr, size_t _operpos, char _dir) {	// Dynamic memory: val_str
	bool reading = false;
	char chr, *val_str = NULL;
	size_t val_low, val_high;	// Range of indices in which value is located
	int nchr;
	double num;

	if (_dir != 'l' && _dir != 'r') // Left and right directions only
		return DBL_FAIL;
	for (nchr = _dir == 'r' ? _operpos + 1 : _operpos - 1; (chr = _expr[nchr]) && nchr >= 0; _dir == 'r' ? nchr++ : nchr--) {	// Get size of value string
		if (isnumer(chr) && !reading) {
			reading = true;
			_dir == 'r' ? (val_low = nchr) : (val_high = nchr);
		} else if (!isnumer(chr) && reading) {
			_dir == 'r' ? (val_high = nchr) : (val_low = nchr);
			break;
		}
	}
	if (!reading) // No value found
		return 0;	// Required for evaluate() functionality
	else if (nchr == -1) // Reached beginning of expression
		val_low = 0;
	else if (chr == 0)	// Reached end of expression
		val_high = strlen(_expr) - 1;
	if ((val_str = popsub(_expr, val_low, val_high)) == NULL) {
		free(_expr);
		return DBL_FAIL;
	}
	if ((num = stod(val_str)) == DBL_FAIL) {
		free(val_str);
		return DBL_OVER;
	}
	free(val_str);
	return num; // Convert value string to value
}

double stod(const char *_s) { // Equivalent to atof(), except that it does not print inaccurate numbers
	bool read_decim = false, is_negative = false, reading = false;
	char chr;
	int nchr;
	double num = 0, placeval = 0.1;

	for (nchr = 0; (chr = _s[nchr]) && nchr <= DBL_DIG + is_negative + read_decim; nchr++) { // DBL_DIG is accurate digit limit
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
