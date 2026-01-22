#ifndef AST_H
#define AST_H
#include <stddef.h>
enum ast_type
{
    AST_COMMAND,
    AST_LIST,
    AST_IF,
    AST_WHILE,
    AST_UNTIL,
    AST_FOR,
    AST_NEGATION,
    AST_AND,
    AST_OR,
    AST_PIPELINE,
    AST_REDIRECTION,
    AST_SUBSHELL
};
enum redir_type
{
    AST_REDIR_OUT,
    AST_REDIR_APP,
    AST_REDIR_DUP_OUT,
    AST_REDIR_DUP_IN,
    AST_REDIR_FORC_OUT,
    AST_REDIR_IN,
    AST_REDIR_RW
};

struct ast
{
	enum ast_type type;
};

struct ast_redirection
{
    struct ast base;
    enum redir_type type;
    struct ast *left;
    char *file;
    int redir_nb;
};
    
struct  ast_while
{
	struct ast base;
	struct ast *condition;
	struct ast *body;
};

struct ast_until
{
	struct ast base;
	struct ast *condition;
	struct ast *body;
};

struct ast_for
{
	struct ast base;
	char *var;
	char **words;
	struct ast *body;
};

struct ast_and_or
{
    struct ast base;
    struct ast *left;
    struct ast *right;
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
struct ast_subshell
{
    struct ast base;
    struct ast *body;
};

struct ast *create_cmd(char **words);
struct ast *create_list(struct ast **cmds, size_t count);
struct ast *create_if(struct ast *cond,
                      struct ast *then_body,
                      struct ast *else_body);
struct ast *ast_pipeline_create(struct ast **cmds, size_t count);
struct ast *create_negation(struct ast *child);
struct ast *create_and(struct ast *left, struct ast *right);
struct ast *create_or(struct ast *left, struct ast *right);
struct ast *create_redir(enum redir_type type, struct ast *left, char *file, int file_desc);
struct ast *create_while(struct ast *cond, struct ast *body);
struct ast *create_until(struct ast *cond, struct ast *body);
struct ast *create_for(char *var, char **words, struct ast *body);
struct ast *create_subshell(struct ast *body);
void ast_free(struct ast *ast);

#endif /* AST_H */
