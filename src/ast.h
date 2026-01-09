#ifndef AST_H
#define AST_H

enum ast_type
{
	AST_IF,
	AST_LIST,
	AST_COMMAND,
};

struct ast
{
	enum ast_type type;
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


#endif /* AST_H */
