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
 * On success, result must be freed
 * Returns NULL on failure */
extern char *dtos(double x, unsigned sig)
attribute(__warn_unused_result__);

/* Returns null-terminated, malloc'd string input from stdin to a certain number of characters, including null character
 * On success, result must be freed, even when no input is given 
 * Returns NULL on failure */
extern char *getln(size_t limit)
attribute(__warn_unused_result__);

/* Returns new, malloc'd substring spanning the given indices
 * On success, result must be freed
 * Returns NULL on failure */
extern char *popsub(const char *str, size_t low, size_t high)
attribute(__warn_unused_result__);

/* Returns new, null-terminated, allocated string where substring replaces given indices in old string
 * Frees given string and substring while returning a newly allocated one
 * On success, result must be freed
 * Returns NULL on failure */
extern char *pushsub(char *str, char *sub, size_t low, size_t high)
attribute(__warn_unused_result__);

/* Returns rounded equivalent of string representation of double
 * Frees original string
 * On success, result must be freed
 * Returns NULL on failure */
extern char *roundnum(const char *string, unsigned sig)
attribute(__warn_unused_result__);

/* Returns the same string rounded up by 1 from given position
 * Frees original string
 * On success, result must be freed
 * Returns NULL on failure */
extern char *roundfrom(char *str, size_t digitpos, direct_t dir)
attribute(__warn_unused_result__);

/* Determines whether a parenthesis indicates multiplication */
extern bool toast(const char *expr, size_t parpos);

/* Get digit at given place */
extern unsigned getdigit(double x, int place);

/* Returns number of decimal places */
extern unsigned ndecim(double x);

/* Returns number of whole places */
extern unsigned nwhole(double x);

/* Returns index position of first invalid parenthesis of expression
 * Returns PASS if none is found */
extern ssize_t chk_parenth(const char *expr);

/* Returns index position of first invalid character of expression
 * Returns PASS if none is found */
extern ssize_t chk_syntax(const char *expr);

/* Returns left- or right-hand limit of range of operation at given position in expression
 * Returns FAIL on failure */
extern ssize_t getlim(char *expr, size_t operpos, direct_t dir);

/* Returns left- or right-hand value of the operand at given position in the expression
 * Returns FAIL on failure */
extern double getval(char *expr, size_t operpos, direct_t dir);

/* Returns double representation of string
 * Returns FAIL on overflow */
extern double stod(const char *str);

#endif // #ifndef UTIL_H