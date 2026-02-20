#include "expansion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../special/special.h"

struct exp_ctx
{
    char *res;
    size_t pos;
    size_t sz;
};

struct exp_params
{
    struct special *spe;
    struct variable *var;
    struct exp_ctx *ctx;
    int in_dquotes;
};

static int agrandir(struct exp_ctx *ctx)
{
    ctx->sz = ctx->sz * 2;
    char *tmp = realloc(ctx->res, ctx->sz);
    if (!tmp)
    {
        free(ctx->res);
        return 0;
    }
    ctx->res = tmp;
    return 1;
}

static int copy(const char *val, struct exp_ctx *ctx)
{
    while (*val)
    {
        if (ctx->pos >= ctx->sz - 1)
        {
            if (!agrandir(ctx))
                return 0;
        }
        ctx->res[ctx->pos] = *val;
        ctx->pos = ctx->pos + 1;
        val = val + 1;
    }
    return 1;
}

static int est_alphanum(char c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 1;
    if (c >= '0' && c <= '9')
        return 1;
    if (c == '_')
        return 1;
    return 0;
}

static int est_alpha(char c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 1;
    if (c == '_')
        return 1;
    return 0;
}

static int copier_dollar(struct exp_ctx *ctx)
{
    if (ctx->pos >= ctx->sz - 1)
    {
        if (!agrandir(ctx))
            return 0;
    }
    ctx->res[ctx->pos] = '$';
    ctx->pos = ctx->pos + 1;
    return 1;
}

static char *get_name(const char *word, size_t *i)
{
    size_t debut = *i;

    if (!est_alpha(word[*i]))
        return NULL;

    while (word[*i] && est_alphanum(word[*i]))
    {
        *i = *i + 1;
    }

    size_t taille = *i - debut;
    char *nom = malloc(taille + 1);
    if (!nom)
        return NULL;

    memcpy(nom, word + debut, taille);
    nom[taille] = '\0';
    return nom;
}

static char *get_name_brace(const char *word, size_t *i)
{
    size_t debut = *i;

    while (word[*i] && word[*i] != '}')
    {
        *i = *i + 1;
    }

    size_t taille = *i - debut;
    if (taille == 0)
        return NULL;

    char *nom = malloc(taille + 1);
    if (!nom)
        return NULL;

    memcpy(nom, word + debut, taille);
    nom[taille] = '\0';
    return nom;
}

static const char *find_var(const char *name, struct variable *var)
{
    struct variable *actuel = var;
    while (actuel)
    {
        if (actuel->nom && strcmp(actuel->nom, name) == 0)
        {
            if (actuel->value)
                return actuel->value;
            return "";
        }
        actuel = actuel->next;
    }
    char *env = getenv(name);
    if (env)
    {
        return env;
    }

    return "";
}

static int exp_special(struct special *spe, struct exp_ctx *ctx)
{
    char buf[32];
    int status = 0;

    if (spe)
        status = spe->exit_code;
    snprintf(buf, sizeof(buf), "%d", status);
    return copy(buf, ctx);
}

static int exp_nom_special(const char *nom, struct exp_params *p)
{
    const char *val = find_var(nom, p->var);
    return copy(val, p->ctx);
}

static int traiter_char_special(char c, struct exp_params *p)
{
    if (c == '?')
        return exp_special(p->spe, p->ctx);

    return copier_dollar(p->ctx);
}

static int exp_brace(struct exp_params *p, const char *word, size_t *i)
{
    if (word[*i] == '?')
    {
        *i = *i + 1;
        return exp_special(p->spe, p->ctx);
    }

    char *nom = get_name_brace(word, i);
    if (!nom)
        return 1;

    const char *val = find_var(nom, p->var);
    free(nom);
    return copy(val, p->ctx);
}

static int exp_simple(struct exp_params *p, const char *word, size_t *i)
{
    char *nom = NULL;
    int ret = 0;

    if (word[*i] == '?')
    {
        char c = word[*i];
        *i = *i + 1;
        return traiter_char_special(c, p);
    }

    nom = get_name(word, i);
    if (!nom)
        return copier_dollar(p->ctx);

    ret = exp_nom_special(nom, p);
    free(nom);
    return ret;
}

