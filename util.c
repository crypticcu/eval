#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "parse.h"
#include "status.h"
#include "util.h"

void fprint(const char *str, size_t begin, size_t end, format_t fmt) {
	size_t swap;

	if (!str) {
		printf("(null)");
		return;
	}
	if (begin > end) {	// Swap values
		swap = end;
		end = begin;
		begin = swap;
	}
	for  (size_t index = 0; index < strlen(str); index++) {
		if (index == begin)
			printf("%s", fmt);
		putchar(str[index]);
		if (index == end)
			printf("%s", F_CLR);
	}
}

char *dtos(double x, unsigned sig) {
	bool only_decimal, is_whole;
	char *string, digit;
	int exponent, abs_exp;
	size_t nwplaces, ndplaces, reqsize = 0, index = 0;
	double mantissa;

	if (isnan(x)) {
		setstat(ERR_IMAGINARY);
		return NULL;
	}
	mantissa = getmant(x);
	exponent = getexp(x);
	abs_exp = abs(exponent);
	if (abs_exp > MantSize || isinf(x)) {
		setstat(ERR_OVERFLOW);
		return NULL;
	}
	if (abs_exp > MantSize - 1) {
		x = mantissa;
		reqsize +=
			1					// 'E'
			+ exponent < 0		// Negative sign, if present
			+ nwhole(exponent);	// Exponent
	}
	only_decimal = x < 1 && x > -1 && x;
	is_whole = iswhole(x);
	nwplaces = nwhole(x);
	ndplaces = ndecim(x);
	if (sig > ndplaces)
		sig = ndplaces;
	if (nwplaces + sig > MantSize)
		sig = MantSize - nwplaces;
	if (sig > MantSize)
		sig = MantSize;
	reqsize +=
		nwplaces + sig	// Digits
		+ !is_whole		// Decimal point
		+ only_decimal	// Leading zero, if present
		+ 1;			// Negative/Positive sign
	string = (char *) calloc(reqsize + 1 /* Null character */, sizeof(char));
	if (!string) {
		setstat(ERR_INTERNAL);
		return NULL;
	}
	string[index++] = x < 0 ? '-' : '+';	// Positive sign required for proper parse() functionality
	if (only_decimal) {
		string[index++] = '0';
		string[index++] = '.';
	}
	for (int place = nwplaces - !(nwplaces == MantSize && is_whole); index < reqsize; index++, place--) {	// Print number or mantissa
		string[index] = getdigit(x, place);
		if (place == 0) {
			if (is_whole)
				break;
			string[++index] = '.';
		}
	}
	if (abs_exp > MantSize - 1) {	// Print sci. notation, if present
		strcat(string, "E");
		index++;
		if (exponent < 0)
			string[index++] = '-';
		if (digit = getdigit(exponent, 1))
			string[index++] = digit;
		string[index++] = getdigit(exponent, 0);
	}
	return string;
}

char *getln(size_t limit) {
	char *input = malloc(sizeof(char)), chr;
	size_t memsize = 1, index = 0;

	if (!input) {
		setstat(ERR_INTERNAL);
		return NULL;
	}
	do {
		chr = getchar();
		if (index == memsize) {
			if (!(input = (char *) realloc(input, (memsize *= 2) * sizeof(char)))) {
				setstat(ERR_INTERNAL);
				return NULL;
			}
		}
		if (limit != 0)
			input[index++] = chr;
		if (chr == '\n')
			break;
	} while (index < limit - 1 && index < SSIZE_MAX /* Prevents overflow */);
	if (index == memsize) {
		if (!(input = (char *) realloc(input, ++memsize * sizeof(char)))) {
			setstat(ERR_INTERNAL);
			return NULL;
		}
	}
	input[index] = '\0';
	if (chr != '\n')
		while ((chr = getchar()) != '\n');	// Flush stdin if needed
	return input;
}

