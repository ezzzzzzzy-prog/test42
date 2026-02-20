#define _POSIX_C_SOURCE 200809L
#include "../parser/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ast/ast.h"
#include "../lexer/lexer.h"

static struct ast *parse_command(struct parser *parser);

static void parser_consume(struct parser *parser)
{
    if (!parser)
        return;
    if (parser->curr_tok)
    {
        free_tok(parser->curr_tok);
        parser->curr_tok = NULL;
    }
    parser->curr_tok = pop(parser->lex);
}

struct parser *new_parse(void)
{
    struct parser *p = malloc(sizeof(*p));
    if (!p)
        return NULL;
    p->lex = new_lex();
    p->curr_tok = pop(p->lex);
    p->var = NULL;
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

void parser_free(struct parser *parser)
{
    if (!parser)
        return;
    if (parser->curr_tok)
        free_tok(parser->curr_tok);
    lexer_free(parser->lex);
    free_variable(parser->var);
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
            setenv(name, value, 1);
            return;
        }
        v = v->next;
    }
    struct variable *var = malloc(sizeof(*var));
    if (!var)
        return;
    var->nom = strdup(name);
    var->value = strdup(value);
    var->exported = 0;
    var->next = parser->var;
    parser->var = var;
    setenv(name, value, 1);
}

static int is_redirection(enum type t)
{
    return t == TOK_REDIR_OUT || t == TOK_REDIR_APP || t == TOK_REDIR_IN
        || t == TOK_REDIR_NB;
}

/* Convertit le type token en type redirection */
static int tok_to_redir(enum type t, enum redir_type *rt)
{
    if (t == TOK_REDIR_OUT)
    {
        *rt = AST_REDIR_OUT;
        return 1;
    }
    if (t == TOK_REDIR_IN)
    {
        *rt = AST_REDIR_IN;
        return 1;
    }
    if (t == TOK_REDIR_APP)
    {
        *rt = AST_REDIR_APP;
        return 1;
    }
    return 0;
}

/* Parse une redirection et retourne le noeud AST */
static struct ast *parse_redir(struct parser *parser, struct ast *left_cmd)
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
    enum redir_type curr_redir_type;
    if (!tok_to_redir(parser->curr_tok->type, &curr_redir_type))
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
    struct ast_redirection *curr = &root->data.redir;
    while (curr->left && curr->left->type == AST_REDIRECTION)
        curr = &curr->left->data.redir;
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

/* Retourne 1 si le token est un ASSIGNMENT_WORD (contient '=' sans chiffre
 * avant) */
static int is_assignment(const char *word)
{
    if (!word || !word[0])
        return 0;
    /* Le premier caractère doit être une lettre ou _ */
    if (!(word[0] == '_' || (word[0] >= 'a' && word[0] <= 'z')
          || (word[0] >= 'A' && word[0] <= 'Z')))
        return 0;
    for (int i = 1; word[i]; i++)
    {
        if (word[i] == '=')
            return 1;
        if (!(word[i] == '_' || (word[i] >= 'a' && word[i] <= 'z')
              || (word[i] >= 'A' && word[i] <= 'Z')
              || (word[i] >= '0' && word[i] <= '9')))
            return 0;
    }
    return 0;
}

/* Collecte les assignments et redirections prefixes */
static int parse_prefixes(struct parser *parser, struct ast **redirs,
                          char ***assignments, size_t *nassign,
                          size_t *assign_cap)
{
    while (parser->curr_tok
           && (is_redirection(parser->curr_tok->type)
               || (parser->curr_tok->type == TOK_WORD
                   && is_assignment(parser->curr_tok->val))))
    {
        if (is_redirection(parser->curr_tok->type))
        {
            struct ast *r = parse_redir(parser, NULL);
            if (!r)
                return 0;
            *redirs = app_redir(*redirs, r);
        }
        else
        {
            if (*nassign + 1 >= *assign_cap)
            {
                *assign_cap *= 2;
                char **tmp =
                    realloc(*assignments, sizeof(char *) * (*assign_cap));
                if (!tmp)
                    return 0;
                *assignments = tmp;
            }
            (*assignments)[(*nassign)++] = strdup(parser->curr_tok->val);
            parser_consume(parser);
        }
    }
    return 1;
}

/* Applique les assignments au parser */
static void apply_assignments(struct parser *parser, char **assignments,
                              size_t nassign)
{
    for (size_t i = 0; i < nassign; i++)
    {
        char *eq = strchr(assignments[i], '=');
        if (eq)
        {
            *eq = '\0';
            add_var(parser, assignments[i], eq + 1);
            *eq = '=';
        }
        free(assignments[i]);
    }
    free(assignments);
}

/* Attache les redirections a la commande */
static struct ast *attach_redirs(struct ast *redirs, struct ast *cmd)
{
    if (!redirs)
        return cmd;
    struct ast_redirection *tmp = &redirs->data.redir;
    while (tmp->left && tmp->left->type == AST_REDIRECTION)
        tmp = &tmp->left->data.redir;
    tmp->left = cmd;
    return redirs;
}

/* Collecte les mots et redirections elements de la commande */
static int parse_elements(struct parser *parser, struct ast **redirs,
                          char ***words, size_t *count, size_t *cap)
{
    while (parser->curr_tok
           && (parser->curr_tok->type == TOK_WORD
               || is_redirection(parser->curr_tok->type)))
    {
        if (is_redirection(parser->curr_tok->type))
        {
            struct ast *r = parse_redir(parser, NULL);
            if (!r)
                return 0;
            *redirs = app_redir(*redirs, r);
        }
        else
        {
            if (*count + 1 >= *cap)
            {
                *cap *= 2;
                char **tmp = realloc(*words, sizeof(char *) * (*cap));
                if (!tmp)
                    return 0;
                *words = tmp;
            }
            (*words)[(*count)++] = strdup(parser->curr_tok->val);
            parser_consume(parser);
        }
    }
    (*words)[*count] = NULL;
    return 1;
}