static int copier_char(struct exp_ctx *ctx, char c)
{
    if (ctx->pos >= ctx->sz - 1)
    {
        if (!agrandir(ctx))
            return 0;
    }
    ctx->res[ctx->pos] = c;
    ctx->pos = ctx->pos + 1;
    return 1;
}

static int traiter_dollar(struct exp_params *p, const char *word, size_t *i)
{
    *i = *i + 1;
    if (!word[*i])
        return copier_dollar(p->ctx);

    if (word[*i] == '{')
    {
        *i = *i + 1;
        if (!exp_brace(p, word, i))
            return 0;
        if (word[*i] == '}')
            *i = *i + 1;
        return 1;
    }

    return exp_simple(p, word, i);
}

static int boucle_expansion(struct exp_params *p, const char *word, size_t *idx)
{
    while (word[*idx])
    {
        char c = word[*idx];

        if (c == '\001')
        {
            if (!copier_char(p->ctx, '$'))
                return 0;
            *idx += 1;
            continue;
        }

        if (c == '$')
        {
            if (!traiter_dollar(p, word, idx))
                return 0;
            continue;
        }

        if (!copier_char(p->ctx, c))
            return 0;

        *idx += 1;
    }
    return 1;
}

/* Gere le cas mot entierement entre single-quotes */
static char *expand_single_quote(const char *word, size_t len)
{
    char *result = malloc(len - 1);
    if (!result)
        return NULL;
    memcpy(result, word + 1, len - 2);
    result[len - 2] = '\0';
    return result;
}

/* Initialise le contexte d'expansion */
static int init_exp_ctx(struct exp_ctx *ctx, struct exp_params *p,
                        struct parser *parser, struct special *spe, size_t len)
{
    ctx->sz = len * 2 + 128;
    ctx->res = malloc(ctx->sz);
    ctx->pos = 0;
    if (!ctx->res)
        return 0;
    p->spe = spe;
    p->var = parser->var;
    p->ctx = ctx;
    p->in_dquotes = 0;
    return 1;
}

/* Realise l'expansion d'un mot */
char *expand(struct parser *parser, struct special *spe, const char *word)
{
    if (!word || !parser)
        return NULL;
    size_t len = strlen(word);
    if (len >= 2 && word[0] == '\'' && word[len - 1] == '\''
        && memchr(word + 1, '\'', len - 2) == NULL)
        return expand_single_quote(word, len);
    struct exp_ctx ctx;
    struct exp_params p;
    if (!init_exp_ctx(&ctx, &p, parser, spe, len))
        return NULL;
    size_t idx = 0;
    char *result = NULL;
    if (boucle_expansion(&p, word, &idx))
    {
        ctx.res[ctx.pos] = '\0';
        result = ctx.res;
    }
    else
        free(ctx.res);
    return result;
}

int unset_variable(struct parser *parser, const char *name)
{
    if (!parser || !name || !*name)
        return 1;

    if (strcmp(name, "@") == 0 || strcmp(name, "*") == 0
        || strcmp(name, "#") == 0 || strcmp(name, "?") == 0
        || strcmp(name, "$") == 0 || strcmp(name, "!") == 0
        || strcmp(name, "0") == 0)
        return 1;

    if (name[0] >= '0' && name[0] <= '9')
        return 1;

    struct variable *prev = NULL;
    struct variable *curr = parser->var;

    while (curr)
    {
        if (strcmp(curr->nom, name) == 0)
        {
            if (prev)
                prev->next = curr->next;
            else
                parser->var = curr->next;

            free(curr->nom);
            free(curr->value);
            free(curr);

            return 0;
        }
        prev = curr;
        curr = curr->next;
    }

    return 0;
}

int unset_function(struct parser *parser, const char *name)
{
    if (!parser || !name || !*name)
        return 1;

    return 0;
}
