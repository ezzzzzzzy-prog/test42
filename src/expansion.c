#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "special.h"
#include "expansion.h"

static int agrandir( char **res, size_t *size) //c'est une fonction pour agrandir
{
        *size *= 2;
       char *tmp = realloc(*res, *size);
       if(!tmp)
       {
                free(*res);
                return 0;
       }
       *res = tmp;
       return 1;
}


static int copy(const char *val, char **res, size_t *j, size_t *size) //pour copier
{
        while(*val)
        {
                 if(*j >= *size -1)
                 {
                         if(!agrandir(res,size))
                         {
                                 return 0;
                         }
                 }
                 (*res)[(*j)++] = *val++;
         }
         return 1;
}

static char *var_normal(const char *word, size_t *i, int is_bra)
{
        size_t start = *i;
        if(is_bra)
        {
                while(word[*i] && word[*i] != '}')
                {
                        (*i)++;
                }
        }
        else
        {
                while(word[*i] && word[*i] != ' ' && word[*i] != '\t' && word[*i] != '\n' && word[*i] != '$' && word[*i] != '"' && word[*i] != '\'')
                {
                                    (*i)++;
                }
        }
        size_t len = *i - start;
        char *name = malloc(len +1);
        if(!name)
        {
                //free(res);
                return NULL;
        }
        memcpy(name, word + start, len);
        name[len] = '\0';
        return name;
}
static const char *find_var(const char *name, struct variable *var)
{
    struct variable *v = var;
    while (v)
    {
        if (v->nom && strcmp(v->nom, name) == 0)
        {
            return v->value ? v->value : "";
        }
        v = v->next;
    }
    return "";
}

static int expand_special(struct special *spe, char spe_char, char **res, size_t *j, size_t *size)
{
	if(spe && spe->args)
		return 1;

                            for(int l = 0; l < spe->argc_count; l++)
                            {
                                    if(!copy(spe->args[l], res, j, size))
                                    {
                                            return 0;
                                    }
                                    if(spe_char == '@' && l +1 < spe->argc_count)
                                    {
                                            if( *j >= *size - 1 && !agrandir(res, size))
                                            {
                                                    return 0;
                                            }
                                            (*res)[(*j)++] = ' ';
                                    }
                            }
			    return 1;
}

static int expand_braced_var(struct parser *parser, const char *word, size_t *i, char **res, size_t *j, size_t *size)
{
	(*i)++;
	char *name = var_normal(word, i, 1);
                    if(!name)
                    {
                            //free(res);
                            return 0;
                    }
                    const char *val = find_var(name, parser->var);
                    if(!copy(val, res,j, size))
                    {
                            free(name);
                            //free(res);
                            return 0;
                    }
                    free(name);
		    return 1;
}

static int expand_simple_var(struct parser *parser, const char *word, size_t *i, char **res, size_t *j, size_t *size)
{
	char *name = var_normal(word, i, 0);
            if(!name)
            {
                    //free(res);
                    return 0;
            }
            const char *val = find_var(name, parser->var);
            if(!copy(val, res, j, size))
            {
                    free(name);
                    //free(res);
                    return 0;
            }
            free(name);
	    return 1;
}

char *expand(struct parser *parser,struct special *spe, const char *word)
{
    if (!word || !parser)
    {
        return NULL;
    }
    size_t len = strlen(word);
    size_t size = len * 2 + 1;
    char *res = malloc(size);
    if (!res)
    {
        return NULL;
    }
    size_t i = 0;
    size_t j = 0;
    while (word[i])
    {
        if (word[i] == '$')
        {
            i++;
            if(!word[i])
            {
                    break;
            }
            if(word[i] == '@' || word[i] == '*')
            {
                    char spe_char = word[i++];
		    if (!expand_special(spe,spe_char, &res, &j, &size))
			    return NULL;
                    /*if(spe && spe->args)
                    {
                            for(int l = 0; l < spe->argc_count; l++)
                            {
                                    if(!copy(spe->args[l], &res, &j, &size))
                                    {
                                            return NULL;
                                    }
                                    if(spe_char == '@' && l +1 < spe->argc_count)
                                    {
                                            if( j >= size - 1 && !agrandir(&res, &size))
                                            {
                                                    return NULL;
                                            }
                                            res[j++] = ' ';
                                    }
                            }
                    }
                    continue;*/
            }
            if(word[i] == '{')
            {
                    /*i++;
                    char *name = var_normal(word, &i, 1);
                    if(!name)
                    {
                            free(res);
                            return NULL;
                    }
                    const char *val = find_var(name, parser->var);
                    if(!copy(val, &res, &j, &size))
                    {
                            free(name);
                            free(res);
                            return NULL;
                    }
                    free(name);*/
		    if (!expand_braced_var(parser, word, &i, &res, &j, &size))
			    return NULL;
                    if(word[i] == '}')
                    {
                            i++;
                    }
                    continue;
            }
            /*char *name = var_normal(word, &i, 0);
            if(!name)
            {
                    free(res);
                    return NULL;
            }
            const char *val = find_var(name, parser->var);
            if(!copy(val, &res, &j, &size))
            {
                    free(name);
                    free(res);
                    return NULL;
            }
            free(name);*/
	    if (!expand_simple_var(parser, word, &i, &res, &j, &size))
		    return NULL;
        }
        else
        {
                if(j >= size - 1 && !agrandir(&res, &size))
                {
                        return NULL;
                }
                res[j++] = word[i++];
        }
    }
    res[j] = '\0';
    return res;
}
