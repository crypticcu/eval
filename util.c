#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "parse.h"
#include "status.h"
#include "util.h"

char *dtos(double x, unsigned sig) {
	bool only_decimal = x < 1 && x > -1 && x, is_whole = isequal(x, (ssize_t) x);
	size_t nplaces = nwhole(x), reqsize = ndecim(x), index = 0;
	char *string;

	if (sig > reqsize)
		sig = reqsize;
	if (nplaces > MaxDec) {
		errstat(ERR_OVERFLOW);
		return NULL;
	}
	if (nplaces + sig > MaxDec)
		sig = MaxDec - nplaces;
	if (sig > MaxDec)
		sig = MaxDec;
	reqsize = nplaces + sig	// Digits
			+ !is_whole		// Decimal point
			+ only_decimal	// Leading zero
			+ 1,			// Negative/Positive sign
	string = (char *) calloc(reqsize + 1 /* Null character */, sizeof(char));
	if (!string) {
		errstat(ERR_INTERNAL);
		return NULL;
	}
	string[index++] = x < 0 ? '-' : '+';
	if (only_decimal) {
		string[index++] = '0';
		string[index++] = '.';
	}
	for (int place = nplaces - !(nplaces == MaxDec && is_whole); index < reqsize; index++, place--) {
		string[index] = getdigit(x, place) + 48;	// '0' = 48
		if (place == 0) {
			if (is_whole)
				break;
			else if (index + 1 != reqsize)
				string[++index] = '.';
		}
	}
	return string;
}

