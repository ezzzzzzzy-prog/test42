#include "expansion.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "special.h"

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
        return env;
    
    return "";
}

static const char *get_ifs(struct variable *var)
{
    const char *ifs = find_var("IFS", var);
    if (!ifs || !*ifs)
        return " \t\n";
    return ifs;
}

static int ajouter_sep(struct exp_ctx *ctx, char sep)
{
    if (ctx->pos >= ctx->sz - 1)
    {
        if (!agrandir(ctx))
            return 0;
    }
    ctx->res[ctx->pos] = sep;
    ctx->pos = ctx->pos + 1;
    return 1;
}

static int exp_args(struct exp_params *p)
{
    int idx = 0;
    struct special *spe = p->spe;
    
    if (!spe || !spe->args)
        return 1;
    
    while (idx < spe->argc_count)
    {
        if (!copy(spe->args[idx], p->ctx))
            return 0;
        
        if (idx + 1 < spe->argc_count)
        {
            if (!ajouter_sep(p->ctx, ' '))
                return 0;
        }
        idx = idx + 1;
    }
    return 1;
}

static int exp_args_star(struct exp_params *p)
{
    int idx = 0;
    char sep = ' ';
    struct special *spe = p->spe;
    
    if (!spe || !spe->args)
        return 1;
    
    if (p->in_dquotes)
    {
        const char *ifs = get_ifs(p->var);
        sep = (ifs && *ifs) ? *ifs : ' ';
    }
    
    while (idx < spe->argc_count)
    {
        if (!copy(spe->args[idx], p->ctx))
            return 0;
        
        if (idx + 1 < spe->argc_count)
        {
            if (!ajouter_sep(p->ctx, sep))
                return 0;
        }
        idx = idx + 1;
    }
    return 1;
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

static int exp_dollar(struct special *spe, struct exp_ctx *ctx)
{
    char buf[32];
    int pid = 0;
    
    if (spe)
        pid = spe->shell_pid;
    snprintf(buf, sizeof(buf), "%d", pid);
    return copy(buf, ctx);
}

static int exp_diese(struct special *spe, struct exp_ctx *ctx)
{
    char buf[32];
    int count = 0;
    
    if (spe)
        count = spe->argc_count;
    snprintf(buf, sizeof(buf), "%d", count);
    return copy(buf, ctx);
}

static int exp_zero(struct special *spe, struct exp_ctx *ctx)
{
    if (spe && spe->script_name)
        return copy(spe->script_name, ctx);
    return copy("42sh", ctx);
}

static int exp_bang(struct special *spe, struct exp_ctx *ctx)
{
    char buf[32];
    int pid = 0;
    
    if (spe)
        pid = spe->last_bg_pid;
    snprintf(buf, sizeof(buf), "%d", pid);
    return copy(buf, ctx);
}

static int exp_tiret(struct special *spe, struct exp_ctx *ctx)
{
    if (spe && spe->shell_opts)
        return copy(spe->shell_opts, ctx);
    return copy("", ctx);
}

static int exp_chiffre(struct special *spe, struct exp_ctx *ctx, char c)
{
    int num = c - '0';
    
    if (spe && spe->args && num <= spe->argc_count)
        return copy(spe->args[num - 1], ctx);
    return 1;
}

static int exp_pwd(struct special *spe, struct variable *var, struct exp_ctx *ctx)
{
    const char *pwd = find_var("PWD", var);
    
    if (pwd && *pwd)
        return copy(pwd, ctx);
    
    if (spe && spe->pwd)
        return copy(spe->pwd, ctx);
    
    return 1;
}

static int exp_oldpwd(struct special *spe, struct variable *var, struct exp_ctx *ctx)
{
    const char *oldpwd = NULL;
    
    if (var)
    {
        struct variable *actuel = var;
        while (actuel)
        {
            if (actuel->nom && strcmp(actuel->nom, "OLDPWD") == 0)
            {
                if (actuel->value)
                    return copy(actuel->value, ctx);
                break;
            }
            actuel = actuel->next;
        }
    }
    
    if (spe && spe->oldpwd)
        return copy(spe->oldpwd, ctx);
    
    oldpwd = getenv("OLDPWD");
    if (oldpwd)
        return copy(oldpwd, ctx);
    
    return 1;
}

static int exp_random(struct exp_ctx *ctx)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", rand() % 32768);
    return copy(buf, ctx);
}