char *popsub(const char *str, size_t low, size_t high) {
	char *sub;

	if (low >= strlen(str) || high >= strlen(str) || low > high) {
		setstat(ERR_INTERNAL);
		return NULL;
	}
	sub = (char *) calloc(
		high - low + 1	// # of characters being removed
		+ 1,			// Null character
		sizeof(char));
	if (!sub) {
		setstat(ERR_INTERNAL);
		return NULL;
	}
	for (size_t index_old = low, index_new = 0; index_old <= high; index_old++, index_new++)
		sub[index_new] = str[index_old];
	return sub;
}

char *pprint(const char *string) {
	char *str, *swap;
	size_t ignore;

	if (!string)
		return NULL;
	if (!(str = strdup(string))) {
		setstat(ERR_INTERNAL);
		return NULL;
	}
	ignore = strspn(str, " ");
	if (str[ignore] == '+')
		ignore++;
	if (ignore > 0) {
		swap = str;
		str = strdup(str + ignore);
		free(swap);
		if (!str) {
			setstat(ERR_INTERNAL);
			return NULL;
		}
	}
	return str;
}

char *pushsub(char *str, char *sub, size_t low, size_t high) {
	char *result;
	size_t index_old, index_new;

	if (low >= strlen(str) || high >= strlen(str) || low > high) {
		free(str);
		free(sub);
		setstat(ERR_INTERNAL);
		return NULL;
	}
	result = (char *) calloc(
		strlen(str)			// Original length
		- (high - low + 1)	// # of characters being removed
		+ strlen(sub)		// Substring length
		+ 1,				// Null character
		sizeof(char));
	if (!result) {
		free(str);
		free(sub);
		setstat(ERR_INTERNAL);
		return NULL;
	}
	for (index_new = 0; index_new < low; index_new++)
		result[index_new] = str[index_new];	// Add contents of old string up to point of integration
	for (index_old = 0; index_old < strlen(sub); index_old++, index_new++)
		result[index_new] = sub[index_old];	// Integrate substring
	for (index_old = high + 1; index_old < strlen(str); index_old++, index_new++)
		result[index_new] = str[index_old];	// Add rest of old string
	free(str);
	free(sub);
	return result;
}

char *roundnum(char *str, unsigned sig) {
	char *curr, *next;
	size_t digitpos;

	if (!str) {
		setstat(ERR_INTERNAL);
		return NULL;
	}
	if (!strchr(str, '.'))	// Whole numer, rounding not necessary
		return str;
	digitpos = strcspn(str, ".") + sig - (sig == 0);
	curr = &str[digitpos];
	next = curr + 1;
	while (*next == '.')	next++;
	return roundfrom(str, digitpos, *next >= '5' ? UP : DOWN);
}

char *roundfrom(char *str, size_t digitpos, direct_t dir) {
	char *digit, *swap, chr;
	ssize_t index = strlen(str) - 1;

	if (dir == UP) {
		for (index = digitpos; index >= 0; index--) {
			digit = &str[index];
			if (!isdigit(*digit))
				continue;
			(*digit)++;
			if (*digit == '9' + 1)
				*digit = '0';
			else
				break;
		}
	}
	swap = str;
	str = (char *) calloc(
		digitpos + 1					// Characters being kept
		+ (dir == UP && index == -1)	// Carryover, if present
		+ 1, 							// Null character
		sizeof(char));
	if (!str) {
		setstat(ERR_INTERNAL);
		free(swap);
		return NULL;
	}
	index = 0;
	if (index == -1)
		str[index++] = '1';	// Carryover
	strncat(str, swap, digitpos + 1);
	free(swap);
	return str;
}

bool toast(const char *expr, size_t parpos) {
	char lead;	// Last non-space
	char trail;	// Next non-space

	if (parpos && parpos < strlen(expr) - 1) {	// Parenthesis is not first nor last
		for (size_t off = 1; isspace(lead = expr[parpos - off]) && parpos - off >= 1; off--);
		for (size_t off = 1; isspace(trail = expr[parpos + off]) && trail; off++);
		return (expr[parpos] == '(' && isnum(lead) && isnumer(trail) ||
				expr[parpos] == ')' && isnum(lead) && isnum(trail)   ||
				expr[parpos] == ')' && trail == '(');
	} else
		return false;
}