char *getln(size_t limit) {
	char *input = malloc(sizeof(char)), chr;
	size_t memsize = 1, index = 0;

	if (!input) {
		errstat(ERR_INTERNAL);
		return NULL;	
	}
	do {
		chr = getchar();
		if (index == memsize) {
			if (!(input = (char *) realloc(input, (memsize *= 2) * sizeof(char)))) {
				errstat(ERR_INTERNAL);
				return NULL;
			}
		}
		if (limit != 0)
			input[index++] = chr;
		if (chr == '\n')
			break;
	} while (index < limit - 1);
	if (index == memsize) {
		if (!(input = (char *) realloc(input, ++memsize * sizeof(char)))) {
			errstat(ERR_INTERNAL);
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
		errstat(ERR_INTERNAL);
		return NULL;
	}
	sub = (char *) calloc(
		high - low + 1	// # of characters being removed
		+ 1,			// Null character
		sizeof(char));
	if (!sub) {
		errstat(ERR_INTERNAL);
		return NULL;
	}
	for (size_t index_old = low, index_new = 0; index_old <= high; index_old++, index_new++)
		sub[index_new] = str[index_old];
	return sub;
}

char *pushsub(char *str, char *sub, size_t low, size_t high) {
	char *result;
	size_t index_old, index_new;

	if (low >= strlen(str) || high >= strlen(str) || low > high) {
		free(str);
		free(sub);
		errstat(ERR_INTERNAL);
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
		errstat(ERR_INTERNAL);
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

char *roundnum(const char *string, unsigned sig) {
	char *str, *curr, *next;
	size_t digitpos;

	if (!(str = strdup(string))) {
		errstat(ERR_INTERNAL);
		return NULL;
	}
	if (!strchr(str, '.'))
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
		errstat(ERR_INTERNAL);
		free(swap);
		return NULL;
	}
	index = 0;
	if (swap[0] == '-')
		str[index++] = '-';	// Negative sign, if present
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
		for (size_t off = 1; isspace(lead = expr[parpos - off]) && parpos - off - 1 >= 0; off--);
		for (size_t off = 1; isspace(trail = expr[parpos + off]) && trail; off++);
		return (isnumer(lead) && isnumer(trail) ||
				expr[parpos] == ')' && trail == '(');
	} else
		return false;
}

unsigned getdigit(double x, int place) {
	x = fabs(x);
	if (abs(place) > MaxDec || x > INTMAX_MAX)	// Place cannot be over/under place limit; Any 'x' over INTMAX_MAX causes overflow on conversion
		return 0;								// Digits that cannot be printed will come up as '0'
	return (ssize_t) (x *= pow(10, -place)) - (ssize_t) (x / 10) * 10;
}

unsigned ndecim(double x) {
	unsigned num;

	if (!x)	// 0 is printed as having 0 decimal places
		return 0;
	if (isnan(x) || isinf(x))
		return UINT_MAX;
	for (num = 0; x - (ssize_t) x; x *= 10, num++);
	return num;
}

unsigned nwhole(double x) {
	unsigned num;

	if (!x)	//	0 is printed as having 1 whole place
		return 1;
	if (isnan(x) || isinf(x))
		return UINT_MAX;
	for (num = 0; (ssize_t) x; x /= 10, num++);
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
			errstat(ERR_SYNTAX);
			invsynt(expr, index);
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
	char singl = '\0';	// Last single operator
	char doubl = '\0';	// Last double oeprator
	size_t nsingle = 0;	// Single operators
	size_t ndouble = 0;	// Double operators
	size_t npoint = 0;	// Decimal points
	size_t pcount = 0;	// # parsed in current expression

	for (size_t index = 0; (chr = expr[index]); index++) {
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
				errstat(ERR_SYNTAX);
				invsynt(expr, index);
				return index;
			}
			doubl = chr;
			ndouble++;
		}
		else if (strchr(ChrSets.opers, chr)) {
			if (chr != singl && isparity(chr))	/* Allow for situations such as */
				nsingle--;						/* '2/-2' without parentheses	*/
			singl = chr;
			nsingle++;
		}
		if (!isdigit(chr) && chr != '.')
			npoint = 0;
		else if (chr == '.')
			npoint++;
		if (nsingle == 2 || ndouble == 3 ||	npoint == 2	  ||	/* Extra operator or comma */
			!strchr(ChrSets.valid, chr) && !isspace(chr)  ||	/* Invalid character	   */
			isnumer(chr) && isnumer(lead) && lead != last) {	/* Two #'s side-by-side    */
			errstat(ERR_SYNTAX);
			invsynt(expr, index);
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
				lim = index - 1;
				break;
			} else if (!reading && (isnumer(chr) || isparity(chr)))
				reading = true;
		} else {
			if (reading && !isnumer(chr)) {
				lim = index + !isparity(chr);
				break;
			} else if (!reading && isnumer(chr))
				reading = true;
		}
	}
	if (!reading)	// No value found
		return operpos;
	else {
		if (index == -1)	// Reached beginning of expression
			lim = 0;
		else if (chr == 0)	// Reached end of expression
			lim = strlen(expr) ? strlen(expr) - 1 : strlen(expr);
	}
	return lim;
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
			} else if (!reading && (isnumer(chr) || isparity(chr))) {
				reading = true;
				val_low = index;
			}
		} else {
			if (reading && !isnumer(chr)) {
				val_low = index + !isparity(chr);
				break;
			} else if (!reading && isnumer(chr)) {
				reading = true;
				val_high = index;
			}
		}
	}
	if (!reading)	// No value found
		return 0;	// Must return 0 in this case to ensure proper parse_sub() functionality (used in unary + and -)
	else if (index == -1)	// Reached beginning of expression
		val_low = 0;
	else if (chr == 0)	// Reached end of expression
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
	bool read_decim = false, is_negative = false, reading = false;
	size_t index;
	double num = 0, placeval = 0.1;

	for (index = 0; (chr = str[index]) && index <= MaxDec + is_negative + read_decim; index++) {	// MaxDec is accurate digit limit
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
	if (index > MaxDec && !read_decim) {
		errstat(ERR_OVERFLOW);
		return FAIL;
	}
	if (is_negative)
		num *= -1;
	return num;
}