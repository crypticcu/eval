#include <stdbool.h>

#ifndef PARSE_H
#define PARSE_H

/* Initializes left- and right-hand values in evaluate() */
#define INIT_VALS()\
	lval = getval(expr, nchr, 'l');\
	rval = getval(expr, nchr, 'r');\
	llim = getlim(expr, nchr, 'l');\
	rlim = getlim(expr, nchr, 'r')\

/* Initializes new expression string in evaluate() */
#define INIT_EXPR()\
	if ((result_str = dtos(result, DBL_DIG)) == NULL) {\
		free(expr);\
		errx(EXIT_FAILURE, "Internal error (evaluate.dtos)");\
	}\
	if ((expr = pushsub(expr, result_str, llim, rlim)) == NULL) {\
		free(result_str);\
		errx(EXIT_FAILURE, "Internal error (evaluate.pushsub)");\
	}

/* Program name macro
 * Ignores './' if included */
#define PROG_NAME\
	program_invocation_name[1] == '/' ?\
	program_invocation_name + 2 :\
	program_invocation_name

/* Prints debug info if debugging is on */
#define PRINT_DEBUG()\
	if (debug)\
		puts(expr)

/* Passed by [type]-returning functions on failure */
#define INT_FAIL	INT_MAX
#define DBL_FAIL	DBL_MAX

/* Indicates valid syntax */
#define CHK_PASS	-1

#define OBS_R		1
#define OBS_L		2

/* Debug mode */
static bool debug = true;

/* Program name for use in error messages */
extern char *program_invocation_name;

/* Types of characters read */
static const char *valchrs = "+-!^*/%1234567890.()\n",
				  *opers = "+-!^*/%",
		  		  *single_opers = "^*/%",
		  		  *double_opers = "+-!";

/* Flags */
enum flag{F_PAR = 0,	/* Ignore parentheses (-p) */
		  F_SYN = 0,	/* Ignore syntax errors (-s) */
		  F_NAN = 0,	/* Return nonreal answers (-n) */
		  F_DEC = 6,	/* Round to number of decminal places (-d [int])*/
		  F_HLP = 0};	/* Outputs help page */

/* Returns string representation of double
 * Resulting string must be freed */
extern char *dtos(double _x, unsigned _sig);

/* Returns new, allocated substring spanning the given elements */
extern char *popsub(const char *_s, unsigned _low, unsigned _high);

/* Returns new, allocated string where substring replaces given elements in old string
 * Frees given string and substring while returning a newly allocated one */
extern char *pushsub(char *_s, char *_sub, unsigned _low, unsigned _high);

/* Evaluates mathemetical expression starting from given index position
 * Returns string representation of result depending on index
 * Returns NULL on failure
 * 
 * Flags:
 *	* F_PAR - Ignores parentheses 
 *	* F_SYN - Ignores syntax errors */
extern char *simplify(const char *_expr, unsigned _from);

/* Returns true if floating-point numbers are equal */
extern bool isequal(double _x, double _y);

/* Returns true if character is in string */
extern bool isin(const char _x, const char *_y);

/* Returns true if character is numerical (digit || '-' || '.') */
extern bool isnumer(char _c);

/* Returns number of whole places */
extern unsigned nplaces(double _x);

/* Returns index position of first invalid character of expression
 * Returns -1 if no invalid characters are found */
extern int chk_syntax(const char *_expr);

/* Returns index position of first invalid parenthesis of expression
 * Returns -1 if no invalid parentheses are found */
extern int chk_parenth(const char *_expr);

/* Get digit at given place */
extern int getdigit(double _x, int _place);

/* Returns left- or right-hand limit of range of operation at given position in expression
 * Returns OPER_MISS if invalid direction or operand is missing */
extern int getlim(char *_expr, unsigned _operpos, char _dir);

/* Returns OBS_R or OBS_L depending on type of obstruction(s)
 * If none are found, returns 0 */
extern unsigned isclr(const char *_expr, unsigned _operpos, unsigned _low, unsigned _high);

extern unsigned findob(const char *_expr, unsigned _operpos, unsigned _llim, unsigned _rlim);

/* Evaluates mathematical expression from index position 0
 * Ignores parentheses and syntax errors
 * Returns DBL_MAX on failure
 * 
 * Flags:
 * 	* F_NAN - Calculates nonreal answers */
extern double evaluate(const char *_expr);

/* Returns left- or right-hand value of the operand at given position in the expression */
extern double getval(char *_expr, unsigned _operpos, char _dir);

/* Returns double representation of string */
extern double stod(const char *_s);

#endif /* #ifdef _PARSE_H_ */