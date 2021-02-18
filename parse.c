#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "parse.h"
#include "status.h"
#include "util.h"

char *parse(const char *expression, unsigned sig, void *null) {
	bool read_parenth = false;
	char *sub, *expr, chr;
	size_t par_low, par_high, ignore, index;

	if (expression == NULL)
		return NULL;
	expr = strdup(expression);
	if (expr == NULL)
		return NULL;
	if (!null) {	// Check syntax (once)
		if (chk_syntax(expression) != PASS || chk_parenth(expression) != PASS) {
			free(expr);
			return NULL;
		}
	}
	for (index = (size_t) null; index < strlen(expr); index++) {	// Check index before retrieving character (dynamic buffer)
		chr = expr[index];
		if (chr == ')') {
			par_high = index;
			read_parenth = false;
			expr[par_low] = (to_ast(expr, par_low)) ? '*' : ' ';
			expr[par_high] = (to_ast(expr, par_high)) ? '*' : ' ';
			if (par_low + 1 < par_high - 1) {	// Ensure parentheses are not empty
				if (!(sub = popsub(expr, par_low + 1, par_high - 1))) {
					free(expr);
					return NULL;
				}
				if (!parse_expr(&sub, sig)) {
					free(expr);
					free(sub);
					return NULL;
				}
				if (!(expr = pushsub(expr, sub, par_low + 1, par_high - 1)))
					return NULL;
			}
			if (null)
				return expr;
		} else if (chr == '(') {
			if (read_parenth) {
				sub = expr;	// Swap causes 'still reachable' error in valgrind
				expr = parse(expr, sig, (void *) index);
				free(sub);
				if (!expr)
					return NULL;
			} else {
				read_parenth = true;
				par_low = index;
			}
		}
	}
	if(!parse_expr(&expr, sig)) {
		free(expr);
		return NULL;
	}
	ignore = strspn(expr, " ");
	if (expr[ignore] == '+')
		ignore++;
	if (ignore > 0) {
		sub = strdup(expr + ignore);
		free(expr);
		expr = sub;
	}
	return expr;
}

char *parse_expr(char **expr_addr, unsigned sig) {
	char *expr;
	size_t ignore = 0;

	expr = *expr_addr;
	while (true) {
		ignore = strspn(expr, " ");
		if (isparity(expr[ignore]))
			ignore++;
		if (ignore == strlen(expr) || strcspn(expr + ignore, chrsets.opers) == strlen(expr + ignore))
			break;
		if (!(parse_oper(&expr, "++", sig)))	return NULL;
		if (!(parse_oper(&expr, "--", sig)))	return NULL;
		if (!(parse_oper(&expr, "!!", sig)))	return NULL;
		if (!(parse_oper(&expr,  "!", sig)))	return NULL;
		if (!(parse_oper(&expr,  "^", sig)))	return NULL;
		if (!(parse_oper(&expr,  "*", sig)))	return NULL;
		if (!(parse_oper(&expr,  "/", sig)))	return NULL;
		if (!(parse_oper(&expr,  "%", sig)))	return NULL;
		if (!(parse_oper(&expr,  "+", sig)))	return NULL;
		if (!(parse_oper(&expr,  "-", sig)))	return NULL;
	}
	return (*expr_addr = expr);
}

char *parse_oper(char **expr_addr, const char *oper, unsigned sig) {
	char *expr, *sub, chr;
	int reqval;
	size_t opernum;
	ssize_t llim, rlim;	// Left- and right-hand limits of operation
	double lval, rval;	// Left and right values of operation
	double result;
	size_t index;
	expr = *expr_addr;
	opernum = strlen(oper);
	if (!expr || opernum == 0 || opernum > 2) {
		errstat(ERR_INTERNAL);
		return NULL;
	}
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
	default:
		return NULL;
	}
	for (size_t index = 0; index < strlen(expr); index++) {	// Check index before retrieving character (dynamic buffer)
		chr = expr[index];
		if (opernum == 1 ? chr == oper[0] : chr == oper[0] && expr[index + 1] == oper[0]) {
			lval = getval(expr, index, 'l');
			llim = getlim(expr, index, 'l');
			rval = getval(expr, opernum == 1 ? index : index + 1, 'r');
			rlim = getlim(expr, opernum == 1 ? index : index + 1, 'r');
			if (fobst(expr, index, llim, rlim) & reqval)
				continue;
			if (isequal(rval, FAIL) || isequal(lval, FAIL))
				return NULL;
			if (reqval & RIGHT && rlim == FAIL || reqval & LEFT  && llim == FAIL) {
				errstat(ERR_MISSOPER);
				return NULL;
			}
			switch(oper[0]) {
			case '^':				// Exponent
				result = pow(lval, rval);
				break;
			case '*':				// Muliplication
				result = lval * rval;
				break;
			case '/':				// Division
				if (!rval) {
					errstat(ERR_DIVZERO);
					return NULL;
				}
				result = lval / rval;
				break;
			case '%':				// Modulus
				if (!isequal(lval, (ssize_t) lval) || !isequal(rval, (ssize_t) rval)) {
					errstat(ERR_MODULO);
					return NULL;
				}
				result = (ssize_t) lval % (ssize_t) rval;
				break;
			case '+':
				if (opernum == 1) {	// Add/Unary plus
					if (llim == FAIL)
						llim = index;
					result = lval + rval;
				} else {			// Increment
					result = rval + 1;
					llim = index;
				}
				break;
			case '-':
				if (opernum == 1) {	// Subtract/Unary minus
					if (llim == FAIL)
						llim = index;
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
				if (rval < 0 && (ssize_t) lval % 2 == 0) {
					errstat(ERR_EVENROOT);
					return NULL;
				}
				if (!lval) {
					errstat(ERR_DIVZERO);
					return NULL;
				}
				result = rval < 0 ? -pow(-rval, 1 / lval) : pow(rval, 1 / lval);
				break;
			}
			if (!(sub = dtos(result, sig))) {
				return NULL;
			}
			if (!(expr = pushsub(expr, sub, llim, rlim))) {
				free(sub);
				return NULL;
			}
		}
	}
	return (*expr_addr = expr);
}