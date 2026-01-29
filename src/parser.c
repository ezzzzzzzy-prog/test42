#define _POSIX_C_SOURCE 200809L
#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "expansion.h"
#include "lexer.h"

static struct ast *parse_command(struct parser *parser);

static void parser_consume(struct parser *parser)
{
    if (!parser)
    {
        return;
    }
    if (parser->curr_tok)
    {
        free_tok(parser->curr_tok);
        parser->curr_tok = NULL;
    }
    parser->curr_tok = pop(parser->lex);
}

static int is_list_end(enum type t)
{
    return t == TOK_THEN || t == TOK_ELSE || t == TOK_ELIF || t == TOK_FI
        || t == TOK_EOF  || t == TOK_DO || t == TOK_DONE || t == TOK_SUB_RB;
}

struct parser *new_parse(void)
{
    struct parser *p = malloc(sizeof(*p));
    if (!p)
        return NULL;

    p->lex = new_lex();
    p->curr_tok = pop(p->lex);
    p->var = NULL;
    p->func = NULL;
    p->spe = NULL;
    p->exit = 0;
    p->ex_code = 0;
    p->last_code = 0;
    p->parse_error = 0;
    return p;
}

void free_variable(struct variable *var)
{
    while (var)
    {
        struct variable *v = var->next;
        free(var->nom);
        free(var->value);
        free(var);
        var = v;
    }
}

void free_function(struct function *func)
{
        while(func)
        {
                struct function *f = func->next;
                free(func->name);
                ast_free(func->body);
                free(func);
                func = f;
        }
}
void parser_free(struct parser *parser)
{
    if (!parser)
        return;
    if (parser->curr_tok)
        free_tok(parser->curr_tok);
    lexer_free(parser->lex);
    free_variable(parser->var);
    free_function(parser->func);
    free(parser);
}

void add_var(struct parser *parser, const char *name, const char *value)
{
    if (!parser || !name)
        return;

    struct variable *v = parser->var;
    while (v)
    {
        if (strcmp(v->nom, name) == 0)
        {
            free(v->value);
            v->value = strdup(value);
            return;
        }
        v = v->next;
    }
    struct variable *var = malloc(sizeof(*var));
    if (!var)
        return;

    var->nom = strdup(name);
    var->value = strdup(value);
    var->next = parser->var;
    parser->var = var;
}


void add_function(struct parser *parser, char *name, struct ast *body)
{
        if(!parser || !name || !body)
        {
                return;
        }
        struct function *f = parser->func;
        while(f)
        {
                if(strcmp(f->name, name) == 0)
                {
                        ast_free(f->body);
                        f->body = body;
                        return;
                }
                f = f->next;
        }
        struct function *new_func = malloc(sizeof(*new_func));
        if(!new_func)
        {
                return;
        }
        new_func->name = strdup(name);
        new_func->body = body;
        new_func->next = parser->func;
        parser->func = new_func;
}

struct function *get_function(struct parser *parser, const char *name)
{
        if(!parser || !name)
        {
                return NULL;
        }
        struct function *f = parser->func;
        while(f)
        {
                if(strcmp(f->name,name) == 0)
                {
                        return f;
                }
                f = f->next;
        }
        return NULL;
}

static int parse_assignment(struct parser *parser)
{
    if (!parser->curr_tok || parser->curr_tok->type != TOK_WORD)
    {
        return 0;
    }
    char *word = parser->curr_tok->val;
    if (!word)
    {
        return 0;
    }
    char *cpy = strdup(word);
    if (!cpy)
    {
        return 0;
    }
    char *cherche = strchr(cpy, '=');
    if (!cherche)
    {
        free(cpy);
        return 0;
    }
    *cherche = '\0';
    char *name = cpy;
    char *value = cherche + 1;
    add_var(parser, name, value);
    free(cpy);
    parser_consume(parser);
    return 1;
}
static bool is_redirection(enum type t)
{
    if (t == TOK_REDIR_OUT)
        return true;
    if (t == TOK_REDIR_APP)
        return true;
    if (t == TOK_REDIR_DUP_OUT)
        return true;
    if (t == TOK_REDIR_DUP_IN)
        return true;
    if (t == TOK_REDIR_FORC_OUT)
        return true;
    if (t == TOK_REDIR_IN)
        return true;
    if (t == TOK_REDIR_RW)
        return true;
    if (t == TOK_REDIR_NB)
        return true;
    return false;
}