static int exp_uid(struct special *spe, struct exp_ctx *ctx)
{
    char buf[32];
    int uid = 0;
    
    if (spe)
        uid = spe->uid;
    snprintf(buf, sizeof(buf), "%d", uid);
    return copy(buf, ctx);
}

static int exp_nom_pwd(struct exp_params *p)
{
    return exp_pwd(p->spe, p->var, p->ctx);
}

static int exp_nom_oldpwd(struct exp_params *p)
{
    return exp_oldpwd(p->spe, p->var, p->ctx);
}

static int exp_nom_ifs(struct exp_params *p)
{
    const char *ifs = get_ifs(p->var);
    return copy(ifs, p->ctx);
}

static int exp_nom_random(struct exp_params *p)
{
    return exp_random(p->ctx);
}

static int exp_nom_uid(struct exp_params *p)
{
    return exp_uid(p->spe, p->ctx);
}

static int exp_nom_normal(const char *nom, struct exp_params *p)
{
    const char *val = find_var(nom, p->var);
    return copy(val, p->ctx);
}

static int exp_nom_special(const char *nom, struct exp_params *p)
{
    if (strcmp(nom, "PWD") == 0)
        return exp_nom_pwd(p);
    
    if (strcmp(nom, "OLDPWD") == 0)
        return exp_nom_oldpwd(p);
    
    if (strcmp(nom, "IFS") == 0)
        return exp_nom_ifs(p);
    
    if (strcmp(nom, "RANDOM") == 0)
        return exp_nom_random(p);
    
    if (strcmp(nom, "UID") == 0)
        return exp_nom_uid(p);
    
    return exp_nom_normal(nom, p);
}

static int traiter_char_special(char c, struct exp_params *p)
{
    if (c == '?')
        return exp_special(p->spe, p->ctx);
    
    if (c == '$')
        return exp_dollar(p->spe, p->ctx);
    
    if (c == '#')
        return exp_diese(p->spe, p->ctx);
    
    if (c == '0')
        return exp_zero(p->spe, p->ctx);
    
    if (c == '!')
        return exp_bang(p->spe, p->ctx);
    
    if (c == '-')
        return exp_tiret(p->spe, p->ctx);
    
    if (c >= '1' && c <= '9')
        return exp_chiffre(p->spe, p->ctx, c);
    
    return 1;
}

