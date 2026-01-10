#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include <stdlib.h>
#include <string.h>




static struct ast *parse_command(struct parser *parser);
/*{
    if (!parser || !parser->curr_tok)
        return NULL;
//to implement
    if (parser->curr_tok->type == TOK_IF)
        return parse_rule_if(parser);

    return parse_simple_command(parser);
}*/


static int is_list_end(enum type t)
{
    return t == TOK_THEN || t == TOK_ELSE || t == TOK_ELIF ||
           t == TOK_FI || t == TOK_EOF;
}

struct parser *new_parse(void)
{
    struct parser *p = malloc(sizeof(*p));
    if (!p)
        return NULL;

    p->lex = new_lex();
    p->curr_tok = peek(p->lex);
    return p;
}

void parser_free(struct parser *parser)
{
    if (!parser)
        return;
    lexer_free(parser->lex);
    free(parser);
}

struct ast *parse_simple_command(struct parser *parser)
{
    if (!parser || !parser->curr_tok ||
        parser->curr_tok->type != TOK_WORD)
        return NULL;

    size_t cap = 4;
    size_t count = 0;
    char **words = malloc(sizeof(char *) * cap);
    if (!words)
        return NULL;

    while (parser->curr_tok && parser->curr_tok->type == TOK_WORD)
    {
        if (count + 1 >= cap)
        {
            cap *= 2;
            char **tmp = realloc(words, sizeof(char *) * cap);
            if (!tmp)
                goto error;
            words = tmp;
        }
        words[count++] = strdup(parser->curr_tok->val);
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
    }

    words[count] = NULL;
    return create_cmd(words);

error:
    for (size_t i = 0; i < count; i++)
        free(words[i]);
    free(words);
    return NULL;
}


struct ast *parse_pipeline(struct parser *parser)
{
    size_t capacity = 4;
    size_t count = 0;
    struct ast **cmds = malloc(sizeof(*cmds) * capacity);
    if (!cmds)
        return NULL;

    struct ast *cmd = parse_command(parser);
    if (!cmd)
        goto error;

    cmds[count++] = cmd;

    while (parser->curr_tok && parser->curr_tok->type == TOK_PIPE)
    {
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);

        cmd = parse_command(parser);
        if (!cmd)
            goto error;

        if (count == capacity)
        {
            capacity *= 2;
            struct ast **tmp = realloc(cmds, sizeof(*cmds) * capacity);
            if (!tmp)
                goto error;
            cmds = tmp;
        }

        cmds[count++] = cmd;
    }

    if (count == 1)
    {
        struct ast *o = cmds[0];
        free(cmds);
        return o;
    }

    return ast_pipeline_create(cmds, count);

error:
    for (size_t i = 0; i < count; i++)
        ast_free(cmds[i]);
    free(cmds);
    return NULL;
}


struct ast *parse_and_or(struct parser *parser)
{
    return parse_pipeline(parser);
}

struct ast *parse_compound_list(struct parser *parser)
{
    while (parser->curr_tok &&
           parser->curr_tok->type == TOK_NEWLINE)
    {
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
    }

    struct ast *first = parse_and_or(parser);
    if (!first)
        return NULL;

    size_t cap = 4;
    size_t count = 1;
    struct ast **cmds = malloc(sizeof(*cmds) * cap);
    if (!cmds)
    {
        ast_free(first);
        return NULL;
    }
    cmds[0] = first;

    while (parser->curr_tok &&
           !is_list_end(parser->curr_tok->type) &&
           (parser->curr_tok->type == TOK_SEMI ||
            parser->curr_tok->type == TOK_NEWLINE))
    {
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);

        while (parser->curr_tok &&
               parser->curr_tok->type == TOK_NEWLINE)
        {
            pop(parser->lex);
            parser->curr_tok = peek(parser->lex);
        }
        if (parser->curr_tok && is_list_end(parser->curr_tok->type))
            break;

        struct ast *next = parse_and_or(parser);
        if (!next)
            break;

        if (count >= cap)
        {
            cap *= 2;
            struct ast **tmp = realloc(cmds, sizeof(*cmds) * cap);
            if (!tmp)
                goto error;
            cmds = tmp;
        }
        cmds[count++] = next;
    }

    if (count == 1)
    {
        free(cmds);
        return first;
    }
    return create_list(cmds, count);

error:
    for (size_t i = 0; i < count; i++)
        ast_free(cmds[i]);
    free(cmds);
    return NULL;
}