char getdigit(double x, int place) {
	if (isnan(x) || isinf(x))
		return '0';
	x = fabs(x);
	if (abs(place) > MaxDec)	// Cannot exceed mantissa, takes care of x's over SSIZE_MAX as well
		return '0';				// Digits that cannot be printed will come up as '0'
	return ((ssize_t) (x *= pow(10, -place)) - (ssize_t) (x / 10) * 10) + 48;	// '0' = 48
}

unsigned getexp(double x) {
	float minsci = pow(10, MantSize);	// Minimimum to be shown in scientific notation

	if (x < minsci && !isequal(x, minsci) || isnan(x) || isinf(x))
		return 0;
	return nwhole(x) - 1;
}

unsigned ndecim(double x) {
	unsigned num;

	if (!x)	// 0 is printed as having 0 decimal places
		return 0;
	if (isnan(x) || isinf(x))
		return UINT_MAX;
	for (num = 0; x - (ssize_t) x; num++)	x *= 10;
	return num;
}

unsigned nwhole(double x) {
	unsigned num;

	if (!x)	//	0 is printed as having 1 whole place
		return 1;
	if (isnan(x) || isinf(x))
		return UINT_MAX;
	for (num = 0; (ssize_t) x; num++)	x /= 10;
	return num;
}

ssize_t chk_parenth(const char *expr) {
	char chr;
	size_t index;
	size_t nopen = 0;	// # of open parentheses
	size_t nclosed = 0;	// # of closed parentheses

	for (index = 0; (chr = expr[index]); index++)	// Get number of closed parentheses
		if (chr == ')')
			nclosed++;
	for (index = 0; (chr = expr[index]); index++) {
		if (chr == '(')
			nopen++;
		else if (chr == ')')
			nopen--;
		if (nopen > nclosed ||	// Extra open parenthesis?
			nopen < 0) {		// Extra closed?
			setstat(ERR_SYNTAX);
			setinv(expr, index);
			return index;
		}
	}
	return PASS;
}

ssize_t chk_syntax(const char *expr) {
	char chr;
	char next;			// Immediate next
	char last = '\0';	// Immediate last
	char lead = '\0';	// Last non-space
	char trail = '\0';	// Next non-space
	char singl = '\0';	// Last single operator
	char doubl = '\0';	// Last double oeprator
	size_t nsingle = 0;	// Single operators
	size_t ndouble = 0;	// Double operators
	size_t npoint = 0;	// Decimal points
	size_t pcount = 0;	// # parsed in current expression

	for (size_t index = 0; (chr = expr[index]); index++) {
		for (size_t trailing = index + 1; trailing < strlen(expr); trailing++) {
			trail = expr[trailing];
			if (!isspace(trail))
				break;
		}
		if (index)
			last = expr[index - 1];
		next = expr[index + 1];
		if (isdigit(chr) || chr == '(' || chr == ')') {
			nsingle = 0, ndouble = 0;
			if (chr == '(')
				pcount = -1;
		}
		else if (strchr(ChrSets.doubl, chr) && (chr == last || chr == next )) {
			if (chr != doubl && ndouble ||							/* Double operator mismatch	  */
			    isparity(chr) && (isdigit(lead) || lead == ')')) {	/* Increment/Decrement misuse */
				setstat(ERR_SYNTAX);
				setinv(expr, index);
				return index;
			}
			doubl = chr;
			ndouble++;
		}
		else if (strchr(ChrSets.opers, chr)) {
			if (chr != singl && isparity(chr)) {	/* Allow for situations such as */
				nsingle--;							/* '2/-2' without parentheses	*/
				if (isparity(trail) && !isnum(lead)) {	// Catch unary operator misuse
					setstat(ERR_SYNTAX);
					setinv(expr, index);
					return index;
				}

			}
			singl = chr;
			nsingle++;
		}
		if (!isdigit(chr) && chr != '.')
			npoint = 0;
		else if (chr == '.')
			npoint++;
		if (nsingle == 2 || ndouble == 3 ||	npoint == 2						||		/* Extra operator or comma */
			!strchr(ChrSets.valid, chr) && !isspace(chr)					||		/* Invalid character	   */
			(isnum(chr) || chr == '(') && isnum(lead) && lead != last		||		/* Two #'s side-by-side    */
			chr == ')' && isnum(trail) && trail != next						||		/*                         */
			chr == 'E' && (!isnumer(last) || !isnumer(next))) {					/* Invalid sci. notation   */
			setstat(ERR_SYNTAX);
			setinv(expr, index);
			return index;
		}
		if (!isspace(chr))
			lead = chr;
		pcount++;
	}
	return PASS;
}

