#ifndef UTIL_H
#define UTIL_H

#include <ctype.h>		// isdigit()
#include <float.h>		// FLT_EPSILON
#include <math.h>		// fabs()
#include <stdbool.h>	// bool
#include <stddef.h>		// size_t
#include <stdint.h>		// intmax_t
#include "global.h"		// ssize_t

/* Returns true if floating-point numbers are equal */
#define isequal(x, y)	(fabs((x) - (y)) < FLT_EPSILON)

/* Returns true if character represents parity */
#define isparity(chr)	((chr) == '+' || (chr) == '-')

/* Returns true if character is a parenthesis */
#define isparenth(chr)	((chr) == '(' || (chr) == ')')

/* Returns true if character is part of a number */
#define isnum(chr)		(isdigit(chr) || (chr) == '.' || (chr) == 'E')

/* Returns true if character is numerical */
#define isnumer(chr)	(isnum(chr) || isparity(chr))

/* Returns true if floating-point number is whole */
#define iswhole(x)		isequal(x, (intmax_t) (x))

/* Prints a portion of given string according to format escape code */
extern void fprint(const char *str, size_t begin, size_t end, format_t fmt);

/* Returns null-terminated, malloc'd string representation of double
 * On success, result must be freed
 * Returns NULL on failure */
extern char *dtos(double x, unsigned sig)
attribute(__warn_unused_result__);

/* Returns null-terminated, malloc'd string input from stdin to a certain number of characters, including null character
 * On success, result must be freed, even when no input is given 
 * Returns NULL on failure */
extern char *getln(size_t lim)
attribute(__warn_unused_result__);

/* Returns new, malloc'd substring spanning the given indices
 * On success, result must be freed
 * Returns NULL on failure */
extern char *popsub(const char *str, size_t low, size_t high)
attribute(__warn_unused_result__, __nonnull__(1));

/* Beautifies number string
 * Returns modified string on success
 * Returns NULL on failure */
extern char *pprint(const char *str)
attribute(__warn_unused_result__, __nonnull__(1));

/* Returns new, null-terminated, allocated string where substring replaces given indices in old string
 * Frees original string and substring while returning a newly allocated one
 * On success, result must be freed
 * Returns NULL on failure */
extern char *pushsub(char *str, char *sub, size_t low, size_t high)
attribute(__warn_unused_result__, __nonnull__(1));

/* Returns rounded equivalent of string representation of double
 * Frees original string
 * On success, result must be freed
 * Returns NULL on failure */
extern char *roundnum(char *str, unsigned sig)
attribute(__warn_unused_result__, __nonnull__(1));

/* Returns the same string rounded from given position
 * Frees original string
 * On success, result must be freed
 * Returns NULL on failure */
extern char *roundfrom(char *str, size_t digitpos, direct_t dir)
attribute(__warn_unused_result__, __nonnull__(1));

/* Determines whether a parenthesis indicates multiplication */
extern bool toast(const char *expr, size_t parpos)
attribute(__nonnull__(1));

/* Get digit at given place
 * Returns '0' if infinity or NaN */
extern char getdigit(double x, int place);

/* Get exponent part of scientific notation
 * Returns 0 if infinity or NaN */
extern unsigned getexp(double x);

/* Number of decimal places
 * Returns UINT_MAX if infinity or NaN */
extern unsigned ndecim(double x);

/* Number of whole places
 * Returns UINT_MAX if infinity or NaN */
extern unsigned nwhole(double x);

/* Returns index position of first invalid parenthesis of expression
 * Returns PASS if none is found */
extern ssize_t chk_parenth(const char *expr)
attribute(__nonnull__(1));

/* Returns index position of first invalid character of expression
 * Returns PASS if none is found */
extern ssize_t chk_syntax(const char *expr)
attribute(__nonnull__(1));

/* Returns left- or right-hand limit of range of operation at operand position */
extern ssize_t getlim(char *expr, size_t operpos, direct_t dir)
attribute(__nonnull__(1));

/* Returns mantissa part of scientific notation
 * Returns 0 if infinity or NaN */
extern double getmant(double x);

/* Returns left- or right-hand value of the operand at given position in the expression
 * Returns FAIL on failure, or 0 if no value is found */
extern double getval(char *expr, size_t operpos, direct_t dir)
attribute(__nonnull__(1));

/* Returns double representation of string
 * Returns FAIL on overflow */
extern double stod(const char *str)
attribute(__nonnull__(1));

#endif // #ifndef UTIL_H