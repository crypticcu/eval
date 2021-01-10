#include <stdbool.h>

#ifndef PARSE_H
#define PARSE_H

#ifdef __GNU_LIBRARY__
#include <err.h>

#define PROG_NAME\
	program_invocation_name[1] == '/' ?\
	program_invocation_name + 2 :\
	program_invocation_name				// Ignore './' if included 

extern char *program_invocation_name;
#else
#define PROG_NAME	"parsr"
#endif /* #ifdef __GNU_LIBRARY__ */

/* Initializes left- and right-hand values in evaluate() */
#define INIT_VALS()\
	lval = getval(expr, nchr, 'l');\
	rval = getval(expr, nchr, 'r');\
	llim = getlim(expr, nchr, 'l');\
	rlim = getlim(expr, nchr, 'r')

/* Initializes new expression string in evaluate() */
#define INIT_EXPR()\
	if ((result_str = dtos(result, LDBL_DIG)) == NULL) {\
		free(expr);\
		fail("Internal error (evaluate.dtos)");\
	}\
	if ((expr = pushsub(expr, result_str, llim, rlim)) == NULL) {\
		free(result_str);\
		fail("Internal error (evaluate.pushsub)");\
	}

#define INT_FAIL	INT_MAX	// Passed by [type]-returning functions on failure
#define DBL_FAIL	DBL_MAX
#define CHK_PASS	-1		// Used by chk_parenth() and chk_syntax(); Indicates valid syntax
#define OBS_R		1		// Used by fobst(); Indicates obstruction in operation
#define OBS_L		2

static bool DEBUG = false;
static const char *VAL_CHRS = "+-!^*/%.()\n1234567890",
				  *OPERS_A = "+-!^*/%",	// All
		  		  *OPERS_S = "^*/%",	// Single
		  		  *OPERS_D = "+-!";		// Double

/* Prints error message and exits program */
extern void fail(const char *_desc);

/* Prints string with character at given position highlighted red */
extern void printh(const char *_s, unsigned _hpos);

/* Returns string representation of double
 * Returns NULL on failure
 * Resulting string must be freed */
extern char *dtos(double _x, unsigned _sig);

/* Returns new, allocated substring spanning the given elements
 * Returns NULL on failure */
extern char *popsub(const char *_s, unsigned _low, unsigned _high);

/* Returns new, allocated string where substring replaces given elements in old string
 * Frees given string and substring while returning a newly allocated one
 * Returns NULL on failure */
extern char *pushsub(char *_s, char *_sub, unsigned _low, unsigned _high);

/* Evaluates mathemetical expression starting from given index position
 * Returns string representation of result depending on index
 * Returns NULL on failure */
extern char *simplify(const char *_expr, unsigned _from);

/* Returns true if floating-point numbers are equal */
extern bool isequal(double _x, double _y);

/* Returns true if character is in string */
extern bool isin(const char _x, const char *_y);

/* Returns true if character is numerical (digit || '-' || '.') */
extern bool isnumer(char _c);

/* Determines whether a parenthesis indicates multiplication */
extern bool toast(const char *_expr, unsigned _parpos);

/* Returns OBS_R or OBS_L depending on type of obstruction(s)
 * If none are found, returns 0 */
extern unsigned isclr(const char *_expr, unsigned _operpos, unsigned _low, unsigned _high);

/* Get digit at given place */
extern unsigned getdigit(double _x, int _place);

/* Returns number of whole places */
extern unsigned nplaces(double _x);

/* Returns index position of first invalid parenthesis of expression
 * Returns CHK_PASS if no invalid parentheses are found */
extern int chk_parenth(const char *_expr);

/* Returns index position of first invalid character of expression
 * Returns CHK_PASS if no invalid characters are found */
extern int chk_syntax(const char *_expr);

/* Returns left- or right-hand limit of range of operation at given position in expression
 * Returns INT_FAIL if invalid direction or operand is missing */
extern int getlim(char *_expr, unsigned _operpos, char _dir);

/* Returns number of indices operator is from closest obstruction in operation
 * If none are found, returns 0
 * Returns INT_FAIL on failure */
extern unsigned fobst(const char *_expr, unsigned _operpos, unsigned _llim, unsigned _rlim);

/* Evaluates mathematical expression from index position 0
 * Ignores parentheses and syntax errors
 * Returns DBL_FAIL on failure */
extern double evaluate(const char *_expr);

/* Returns left- or right-hand value of the operand at given position in the expression
 * Returns DBL_FAIL on failure */
extern double getval(char *_expr, unsigned _operpos, char _dir);

/* Returns double representation of string
 * Stops program on failure */
extern double stod(const char *_s);

#endif /* #ifndef PARSE_H */