ssize_t getlim(char *expr, size_t operpos, direct_t dir) {
	char chr;
	bool reading = false;
	ssize_t index, lim = -1;

	for (index = dir == RIGHT ? operpos + 1 : operpos - 1; index >= 0 && (chr = expr[index]); dir == RIGHT ? index++ : index--) {
		if (dir == RIGHT) {
			if (reading && !isnumer(chr)) {
				lim = (index) ? index - 1 : index;
				break;
			} else if (!reading && isnumer(chr))
				reading = true;
		} else {
			if (reading && !isnumer(chr)) {
				lim = index + 1;
				break;
			} else if (!reading && isnumer(chr))
				reading = true;
		}
	}
	if (!reading)		// No value found
		return operpos;
	if (index == -1)	// Reached beginning of expression
		lim = 0;
	else if (chr == 0)	// Reached end of expression
		lim = strlen(expr) - 1;
	return lim;
}

double getmant(double x) {
	double step;
	double temp = x;

	if (!x || isnan(x) || isinf(x))
		return x;
	step = x < 0 ? 1.0/10 : 10;
	while (nwhole(x) + 9 > MantSize)	x /= step;
	return x;
}

double getval(char *expr, size_t operpos, direct_t dir) {
	char *valstr, chr;
	bool reading = false;
	size_t val_low, val_high;
	ssize_t index;
	double val;

	for (index = dir == RIGHT ? operpos + 1 : operpos - 1; index >= 0 && (chr = expr[index]); dir == RIGHT ? index++ : index--) {	// Get size of value string
		if (dir == RIGHT) {
			if (reading && !isnumer(chr)) {
				val_high = (index) ? index - 1 : index;
				break;
			} else if (!reading && isnumer(chr)) {
				reading = true;
				val_low = index;
			}
		} else {
			if (reading && !isnumer(chr)) {
				val_low = index + 1;
				break;
			} else if (!reading && isnumer(chr)) {
				reading = true;
				val_high = index;
			}
		}
	}
	if (!reading)			// No value found
		return 0;
	else if (index == -1)	// Reached beginning of expression
		val_low = 0;
	else if (chr == 0)		// Reached end of expression
		val_high = strlen(expr) - 1;
	if (!(valstr = popsub(expr, val_low, val_high))) {
		free(expr);
		return FAIL;
	}
	if ((val = stod(valstr)) == FAIL) {
		free(valstr);
		return FAIL;
	}
	free(valstr);
	return val;
}

double stod(const char *str) {
	char chr;
	bool read_decim = false, mant_neg = false, exp_neg = false, is_sci = false;
	unsigned exp = 0;
	size_t index;
	double mant = 0, placeval = 0.1;

	for (index = 0; (chr = str[index]) && index <= MaxDec + mant_neg + read_decim; index++) {
		if (chr == '-') {
			if (is_sci)
				exp_neg = true;
			else
				mant_neg = true;
		} else if (chr == '.' && !read_decim)
			read_decim = true;
		else if (chr == 'E')
			is_sci = true;
		if (!read_decim && isdigit(chr)) {
			if (is_sci) {
				exp *= 10;
				exp += chr - 48;	// '0' = 48
			} else {
				mant *= 10;
				mant += chr - 48;
			}
		} else if (isdigit(chr)) {
			mant += placeval * (chr - 48);
			placeval /= 10;
		}
	}
	if (mant_neg)
		mant *= -1;
	if (abs(exp) > MaxExp) {
		setstat(ERR_OVERFLOW);
		return FAIL;
	}
	if (exp_neg)
		exp *= -1;
	return mant * pow(10, exp);
}