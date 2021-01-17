#include <stdbool.h>

#ifndef PARSE_H
#define PARSE_H


/* FUNCTION-LIKE MACROS **************************************************************************/

/* Initializes left- and right-hand values in evaluate() */
#define INIT_VALS()\
	lval = getval(expr, nchr, 'l');\
	rval = getval(expr, nchr, 'r');\
	llim = getlim(expr, nchr, 'l');\
	rlim = getlim(expr, nchr, 'r')

/* Initializes new expression string in evaluate() */
#define INIT_EXPR()\
	if ((result_str = dtos(result, DBL_DIG)) == NULL) {\
		free(expr);\
		fail("Internal error (evaluate.dtos)");\
	}\
	if ((expr = pushsub(expr, result_str, llim, rlim)) == NULL) {\
		free(result_str);\
		fail("Internal error (evaluate.pushsub)");\
	}

/* Checks for overflow, getval() failure, and missing operand(s) in evaluate()
 * Requires needed values, LEFT or RIGHT */
#define CHK_VALS(reqval)\
	if (isequal(rval, DBL_OVER) || isequal(lval, DBL_OVER))\
		goto overflow_err;\
	if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))\
		goto getval_err;\
	if (reqval & RIGHT && rlim == INT_FAIL)\
		goto opermiss_err;\
	if (reqval & LEFT && llim == INT_FAIL)\
		goto opermiss_err

/* MACROS ****************************************************************************************/

/* Debug */
#define DEBUG		0			// 1 = Debugging on

/* Error Handling */
#define INT_FAIL	INT_MAX		// Passed on failure
#define DBL_FAIL	DBL_MAX
#define STR_OVER	"..."		// Passed on overflow
#define DBL_OVER	FLT_EPSILON

/* Program name */
#ifdef __GNU_LIBRARY__
#include <err.h>
#define PROG_NAME\
	program_invocation_name[1] == '/' ?\
	program_invocation_name + 2 :\
	program_invocation_name		// Ignore './' if included 

extern char *program_invocation_name;
#else
#define PROG_NAME	"eval"
#endif /* #ifdef __GNU_LIBRARY__ */

/* Misc */
#define CHK_PASS	-1			// Indicates valid syntax
#define RIGHT		1
#define LEFT		2

/* VARIABLES *************************************************************************************/

typedef const struct {
	char *valid,
		 *digits,
		 *opers,
		 *unary,
		 *binary;
} _character_sets;

typedef struct {
	bool help,
		 round;
} _flags;

extern _character_sets chrsets;
extern _flags flags;
extern bool cmd_line;

/* FUNCTIONS *************************************************************************************/

/* Prints error message 
 * Exits program if in command-line interface */
extern void fail(const char *_desc);

/* Prints string with character at given position underlined */
extern void printu(const char *_s, size_t _hpos);

/* Returns string representation of double
 * Returns NULL on failure
 * Returns OVERFLOW on overflow
 * Resulting string must be freed */
extern char *dtos(double _x, size_t _sig);

/* Returns new, allocated substring spanning the given elements
 * Returns NULL on failure */
extern char *popsub(const char *_s, size_t _low, size_t _high);

/* Returns new, allocated string where substring replaces given elements in old string
 * Frees given string and substring while returning a newly allocated one
 * Returns NULL on failure */
extern char *pushsub(char *_s, char *_sub, size_t _low, size_t _high);

/* Evaluates mathemetical expression starting from given index position
 * Returns string representation of result depending on index
 * Returns NULL on failure */
extern char *simplify(const char *_expr, size_t _from);

/* Returns true if floating-point numbers are equal */
extern bool isequal(double _x, double _y);

/* Returns true if character is in string */
extern bool isin(const char _x, const char *_y);

/* Returns true if character is numerical (digit || '-' || '.') */
extern bool isnumer(char _c);

/* Determines whether a parenthesis indicates multiplication */
extern bool toast(const char *_expr, size_t _parpos);

/* Get digit at given place */
extern size_t getdigit(double _x, int _place);

/* Returns number of whole places */
extern size_t nplaces(double _x);

/* Returns index position of first invalid parenthesis of expression
 * Returns CHK_PASS if no invalid parentheses are found */
extern int chk_parenth(const char *_expr);

/* Returns index position of first invalid character of expression
 * Returns CHK_PASS if no invalid characters are found */
extern int chk_syntax(const char *_expr);

/* Returns left- or right-hand limit of range of operation at given position in expression
 * Returns INT_FAIL if invalid direction or operand is missing */
extern int getlim(char *_expr, size_t _operpos, char _dir);

/* Returns OBS_R or OBS_L depending on type of obstruction(s)
 * If none are found, returns 0
 * Returns INT_FAIL on failure */
extern size_t fobst(const char *_expr, size_t _operpos, size_t _llim, size_t _rlim);

/* Evaluates mathematical expression from index position 0
 * Ignores parentheses and syntax errors
 * Returns DBL_FAIL on failure */
extern double evaluate(const char *_expr);

/* Returns left- or right-hand value of the operand at given position in the expression
 * Returns DBL_FAIL on failure */
extern double getval(char *_expr, size_t _operpos, char _dir);

/* Returns double representation of string
 * Returns DBL_FAIL if string does not contain double
 * Returns DBL_OVER on overflow */
extern double stod(const char *_s);

#endif /* #ifndef PARSE_H */