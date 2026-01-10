#ifndef AST_H
#define AST_H
#include <stddef.h>
enum ast_type
{
    AST_COMMAND,
    AST_LIST,
    AST_IF,
    AST_PIPELINE
};

struct ast
{
	enum ast_type type;
};

struct ast_pipeline
{
    struct ast base;
    struct ast **cmds;
    size_t count;
};

struct ast_cmd
{
	struct ast base;
	char **words;
};

struct ast_list
{
	struct ast base;
	struct ast **commands;
	size_t count;
	char *sep;
};

struct ast_if
{
	struct ast base;
	struct ast *condition;
	struct ast *then_body;
	struct ast *else_body;
};

struct ast_negation
{
    struct ast base;
    struct ast *child;
};

struct ast *create_cmd(char **words);
struct ast *create_list(struct ast **cmds, size_t count);
struct ast *create_if(struct ast *cond,
                      struct ast *then_body,
                      struct ast *else_body);
struct ast *ast_pipeline_create(struct ast **cmds, size_t count);

void ast_free(struct ast *ast);

#endif /* AST_H */
