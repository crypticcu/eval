#ifndef PARSE_H
#define PARSE_H

/* Evaluates mathemetical expression
 * Returns string representation of result
 * On success, result must be freed
 * Returns NULL on failure */
extern char *parse(const char *expression, unsigned sig, void *null)
attribute(__warn_unused_result__);

/* Evaluates mathematical expression
 * Ignores parentheses and syntax errors
 * Returns NULL on failure */
extern char *parse_sub(char **expr_addr);

/* Evaluates given operation in mathematical expression
 * Ignores parentheses and syntax errors
 * Returns NULL on failure */
extern char *parse_oper(char **expr_addr, const char *oper);

#endif // #ifndef PARSE_H