struct ast *parse_redir(struct parser *parser, struct ast *left_cmd)
{
    if (!parser || !parser->curr_tok || !is_redirection(parser->curr_tok->type))
        return NULL;
    int redir_nb = -1;
    if (parser->curr_tok->type == TOK_REDIR_NB)
    {
        redir_nb = atoi(parser->curr_tok->val);
        parser_consume(parser);
        if (!parser->curr_tok || !is_redirection(parser->curr_tok->type))
            return NULL;
    }
    enum type curr_type = parser->curr_tok->type;
    enum redir_type curr_redir_type;

    if (curr_type == TOK_REDIR_OUT)
        curr_redir_type = AST_REDIR_OUT;
    else if (curr_type == TOK_REDIR_IN)
        curr_redir_type = AST_REDIR_IN;
    else if (curr_type == TOK_REDIR_APP)
        curr_redir_type = AST_REDIR_APP;
    else if (curr_type == TOK_REDIR_DUP_OUT)
        curr_redir_type = AST_REDIR_DUP_OUT;
    else if (curr_type == TOK_REDIR_DUP_IN)
        curr_redir_type = AST_REDIR_DUP_IN;
    else if (curr_type == TOK_REDIR_FORC_OUT)
        curr_redir_type = AST_REDIR_FORC_OUT;
    else if (curr_type == TOK_REDIR_RW)
        curr_redir_type = AST_REDIR_RW;
    else
        return NULL;
    parser_consume(parser);
    if (!parser->curr_tok || parser->curr_tok->type != TOK_WORD)
        return NULL;
    char *f = strdup(parser->curr_tok->val);
    if (!f)
        return NULL;

    parser_consume(parser);

    struct ast *r = create_redir(curr_redir_type, left_cmd, f, redir_nb);
    if (!r)
    {
        free(f);
        return NULL;
    }
    return r;
}

static struct ast *app_redir(struct ast *root, struct ast *new_redir)
{
    if (!root)
        return new_redir;
    struct ast_redirection *curr = (struct ast_redirection *)root;
    while (curr->left)
        curr = (struct ast_redirection *)curr->left;
    curr->left = new_redir;
    return root;
}

static void free_words(char **words, size_t count)
{
    if (!words)
        return;
    for (size_t i = 0; i < count; i++)
        free(words[i]);
    free(words);
}

static int add_words(struct parser *parser, char ***words, size_t *count,
                     size_t *cap)
{
    if (*count + 1 >= *cap)
    {
        *cap *= 2;
        char **tmp = realloc(*words, sizeof(char *) * (*cap));
        if (!tmp)
            return 0;
        *words = tmp;
    }
    (*words)[*count] = strdup(parser->curr_tok->val);
    if (!(*words)[*count])
        return 0;

    (*count)++;
    parser_consume(parser);
    return 1;
}

struct ast *parse_simple_command(struct parser *parser)
{
    if (!parser || !parser->curr_tok)
        return NULL;
    if (parse_assignment(parser))
    {
        return create_cmd(NULL);
    }
    if (parser->curr_tok->type != TOK_WORD)
    {
        return NULL;
    }
    struct ast *res = NULL;
    size_t cap = 4;
    size_t count = 0;
    char **words = malloc(sizeof(char *) * cap);
    if (!words)
        return NULL;
    while (parser->curr_tok
           && (parser->curr_tok->type == TOK_WORD
               || is_redirection(parser->curr_tok->type)))
    {
        if (is_redirection(parser->curr_tok->type))
        {
            struct ast *new_res = parse_redir(parser, NULL);
            if (!new_res)
                goto error;
            res = app_redir(res, new_res);
        }
        else
        {
            if (!add_words(parser, &words, &count, &cap))
                goto error;
        }
    }

