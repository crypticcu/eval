#ifndef PARSE_H
#define PARSE_H

/* Evaluates mathemetical expression
 * Returns string representation of result
 * Returns NULL on failure */
extern char *parse(const char *expression, void *null);

/* Evaluates mathematical expression
 * Ignores parentheses and syntax errors
 * Returns NULL on failure */
extern char *parse_expr(char **expr_addr);

/* Evaluates given operation in mathematical expression
 * Ignores parentheses and syntax errors
 * Returns NULL on failure */
extern char *parse_oper(char **expr_addr, const char *oper);

#endif /* #ifndef PARSE_H */