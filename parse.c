#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "parse.h"
#include "status.h"
#include "util.h"

char *parse(const char *expression, unsigned sig, void *null) {
	char *sub, chr;
	char *expr;	// Dynamic buffer
	bool read_parenth = false;
	size_t par_low, par_high, ignore, index;

	if (!(expr = strdup(expression))) {
		setstat(ERR_INTERNAL);
		return NULL;
	}
	if (!null) {	// Check syntax once
		if (chk_syntax(expression) != PASS || chk_parenth(expression) != PASS) {
			free(expr);
			return NULL;
		}
	}
	for (index = (size_t) null; index < strlen(expr); index++) {
		chr = expr[index];
		if (chr == ')') {
			par_high = index;
			read_parenth = false;
			expr[par_low] = (toast(expr, par_low)) ? '*' : ' ';
			expr[par_high] = (toast(expr, par_high)) ? '*' : ' ';
			if (par_low + 1 < par_high - 1) {	// Ensure parentheses are not empty
				if (!(sub = popsub(expr, par_low + 1, par_high - 1))) {
					free(expr);
					return NULL;
				}
							if (!parse_sub(&sub)) {
					free(expr);
					free(sub);
					return NULL;
				}
				if (!(expr = pushsub(expr, sub, par_low + 1, par_high - 1)))
					return NULL;
			}
			if (null)
				return expr;
			index = 0;
		} else if (chr == '(') {
			if (read_parenth) {
				sub = expr;
				expr = parse(expr, sig, (void *) index);
				free(sub);
				if (!expr)
					return NULL;
				index = 0;
			} else {
				read_parenth = true;
				par_low = index;
			}
		}
	}
	if (!parse_sub(&expr)) {
		free(expr);
		return NULL;
	}
	sub = expr;
	expr = pprint(expr);
	free(sub);
	if (!expr)
		return NULL;
	return roundnum(expr, sig);
}

char *parse_sub(char **expr_addr) {
	char *expr = *expr_addr;	// Dynamic buffer
	size_t ignore = 0;

	while (true) {
		ignore = strspn(expr, " ");
		if (isparity(expr[ignore]))
			ignore++;
		if (ignore == strlen(expr) || strcspn(expr + ignore, ChrSets.opers) == strlen(expr + ignore))
			break;
		if (!(parse_oper(&expr, "++")))	return NULL;
		if (!(parse_oper(&expr, "--")))	return NULL;
		if (!(parse_oper(&expr, "!!")))	return NULL;
		if (!(parse_oper(&expr,  "!")))	return NULL;
		if (!(parse_oper(&expr,  "^")))	return NULL;
		if (!(parse_oper(&expr,  "/")))	return NULL;
		if (!(parse_oper(&expr,  "*")))	return NULL;
		if (!(parse_oper(&expr,  "%")))	return NULL;
		if (!(parse_oper(&expr,  "-")))	return NULL;
		if (!(parse_oper(&expr,  "+")))	return NULL;
	}
	return (*expr_addr = expr);
}

char *parse_oper(char **expr_addr, const char *oper) {
	char *sub, chr;
	char *expr = *expr_addr;	// Dynamic buffer
	int reqval;
	size_t opernum;
	ssize_t llim, rlim;	// Left- and right-hand limits of operation
	double lval, rval;	// Left and right values of operation
	double result;
	size_t index;

	opernum = strlen(oper);
	for (size_t index = 0; index < strlen(expr); index++) {
		chr = expr[index];
		if (opernum == 1 ? chr == oper[0] : chr == oper[0] && expr[index + 1] == oper[0]) {
			switch(oper[0]) {
			case '^': case '*': case '/': case '%':
				reqval = LEFT|RIGHT;
				break;
			case '+': case '-':
				reqval = RIGHT;
				break;
			case '!':
				reqval = opernum == 1 ? RIGHT: LEFT|RIGHT;
			break;
			}
			lval = getval(expr, index, LEFT);
			llim = getlim(expr, index, LEFT);
			rval = getval(expr, opernum == 1 ? index : index + 1, RIGHT);
			rlim = getlim(expr, opernum == 1 ? index : index + 1, RIGHT);
			if (isequal(rval, FAIL) || isequal(lval, FAIL))
				return NULL;
			if (rlim == index || reqval & LEFT  && llim == index) {
				setstat(ERR_MISSOPER);
				return NULL;
			}
			switch(chr) {
			case '^':				// Exponent
				result = pow(lval, rval);
				break;
			case '*':				// Muliplication
				result = lval * rval;
				break;
			case '/':				// Division
				if (!rval) {
					setstat(ERR_DIVZERO);
					return NULL;
				}
				result = lval / rval;
				break;
			case '%':				// Modulus
				result = lval < 0 ? rval - lval : fmod(lval, rval);
				break;
			case '+':
				if (opernum == 1) {	// Add
					result = lval + rval;
				} else {			// Increment
					result = rval + 1;
					llim = index;
				}
				break;
			case '-':
				if (opernum == 1) {	// Subtract/Unary minus
					result = lval - rval;
				} else {			// Decrement
					result = rval - 1;
					llim = index;
				}
				break;
			case '!':				// Square root/Other root
				if (opernum == 1) {
					lval = 2;
					llim = index;
				}
				if (rval < 0 && (intmax_t) lval % 2 == 0) {
					setstat(ERR_IMAGINARY);
					return NULL;
				}
				if (!lval) {
					setstat(ERR_DIVZERO);
					return NULL;
				}
				result = rval < 0 ? -pow(-rval, 1 / lval) : pow(rval, 1 / lval);
				break;
			}
			if (!(sub = dtos(result, MaxDec)))
				return NULL;
			if (!(expr = pushsub(expr, sub, llim, rlim)))
				return NULL;
			index = strspn(expr, " ");
			if (isparity(expr[index]))
				index++;
		}
	}
	return (*expr_addr = expr);
}