static struct ast *parse_else(struct parser *parser)
{
        if(!parser || !parser->curr_tok)
        {
                return NULL;
        }
        if(parser->curr_tok->type == TOK_ELIF)
        {
                pop(parser->lex);
                parser->curr_tok = peek(parser->lex);
                struct ast *elif_cond = parse_compound_list(parser);
                if(!elif_cond)
                {
                        return NULL;
                }
                if(!parser->curr_tok || parser->curr_tok->type != TOK_THEN)
                {
                        ast_free(elif_cond);
                        return NULL;
                }
                pop(parser->lex);
                parser->curr_tok= peek(parser->lex);
                struct ast *elif_body = parse_compound_list(parser);
                if(!elif_body)
                {
                        ast_free(elif_body);
                        return NULL;
                }
                struct ast *next_else = parse_else(parser);
                return create_if(elif_cond, elif_body, next_else);
        }
        if(parser->curr_tok->type == TOK_ELSE)
        {
                pop(parser->lex);
                parser->curr_tok = peek(parser->lex);
                return parse_compound_list(parser);
        }
        return NULL;
}

struct ast *parse_rule_if(struct parser *parser)
{
        if(!parser || !parser->curr_tok || parser->curr_tok->type != TOK_IF)
        {
                return NULL;
        }
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
        struct ast *condition = parse_compound_list(parser);
        if(!condition)
        {
                return NULL;
        }
        if(!parser->curr_tok || parser->curr_tok->type != TOK_THEN)
        {
                ast_free(condition);
                return NULL;
        }
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
        struct ast *then_body = parse_compound_list(parser);
        if(!then_body)
        {
                ast_free(condition);
                return NULL;
        }
        struct ast *else_body = parse_else(parser);
        if(!parser->curr_tok || parser->curr_tok->type != TOK_FI)
        {
                ast_free(condition);
                ast_free(then_body);
                ast_free(else_body);
                return NULL;
        }
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
        return create_if(condition, then_body, else_body);
}
static struct ast *parse_command(struct parser *parser)
{
    if (!parser || !parser->curr_tok)
        return NULL;
//to implement
    if (parser->curr_tok->type == TOK_IF)
        return parse_rule_if(parser);

    return parse_simple_command(parser);
}

/*
struct ast *parse_pipeline(struct parser *parser)
{
    size_t capacity = 4;
    size_t count = 0;
    struct ast **cmds = malloc(sizeof(*cmds) * capacity);
    if (!cmds)
        return NULL;

    struct ast *cmd = parse_command(parser);
    if (!cmd)
        goto error;

    cmds[count++] = cmd;

    while (parser->curr_tok && parser->curr_tok->type == TOK_PIPE)
    {
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);

        cmd = parse_command(parser);
        if (!cmd)
            goto error;

        if (count == capacity)
        {
            capacity *= 2;
            struct ast **tmp = realloc(cmds, sizeof(*cmds) * capacity);
            if (!tmp)
                goto error;
            cmds = tmp;
        }

        cmds[count++] = cmd;
    }

    if (count == 1)
    {
        struct ast *o = cmds[0];
        free(cmds);
        return o;
    }

    return ast_pipeline_create(cmds, count);

error:
    for (size_t i = 0; i < count; i++)
        ast_free(cmds[i]);
    free(cmds);
    return NULL;
}


struct ast *parse_and_or(struct parser *parser)
{
    return parse_pipeline(parser);
}

struct ast *parse_compound_list(struct parser *parser)
{
    while (parser->curr_tok &&
           parser->curr_tok->type == TOK_NEWLINE)
    {
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
    }

    struct ast *first = parse_and_or(parser);
    if (!first)
        return NULL;

    size_t cap = 4;
    size_t count = 1;
    struct ast **cmds = malloc(sizeof(*cmds) * cap);
    if (!cmds)
    {
        ast_free(first);
        return NULL;
    }
    cmds[0] = first;

    while (parser->curr_tok &&
           !is_list_end(parser->curr_tok->type) &&
           (parser->curr_tok->type == TOK_SEMI ||
            parser->curr_tok->type == TOK_NEWLINE))
    {
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);

        while (parser->curr_tok &&
               parser->curr_tok->type == TOK_NEWLINE)
        {
            pop(parser->lex);
            parser->curr_tok = peek(parser->lex);
        }
        if (parser->curr_tok && is_list_end(parser->curr_tok->type))
            break;

        struct ast *next = parse_and_or(parser);
        if (!next)
            break;

        if (count >= cap)
        {
            cap *= 2;
            struct ast **tmp = realloc(cmds, sizeof(*cmds) * cap);
            if (!tmp)
                goto error;
            cmds = tmp;
        }
        cmds[count++] = next;
    }

    if (count == 1)
    {
        free(cmds);
        return first;
    }
    return create_list(cmds, count);

error:
    for (size_t i = 0; i < count; i++)
        ast_free(cmds[i]);
    free(cmds);
    return NULL;
}
*/
struct ast *parser_input(struct parser *parser)
{
    if (!parser)
        return NULL;
    if (!parser->curr_tok || parser->curr_tok->type == TOK_EOF)
        return create_list(NULL, 0);
    return parse_compound_list(parser);
}