static int exp_brace(struct exp_params *p, const char *word, size_t *i)
{
    char *nom = NULL;
    int ret = 0;
    
    if (word[*i] == '@')
    {
        *i = *i + 1;
        return exp_args(p);
    }
    
    if (word[*i] == '*')
    {
        *i = *i + 1;
        return exp_args_star(p);
    }
    
    if (word[*i] == '?' || word[*i] == '$' || word[*i] == '#' || word[*i] == '0' || word[*i] == '!' || word[*i] == '-' || (word[*i] >= '1' && word[*i] <= '9'))
    {
        char c = word[*i];
        *i = *i + 1;
        return traiter_char_special(c, p);
    }
    
    nom = get_name_brace(word, i);
    if (!nom)
        return 1;
    
    ret = exp_nom_special(nom, p);
    free(nom);
    return ret;
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

static int exp_simple(struct exp_params *p, const char *word, size_t *i)
{
    char *nom = NULL;
    int ret = 0;
    
    if (word[*i] == '@')
    {
        *i = *i + 1;
        return exp_args(p);
    }
    
    if (word[*i] == '*')
    {
        *i = *i + 1;
        return exp_args_star(p);
    }
    
    if (word[*i] == '?' || word[*i] == '$' || word[*i] == '#' || word[*i] == '0' || word[*i] == '!' || word[*i] == '-' || (word[*i] >= '1' && word[*i] <= '9'))
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

static int copier_backslash(struct exp_ctx *ctx, char next)
{
    if (ctx->pos >= ctx->sz - 1)
    {
        if (!agrandir(ctx))
            return 0;
    }
    ctx->res[ctx->pos] = next;
    ctx->pos = ctx->pos + 1;
    return 1;
}

static int exp_backslash_dquotes(const char *word, size_t *i, struct exp_ctx *ctx)
{
    char next = 0;
    
    *i = *i + 1;
    if (!word[*i])
        return copier_backslash(ctx, '\\');
    
    next = word[*i];
    
    if (next == '$' || next == '`' || next == '"' || next == '\\')
    {
        *i = *i + 1;
        return copier_backslash(ctx, next);
    }
    
    if (next == '\n')
    {
        *i = *i + 1;
        return 1;
    }
    
    return copier_backslash(ctx, '\\');
}

static int exp_backslash_normal(const char *word, size_t *i, struct exp_ctx *ctx)
{
    char next = 0;
    
    *i = *i + 1;
    if (!word[*i])
        return copier_backslash(ctx, '\\');
    
    next = word[*i];
    
    if (next == '\n')
    {
        *i = *i + 1;
        return 1;
    }
    
    *i = *i + 1;
    return copier_backslash(ctx, next);
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

static char *expand_squotes(const char *word)
{
    size_t len = strlen(word);
    char *result = NULL;
    
    if (len < 2)
        return NULL;
    
    result = malloc(len - 1);
    if (!result)
        return NULL;
    
    memcpy(result, word + 1, len - 2);
    result[len - 2] = '\0';
    return result;
}

static int boucle_squotes(const char *word, size_t *idx, struct exp_ctx *ctx)
{
    *idx = *idx + 1;
    
    while (word[*idx] && word[*idx] != '\'')
    {
        if (!copier_char(ctx, word[*idx]))
            return 0;
        *idx = *idx + 1;
    }
    
    if (word[*idx] == '\'')
        *idx = *idx + 1;
    return 1;
}

static int traiter_squote(struct exp_params *p, const char *word, size_t *idx)
{
    if (!boucle_squotes(word, idx, p->ctx))
        return 0;
    return 1;
}

static int traiter_dquote(struct exp_params *p, size_t *idx)
{
    p->in_dquotes = !p->in_dquotes;
    *idx = *idx + 1;
    return 1;
}

static int traiter_backslash(struct exp_params *p, const char *word, size_t *idx)
{
    if (p->in_dquotes)
        return exp_backslash_dquotes(word, idx, p->ctx);
    return exp_backslash_normal(word, idx, p->ctx);
}

static int traiter_char_normal(struct exp_params *p, char c, size_t *idx)
{
    if (!copier_char(p->ctx, c))
        return 0;
    *idx = *idx + 1;
    return 1;
}

static int boucle_expansion(struct exp_params *p, const char *word, size_t *idx)
{
    p->in_dquotes = 0;
    
    while (word[*idx])
    {
        char c = word[*idx];
        
        if (c == '\'' && !p->in_dquotes)
        {
            if (!traiter_squote(p, word, idx))
                return 0;
            continue;
        }
        
        if (c == '"')
        {
            if (!traiter_dquote(p, idx))
                return 0;
            continue;
        }
        
        if (c == '\\')
        {
            if (!traiter_backslash(p, word, idx))
                return 0;
            continue;
        }
        
        if (c == '$')
        {
            if (!traiter_dollar(p, word, idx))
                return 0;
            continue;
        }
        
        if (!traiter_char_normal(p, c, idx))
            return 0;
    }
    return 1;
}

char *expand(struct parser *parser, struct special *spe, const char *word)
{
    size_t len = 0;
    size_t idx = 0;
    struct exp_ctx ctx;
    struct exp_params p;
    
    if (!word || !parser)
        return NULL;
    
    len = strlen(word);
    
    if (len >= 2 && word[0] == '\'' && word[len - 1] == '\'')
        return expand_squotes(word);
    
    ctx.sz = len * 2 + 128;
    ctx.res = malloc(ctx.sz);
    ctx.pos = 0;
    if (!ctx.res)
        return NULL;
    
    p.spe = spe;
    p.var = parser->var;
    p.ctx = &ctx;
    p.in_dquotes = 0;
    
    if (!boucle_expansion(&p, word, &idx))
    {
        free(ctx.res);
        return NULL;
    }
    
    ctx.res[ctx.pos] = '\0';
    return ctx.res;
}

