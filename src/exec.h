#ifndef EXEC_H
#define EXEC_H

#include "ast.h"

int exec_ast(struct ast *ast);

#endif /* EXEC_H */

/*#ifndef EXEC_H
#define EXEC_H

int execute_command(char **argv);

int execute_pipeline(void);
int execute_redirect(void);
*/