    if (count == 0 && res == NULL)
    {
        free(words);
        return NULL;
    }
    words[count] = NULL;
    struct ast *cmd = create_cmd(words);
    if (res != NULL)
    {
        struct ast_redirection *tmp = (struct ast_redirection *)res;
        while (tmp->left != NULL)
            tmp = (struct ast_redirection *)tmp->left;
        tmp->left = cmd;
        return res;
    }
    return cmd;

error:
    free_words(words, count);
    ast_free(res);
    return NULL;
}

static int parse_pipeline_neg(struct parser *parser)
{
    if (parser->curr_tok && parser->curr_tok->type == TOK_NOT)
    {
        parser_consume(parser);
        return 1;
    }
    return 0;
}

static struct ast **init_pipeline_cmds(size_t *capacity, size_t *count)
{
    *capacity = 4;
    *count = 0;
    struct ast **cmds = malloc(sizeof(*cmds) * (*capacity));
    if (!cmds)
    {
        return NULL;
    }
    return cmds;
}

/*static int parse_pipeline_cmds(struct parser *parser, struct ast ***cmds,
                               size_t *count, size_t *capacity)
{
    struct ast *cmd = parse_command(parser);
    if (!cmd)
    {
        return 0;
    }
    (*cmds)[(*count)++] = cmd;
    while (parser->curr_tok && parser->curr_tok->type == TOK_PIPE)
    {
        parser_consume(parser);
        cmd = parse_command(parser);
        if (!cmd)
        {
            return 0;
        }
        if (*count == *capacity)
        {
            *capacity *= 2;
            struct ast **tmp = realloc(*cmds, sizeof(**cmds) * (*capacity));
            if (!tmp)
            {
                return 0;
            }
            *cmds = tmp;
        }
        (*cmds)[(*count)++] = cmd;
    }
    return 1;
}*/