/* Parse une commande simple avec prefixes et elements */
struct ast *parse_simple_command(struct parser *parser)
{
    if (!parser || !parser->curr_tok)
        return NULL;

    struct ast *redirs = NULL;
    char **assignments = malloc(sizeof(char *) * 4);
    size_t nassign = 0;
    size_t assign_cap = 4;
    if (!assignments)
        return NULL;

    if (!parse_prefixes(parser, &redirs, &assignments, &nassign, &assign_cap))
        goto error_prefix;

    if (!parser->curr_tok || parser->curr_tok->type != TOK_WORD)
    {
        apply_assignments(parser, assignments, nassign);
        return attach_redirs(redirs, create_cmd(NULL));
    }

    size_t cap = 4;
    size_t count = 0;
    char **words = malloc(sizeof(char *) * cap);
    if (!words)
        goto error_prefix;

    if (!parse_elements(parser, &redirs, &words, &count, &cap))
        goto error_words;

    apply_assignments(parser, assignments, nassign);
    return attach_redirs(redirs, create_cmd(words));

error_words:
    free_words(words, count);
error_prefix:
    for (size_t i = 0; i < nassign; i++)
        free(assignments[i]);
    free(assignments);
    ast_free(redirs);
    return NULL;
}

static int parse_pipeline_cmds(struct parser *parser, struct ast ***cmds,
                               size_t *count, size_t *capacity)
{
    struct ast *cmd = parse_command(parser);
    if (!cmd)
        return 0;
    (*cmds)[(*count)++] = cmd;

    while (parser->curr_tok && parser->curr_tok->type == TOK_PIPE)
    {
        parser_consume(parser);
        /* Sauter les newlines après | */
        while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
            parser_consume(parser);
        if (!parser->curr_tok || parser->curr_tok->type == TOK_EOF
            || parser->curr_tok->type == TOK_PIPE)
        {
            fprintf(stderr,
                    "minishell: syntax error near unexpected token `|'\n");
            return 0;
        }
        cmd = parse_command(parser);
        if (!cmd)
            return 0;
        if (*count == *capacity)
        {
            *capacity *= 2;
            struct ast **tmp = realloc(*cmds, sizeof(*tmp) * (*capacity));
            if (!tmp)
                return 0;
            *cmds = tmp;
        }
        (*cmds)[(*count)++] = cmd;
    }
    return 1;
}

struct ast *parse_pipeline(struct parser *parser)
{
    size_t capacity = 4;
    size_t count = 0;
    struct ast **cmds = malloc(sizeof(*cmds) * capacity);
    if (!cmds)
        return NULL;

    if (!parse_pipeline_cmds(parser, &cmds, &count, &capacity))
    {
        for (size_t i = 0; i < count; i++)
            ast_free(cmds[i]);
        free(cmds);
        return NULL;
    }

    if (count == 1)
    {
        struct ast *r = cmds[0];
        free(cmds);
        return r;
    }
    return ast_pipeline_create(cmds, count);
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
            parser_consume(parser);

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

/* list = and_or { ';' and_or } [ ';' ] */

/* Gere l'erreur de double point-virgule */
static void free_list_cmds(struct ast **cmds, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ast_free(cmds[i]);
    free(cmds);
}

static void handle_double_semi(struct parser *parser, struct ast **cmds,
                               size_t count)
{
    fprintf(stderr, "minishell: syntax error near unexpected token ';;'\n");
    parser->parse_error = 1;
    while (parser->curr_tok && parser->curr_tok->type != TOK_EOF
           && parser->curr_tok->type != TOK_NEWLINE)
        parser_consume(parser);
    for (size_t i = 0; i < count; i++)
        ast_free(cmds[i]);
    free(cmds);
}

/* Parse une liste de commandes separees par ';' */
static struct ast *parse_list(struct parser *parser)
{
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
    while (parser->curr_tok && parser->curr_tok->type == TOK_SEMI)
    {
        parser_consume(parser);
        if (parser->curr_tok && parser->curr_tok->type == TOK_SEMI)
        {
            handle_double_semi(parser, cmds, count);
            return NULL;
        }
        if (!parser->curr_tok || parser->curr_tok->type == TOK_EOF
            || parser->curr_tok->type == TOK_NEWLINE)
            break;
        struct ast *next = parse_and_or(parser);
        if (!next)
            break;
        if (!add_ast(&cmds, &count, &cap, next))
        {
            ast_free(next);
            free_list_cmds(cmds, count);
            return NULL;
        }
    }
    if (count == 1)
    {
        free(cmds);
        return first;
    }
    return create_list(cmds, count);
}
static struct ast *parse_command(struct parser *parser)
{
    if (!parser || !parser->curr_tok)
        return NULL;
    return parse_simple_command(parser);
}

/* input = list '\n' | list EOF | '\n' | EOF */
struct ast *parser_input(struct parser *parser)
{
    if (!parser)
        return NULL;
    /* Sauter les newlines en tête */
    while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
        parser_consume(parser);
    if (!parser->curr_tok || parser->curr_tok->type == TOK_EOF)
        return NULL;
    return parse_list(parser);
}
