#include <stdbool.h>

#ifndef PARSE_H
#define PARSE_H

/* Initializes left- and right-hand values
 * Scope: evaluate() */
#define INIT_VALS()\
	lval = getval(modexpr, nchr, 'l');\
	rval = getval(modexpr, nchr, 'r');\
	llim = getlim(modexpr, nchr, 'l');\
	rlim = getlim(modexpr, nchr, 'r')

/* Initializes new expression string in evaluate()
 * Scope: evaluate() */
#define INIT_EXPR()\
	if ((result_str = dtos(result, DBL_DIG)) == NULL)					goto dtos_err;\
	if (!strcmp(result_str, STR_OVER))									goto overflow_err;\
	if ((modexpr = pushsub(modexpr, result_str, llim, rlim)) == NULL)	goto pushsub_err

/* Checks for overflow, getval() failure, and missing operand(s)
 * Requires needed values, LEFT or RIGHT
 * Scope: evaluate() */
#define CHK_VALS(reqval)\
	if (fobst(modexpr, nchr, llim, rlim) & (reqval))			continue;\
	if (isequal(rval, DBL_OVER) || isequal(lval, DBL_OVER))		goto overflow_err;\
	if (isequal(rval, DBL_FAIL) || isequal(lval, DBL_FAIL))		goto getval_err;\
	if ((reqval) & RIGHT && rlim == INT_FAIL)					goto opermiss_err;\
	if ((reqval) & LEFT && llim == INT_FAIL)					goto opermiss_err

/* Free memory and return a value
 * Scope: any */
#define CLEANUP(memory, retval)\
	if (memory != NULL)\
		free(memory);\
	return retval

#ifdef __GNU_LIBRARY__
#include <err.h>
extern char *program_invocation_short_name;
#define PROG_NAME	program_invocation_short_name
#else
#define PROG_NAME	"parse"
#endif

#define DEBUG		0			// 1 = Debugging on
#define INT_FAIL	INT_MAX		// Passed on failure
#define DBL_FAIL	DBL_MAX
#define STR_OVER	"..."		// Passed on overflow
#define DBL_OVER	FLT_EPSILON
#define CHK_PASS	-1			// Indicates valid syntax
#define RIGHT		1
#define LEFT		2

static const struct CharacterSets {
	char *valid,
		 *digits,
		 *opers,
		 *doubl;
} chrsets = {
	"+-!^*/%.()1234567890\'",	// Valid characters
	"1234567890",				// Digits
	"+-!^*/%",					// Operators
	"+-!",						// Double operators
};
static struct Flags {
	bool help,
		 round,
		 radian;
} flags = {
	false,						// Show help				-h
	false,						// Round to # of decimals	-d
	false						// Radian mode				-r
};
extern bool cmd_line;

/* Prints error message 
 * Exits program if in command-line interface */
extern void fail(const char *desc);

/* Prints the help page */
extern void print_help(void);

/* Prints string with character at given position underlined */
extern void printu(const char *string, size_t pos);

/* Returns string representation of double
 * Returns NULL on failure
 * Returns OVERFLOW on overflow
 * Resulting string must be freed */
extern char *dtos(double x, unsigned sig);

/* Returns new, allocated substring spanning the given elements
 * Returns NULL on failure */
extern char *popsub(const char *string, size_t low, size_t high);

/* Returns new, allocated string where substring replaces given elements in old string
 * Frees given string and substring while returning a newly allocated one
 * Returns NULL on failure */
extern char *pushsub(char *string, char *sub, size_t low, size_t high);

/* Evaluates mathemetical expression starting from given index position
 * Returns string representation of result depending on index
 * Returns NULL on failure */
extern char *simplify(const char *expr, size_t from);

/* Returns true if floating-point numbers are equal */
extern bool isequal(double x, double y);

/* Returns true if character is numerical */
extern bool isnumer(char chr);

/* Determines whether a parenthesis indicates multiplication */
extern bool toast(const char *expr, size_t parpos);

/* Get digit at given place */
extern unsigned getdigit(double x, int place);

/* Returns number of whole places */
extern unsigned nplaces(double x);

/* Returns OBS_R or OBS_L depending on type of obstruction(s)
 * If none are found, returns 0
 * Returns INT_FAIL on failure */
extern int fobst(const char *expr, size_t operpos, size_t llim, size_t rlim);

/* Returns index position of first invalid parenthesis of expression
 * Returns CHK_PASS if no invalid parentheses are found */
extern long long chk_parenth(const char *expr);

/* Returns index position of first invalid character of expression
 * Returns CHK_PASS if no invalid characters are found */
extern long long chk_syntax(const char *expr);

/* Returns left- or right-hand limit of range of operation at given position in expression
 * Returns INT_FAIL if invalid direction or operand is missing */
extern long long getlim(char *expr, size_t operpos, char dir);

/* Evaluates mathematical expression from index position 0
 * Ignores parentheses and syntax errors
 * Returns DBL_FAIL on failure */
extern double evaluate(const char *expr);

/* Returns left- or right-hand value of the operand at given position in the expression
 * Returns DBL_FAIL on failure */
extern double getval(char *expr, size_t operpos, char dir);

/* Returns double representation of string
 * Returns DBL_OVER on overflow */
extern double stod(const char *string);

#endif /* #ifndef PARSE_H */