static int parse_pipeline_cmds(struct parser *parser, struct ast ***cmds,
                               size_t *count, size_t *capacity)
{
    struct ast *cmd = parse_command(parser);
    if (!cmd)
    {
        return 0;
    }
    (*cmds)[(*count)++] = cmd;
    while (parser->curr_tok && parser->curr_tok->type == TOK_PIPE)
    {
        parser_consume(parser);
        while(parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
        {
                parser_consume(parser);
        }
        if(!parser->curr_tok || parser->curr_tok->type == TOK_EOF || parser->curr_tok->type == TOK_PIPE || parser->curr_tok->type == TOK_SEMI || parser->curr_tok->type == TOK_AND || parser->curr_tok->type == TOK_OR)
        {
                fprintf(stderr, "parse error \n");
                return 0;
        }
        cmd = parse_command(parser);
        if (!cmd)
        {
                fprintf(stderr, "parse error\n");
            return 0;
        }
        if (*count == *capacity)
        {
            *capacity *= 2;
            struct ast **tmp = realloc(*cmds, sizeof(**cmds) * (*capacity));
            if (!tmp)
            {
                return 0;
            }
            *cmds = tmp;
        }
        (*cmds)[(*count)++] = cmd;
    }
    return 1;
}
static struct ast *build_pipeline(struct ast **cmds, size_t count, int neg)
{
    struct ast *result;
    if (count == 1)
    {
        result = cmds[0];
        free(cmds);
    }
    else
    {
        result = ast_pipeline_create(cmds, count);
    }
    if (neg)
    {
        result = create_negation(result);
    }
    return result;
}

struct ast *parse_pipeline(struct parser *parser)
{
        if(parser->curr_tok && parser->curr_tok->type == TOK_PIPE)
        {
                fprintf(stderr, "parse error\n");
                parser->parse_error = 1;
                return NULL;
        }
    int neg = parse_pipeline_neg(parser);
    size_t capacity;
    size_t count;
    struct ast **cmds = init_pipeline_cmds(&capacity, &count);
    if (!cmds)
    {
        return NULL;
    }
    if (!parse_pipeline_cmds(parser, &cmds, &count, &capacity))
    {
        goto error;
    }
    return build_pipeline(cmds, count, neg);
error:
    for (size_t i = 0; i < count; i++)
        ast_free(cmds[i]);
    free(cmds);
    return NULL;
}

struct ast *parse_and_or(struct parser *parser)
{
    struct ast *left = parse_pipeline(parser);
    if (!left)
        return NULL;

    while (parser->curr_tok
           && (parser->curr_tok->type == TOK_AND
               || parser->curr_tok->type == TOK_OR))
    {
        enum type op = parser->curr_tok->type;

        parser_consume(parser);
        while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
        {
            parser_consume(parser);
        }

        struct ast *right = parse_pipeline(parser);
        if (!right)
        {
            ast_free(left);
            return NULL;
        }

        if (op == TOK_AND)
            left = create_and(left, right);
        else
            left = create_or(left, right);
    }

    return left;
}

static int add_ast(struct ast ***cmds, size_t *count, size_t *cap,
                   struct ast *next)
{
    if (*count >= *cap)
    {
        *cap *= 2;
        struct ast **tmp = realloc(*cmds, sizeof(**cmds) * (*cap));
        if (!tmp)
            return 0;
        *cmds = tmp;
    }
    (*cmds)[(*count)++] = next;
    return 1;
}

static void skip_newlines(struct parser *parser)
{
    while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
    {
        parser_consume(parser);
    }
}

static struct ast **init_cmds(struct ast *first, size_t *cap, size_t *count)
{
    *cap = 4;
    *count = 1;
    struct ast **cmds = malloc(sizeof(*cmds) * (*cap));
    if (!cmds)
    {
        return NULL;
    }
    cmds[0] = first;
    return cmds;
}

/*static int parse_compound_sql(struct parser *parser, struct ast ***cmds,
                              size_t *count, size_t *cap)
{
    while (parser->curr_tok
           && (parser->curr_tok->type == TOK_SEMI
               || parser->curr_tok->type == TOK_NEWLINE))
    {
        parser_consume(parser);
        while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
        {
            parser_consume(parser);
        }
        if (!parser->curr_tok || is_list_end(parser->curr_tok->type))
        {
            break;
        }
        struct ast *next = parse_and_or(parser);
        if (!next)
        {
            break;
        }
        if (!add_ast(cmds, count, cap, next))
        {
            return 0;
        }
    }
    return 1;
}*/

static int parse_compound_sql(struct parser *parser, struct ast ***cmds,
                              size_t *count, size_t *cap)
{
    while (parser->curr_tok
           && (parser->curr_tok->type == TOK_SEMI
               || parser->curr_tok->type == TOK_NEWLINE))
    {
        parser_consume(parser);
        while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
        {
            parser_consume(parser);
        }
        if (!parser->curr_tok || parser->curr_tok->type == TOK_EOF)
        {
            break;
        }
        struct ast *next = parse_command(parser);
        if (!next)
        {
            break;
        }
        if (!add_ast(cmds, count, cap, next))
        {
            return 0;
        }
    }
    return 1;
}

struct ast *parse_compound_list(struct parser *parser)
{
    skip_newlines(parser);
    if (!parser->curr_tok || parser->curr_tok->type == TOK_EOF)
    {
        return create_cmd(NULL);
    }
    if (is_list_end(parser->curr_tok->type))
    {
        return create_cmd(NULL);
    }
    struct ast *first = parse_and_or(parser);
    if (!first)
    {
        if (!parser->curr_tok || is_list_end(parser->curr_tok->type))
        {
            return create_cmd(NULL);
        }
        return NULL;
    }
    size_t cap;
    size_t count;
    struct ast **cmds = init_cmds(first, &cap, &count);
    if (!cmds)
    {
        ast_free(first);
        return NULL;
    }
    if (!parse_compound_sql(parser, &cmds, &count, &cap))
    {
        goto error;
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
    if (!parser || !parser->curr_tok)
    {
        return NULL;
    }
    if (parser->curr_tok->type == TOK_ELIF)
    {
        parser_consume(parser);
        if (!parser->curr_tok || parser->curr_tok->type == TOK_THEN)
        {
            fprintf(stderr, "parse error: condition in elif\n");
            parser->parse_error = 1;
            return NULL;
        }
        struct ast *elif_cond = parse_compound_list(parser);
        if (!elif_cond)
        {
            fprintf(stderr, "parse error: condition in elif\n");
            parser->parse_error = 1;
            return NULL;
        }
        if (!parser->curr_tok || parser->curr_tok->type != TOK_THEN)
        {
            ast_free(elif_cond);
            fprintf(stderr, "parse error: then in elif\n");
            parser->parse_error = 1;
            return NULL;
        }
        parser_consume(parser);
        struct ast *elif_body = parse_compound_list(parser);
        if (!elif_body)
        {
            elif_body = create_cmd(NULL);
        }
        struct ast *next_else = parse_else(parser);
        return create_if(elif_cond, elif_body, next_else);
    }
    if (parser->curr_tok->type == TOK_ELSE)
    {
        parser_consume(parser);
        struct ast *else_body = parse_compound_list(parser);
        if (!else_body)
        {
            else_body = create_cmd(NULL);
        }
        return else_body;
    }
    return NULL;
}

struct ast *parse_rule_if(struct parser *parser)
{
    if (!parser || !parser->curr_tok || parser->curr_tok->type != TOK_IF)
    {
        return NULL;
    }
    parser_consume(parser);
    while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
    {
        parser_consume(parser);
    }
    if (!parser->curr_tok || parser->curr_tok->type == TOK_THEN)
    {
        fprintf(stderr, "parse error: condition in if\n");
        parser->parse_error = 1;
        return NULL;
    }
    struct ast *condition = parse_compound_list(parser);
    if (!condition)
    {
        fprintf(stderr, "parse error: condition in if\n");
        parser->parse_error = 1;
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_THEN)
    {
        ast_free(condition);
        fprintf(stderr, "parse error: then in if\n");
        parser->parse_error = 1;
        return NULL;
    }
    parser_consume(parser);
    struct ast *then_body = parse_compound_list(parser);
    if (!then_body)
    {
        then_body = create_cmd(NULL);
    }
    struct ast *else_body = parse_else(parser);
    if (!parser->curr_tok || parser->curr_tok->type != TOK_FI)
    {
        ast_free(condition);
        ast_free(then_body);
        ast_free(else_body);
        fprintf(stderr, "parse error :fi in if\n");
        parser->parse_error = 1;
        return NULL;
    }
    parser_consume(parser);
    return create_if(condition, then_body, else_body);
}

struct ast *parser_input(struct parser *parser)
{
    if (!parser)
    {
        return NULL;
    }
    while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
    {
        parser_consume(parser);
    }
    if (!parser->curr_tok || parser->curr_tok->type == TOK_EOF)
    {
        return NULL;
    }
    return parse_compound_list(parser);
}

struct ast *parse_rule_while(struct parser *parser)
{
    if (!parser || !parser->curr_tok || parser->curr_tok->type != TOK_WHILE)
    {
        return NULL;
    }
    parser_consume(parser);
    struct ast *condition = parse_compound_list(parser);
    if (!condition)
    {
         fprintf(stderr, "parse error: condition in while\n");
        parser->parse_error = 1;
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DO)
    {
        ast_free(condition);
        fprintf(stderr, "parse error: do in while\n");
        parser->parse_error = 1;
        return NULL;
    }
    parser_consume(parser);
    struct ast *body = parse_compound_list(parser);
    if (!body)
    {
        ast_free(condition);
        fprintf(stderr, "parse error: body in while\n");
        parser->parse_error = 1;
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DONE)
    {
        ast_free(condition);
        ast_free(body);
        fprintf(stderr, "parse error: done in while\n");
        parser->parse_error = 1;
        return NULL;
    }
    parser_consume(parser);
    return create_while(condition, body);
}


struct ast *parse_rule_until(struct parser *parser)
{
    if (!parser || !parser->curr_tok || parser->curr_tok->type != TOK_UNTIL)
    {
        return NULL;
    }
    parser_consume(parser);
    struct ast *condition = parse_compound_list(parser);
    if (!condition)
    {
            fprintf(stderr, "parse error: condition in until\n");
        parser->parse_error = 1;
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DO)
    {
        ast_free(condition);
         fprintf(stderr, "parse error: do in until\n");
        parser->parse_error = 1;
        return NULL;
    }
    parser_consume(parser);
    struct ast *body = parse_compound_list(parser);
    if (!body)
    {
        ast_free(condition);
        fprintf(stderr, "parse error: body in until\n");
        parser->parse_error = 1;
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DONE)
    {
        ast_free(condition);
        ast_free(body);
        fprintf(stderr, "parse error: done in until\n");
        parser->parse_error = 1;
        return NULL;
    }
    parser_consume(parser);
    return create_until(condition, body);
}
static void free_for_parts(char *var, char **words)
{
    if (var)
    {
        free(var);
    }
    if (words)
    {
        for (size_t i = 0; words[i]; i++)
        {
            free(words[i]);
        }
        free(words);
    }
}

static char *parse_for_var(struct parser *parser)
{
    if (!parser->curr_tok || parser->curr_tok->type != TOK_WORD)
    {
        return NULL;
    }
    char *var = strdup(parser->curr_tok->val);
    parser_consume(parser);
    return var;
}


static char **parse_for_words(struct parser *parser)
{
    if (!parser->curr_tok || parser->curr_tok->type != TOK_IN)
    {
        return NULL;
    }
    parser_consume(parser);
    size_t cap = 8;
    size_t count = 0;
    char **words = malloc(sizeof(char *) * cap);
    if (!words)
    {
        return NULL;
    }
    while (parser->curr_tok && parser->curr_tok->type == TOK_WORD)
    {
        char *expanded = expand(parser, parser->spe, parser->curr_tok->val);
        if (!expanded)
        {
            expanded = strdup(parser->curr_tok->val);
            if (!expanded)
            {
                for (size_t i = 0; i < count; i++)
		{
                    free(words[i]);
		}
                free(words);
                return NULL;
            }
        }
        if (count + 1 >= cap)
        {
            cap *= 2;
            char **tmp = realloc(words, sizeof(char *) * cap);
            if (!tmp)
            {
                free(expanded);
                for (size_t i = 0; i < count; i++)
		{
                    free(words[i]);
		}
                free(words);
                return NULL;
            }
            words = tmp;
        }
        words[count++] = expanded;
        parser_consume(parser);
    }
    words[count] = NULL;
    return words;
}

static int skip_semi_newline(struct parser *parser)
{
    if (!parser->curr_tok
        || (parser->curr_tok->type != TOK_SEMI
            && parser->curr_tok->type != TOK_NEWLINE))
    {
        return 0;
    }
    while (parser->curr_tok
           && (parser->curr_tok->type == TOK_SEMI
               || parser->curr_tok->type == TOK_NEWLINE))
    {
        parser_consume(parser);
    }
    return 1;
}

static struct ast *parse_for_body(struct parser *parser)
{
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DO)
    {
        return NULL;
    }
    parser_consume(parser);
    struct ast *body = parse_compound_list(parser);
    if (!body || !parser->curr_tok || parser->curr_tok->type != TOK_DONE)
    {
        ast_free(body);
        return NULL;
    }
    parser_consume(parser);
    return body;
}

struct ast *parse_rule_for(struct parser *parser)
{
    parser_consume(parser);
    char *var = parse_for_var(parser);
    if (!var)
    {
            fprintf(stderr, "parse error near 'for'\n");
        parser->parse_error = 1;
        return NULL;
    }
    char **words = NULL;
    if (parser->curr_tok && parser->curr_tok->type == TOK_IN)
    {
        words = parse_for_words(parser);
        if(!words)
        {
                free(var);
            fprintf(stderr, "parse error in for loop\n");
            parser->parse_error = 1;
            return NULL;
        }
    }
    if (!skip_semi_newline(parser))
    {
        free_for_parts(var, words);
         fprintf(stderr, "parse error:newline in for\n");
        parser->parse_error = 1;
        return NULL;
    }
    struct ast *body = parse_for_body(parser);
    if (!body)
    {
        free_for_parts(var, words);
        fprintf(stderr, "parse error: do/done in for\n");
        parser->parse_error = 1;
        return NULL;
    }
    return create_for(var, words, body);
}

static struct ast *parse_paren(struct parser *parser)
{
    struct ast *body;
    // pass the (
    parser_consume(parser);

    body = parse_compound_list(parser);
    if (!body)
        return NULL;

    if (!parser->curr_tok || parser->curr_tok->type != TOK_SUB_RP)
    {
        ast_free(body);
        return NULL;
    }
    // pass the )
    parser_consume(parser);
    return create_subshell(body);
}

static struct ast *parse_brace(struct parser *parser)
{
    struct ast *body;
    // pass {
    parser_consume(parser);
    body = parse_compound_list(parser);
    if (!body)
        return NULL;
    // look for end
    if (!parser->curr_tok || parser->curr_tok->type != TOK_SUB_RB)
    {
        ast_free(body);
        return NULL;
    }

    // pass }
    parser_consume(parser);
    return create_subshell(body);
}

static struct ast *parse_subshell(struct parser *parser)
{
    if (!parser || !parser->curr_tok)
        return NULL;

    if (parser->curr_tok->type == TOK_SUB_LP)
        return parse_paren(parser);

    if (parser->curr_tok->type == TOK_SUB_LB)
        return parse_brace(parser);

    return NULL;
}
static struct ast *parse_choose(struct parser *parser)
{
    if (parser->curr_tok->type == TOK_SUB_LP
        || parser->curr_tok->type == TOK_SUB_LB)
        return parse_subshell(parser);

    if (parser->curr_tok->type == TOK_IF)
        return parse_rule_if(parser);

    if (parser->curr_tok->type == TOK_WHILE)
        return parse_rule_while(parser);

    if (parser->curr_tok->type == TOK_UNTIL)
        return parse_rule_until(parser);

    if (parser->curr_tok->type == TOK_FOR)
        return parse_rule_for(parser);

    return NULL;
}
static struct ast *parse_br_ct(struct parser *parser)
{
    if (parser->curr_tok->type == TOK_BREAK)
    {
        parser_consume(parser);
        return create_break();
    }
    if (parser->curr_tok->type == TOK_CONTINUE)
    {
        parser_consume(parser);
        return create_continue();
    }
    return NULL;
}
static struct ast *parse_function(struct parser *parser)
{
    if (!parser || !parser->curr_tok || parser->curr_tok->type != TOK_WORD)
        return NULL;

    struct token *next = peek(parser->lex);
    if (!next || next->type != TOK_SUB_LP)
        return NULL;
    char *name = strdup(parser->curr_tok->val);
    parser_consume(parser);
    parser_consume(parser);
    if (!parser->curr_tok || parser->curr_tok->type != TOK_SUB_RP)
    {
        free(name);
        fprintf(stderr, "parse error: ')' in function\n");
        parser->parse_error = 1;
        return NULL;
    }
    parser_consume(parser);
    while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
    {
        parser_consume(parser);
    }
    struct ast *body = NULL;
    if (parser->curr_tok && (parser->curr_tok->type == TOK_SUB_LP || parser->curr_tok->type == TOK_SUB_LB))
    {
        body = parse_subshell(parser);
    }
    else
    {
        body = parse_choose(parser);
    }

    if (!body)
    {
        free(name);
        fprintf(stderr, "parse error:body in function\n");
        parser->parse_error = 1;
        return NULL;
    }
    return create_function(name, body);
}
static struct ast *parse_simple_with_redir(struct parser *parser)
{
    struct ast *cmd = parse_simple_command(parser);
    if (!cmd)
        return NULL;

    while (parser->curr_tok && is_redirection(parser->curr_tok->type))
    {
        struct ast *redir = parse_redir(parser, cmd);
        if (!redir)
        {
            ast_free(cmd);
            return NULL;
        }
        cmd = redir;
    }
    return cmd;
}
static struct ast *parse_command(struct parser *parser)
{
    if (!parser || !parser->curr_tok)
        return NULL;

    struct ast *res = parse_choose(parser);
    if (res)
    {
        return res;
    }
    res = parse_br_ct(parser);
    if (res)
        return res;
    res = parse_function(parser);
    if(res)
    {
            return res;
    }
    return parse_simple_with_redir(parser);
}
