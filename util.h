#ifndef UTIL_H
#define UTIL_H

#include <ctype.h>		// isdigit()
#include <float.h>		// FLT_EPSILON
#include <math.h>		// fabs()
#include <stdbool.h>	// bool
#include <stddef.h>		// size_t
#include "global.h"		// ssize_t (if needed)

/* Returns true if floating-point numbers are equal */
#define isequal(x, y)	(fabs((x) - (y)) < FLT_EPSILON)

/* Returns true if character is numerical */
#define isnumer(chr)	(isdigit(chr) || (chr) == '.')

/* Returns true if character represents parity */
#define isparity(chr)	((chr) == '+' || (chr) == '-')

/* Returns null-terminated, malloc'd string representation of double
 * Returns NULL on failure
 * Resulting string must be freed */
extern char *dtos(double x, unsigned sig);

/* Returns null-terminated, malloc'd string input from stdin to a certain number of characters
 * Returns NULL on failure
 * Resulting string must be freed, even when no input is given */
extern char *getln(size_t limit);

/* Returns new, malloc'd substring spanning the given indices
 * Returns NULL on failure */
extern char *popsub(const char *string, size_t low, size_t high);

/* Returns new, null-terminated, allocated string where substring replaces given indices in old string
 * Frees given string and substring while returning a newly allocated one
 * Returns NULL on failure */
extern char *pushsub(char *string, char *sub, size_t low, size_t high);

/* Determines whether a parenthesis indicates multiplication */
extern bool to_ast(const char *expr, size_t parpos);

/* Get digit at given place */
extern unsigned getdigit(double x, int place);

/* Returns number of whole places */
extern unsigned nplaces(double x);

/* Returns direction of obstruction(s)
 * If none are found, returns 0
 * Returns FAIL on failure */
extern int fobst(const char *expr, size_t operpos, size_t llim, size_t rlim);

/* Returns index position of first invalid parenthesis of expression
 * Returns PASS if none is found */
extern ssize_t chk_parenth(const char *expr);

/* Returns index position of first invalid character of expression
 * Returns PASS if none is found */
extern ssize_t chk_syntax(const char *expr);

/* Returns left- or right-hand limit of range of operation at given position in expression
 * Returns FAIL on failure */
extern ssize_t getlim(char *expr, size_t operpos, char dir);

/* Returns left- or right-hand value of the operand at given position in the expression
 * Returns DBL_FAIL on failure */
extern double getval(char *expr, size_t operpos, char dir);

/* Returns double representation of string
 * Returns DBL_OVER on overflow */
extern double stod(const char *string);

#endif /* #ifndef UTIL_H */