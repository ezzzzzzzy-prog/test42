#ifndef BUILTIN_H
#define BUILTIN_H

int is_builtin(const char *cmd);
int execute_builtin(char **argv);

#endif
