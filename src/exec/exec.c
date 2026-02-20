#define _POSIX_C_SOURCE 200809L
#include "exec.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../ast/ast.h"
#include "../builtin/builtin.h"
#include "../expansion/expansion.h"
#include "../parser/parser.h"

struct parser *g_parser = NULL;

/* ---- Declarations anticipees ---- */
int exec_ast(struct ast *ast);
static int find_targ_fd(struct ast_redirection *r);
static void redirect(struct ast_redirection *r);
static struct ast *all_redirections(struct ast *ast);

/* ---- Utilitaires redirection ---- */

static int find_targ_fd(struct ast_redirection *r)
{
    if (r->redir_nb != -1)
        return r->redir_nb;
    if (r->type == AST_REDIR_IN)
        return STDIN_FILENO;
    return STDOUT_FILENO;
}

static void redirect(struct ast_redirection *r)
{
    int target_fd = find_targ_fd(r);
    int fd = -1;

    if (r->type == AST_REDIR_IN)
        fd = open(r->file, O_RDONLY);
    else if (r->type == AST_REDIR_OUT)
        fd = open(r->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    else if (r->type == AST_REDIR_APP)
        fd = open(r->file, O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (fd < 0)
    {
        perror(r->file);
        _exit(1);
    }

    dup2(fd, target_fd);
    close(fd);
}

static struct ast *all_redirections(struct ast *ast)
{
    while (ast && ast->type == AST_REDIRECTION)
    {
        redirect(&ast->data.redir);
        ast = ast->data.redir.left;
    }
    return ast;
}

/* Expand tous les arguments de la commande */
static char **expand_args(char **argv, int cnt)
{
    char **exp = malloc(sizeof(char *) * (cnt + 1));
    if (!exp || !g_parser)
    {
        free(exp);
        return NULL;
    }
    for (int k = 0; k < cnt; k++)
    {
        char *e = expand(g_parser, g_parser->spe, argv[k]);
        exp[k] = e ? e : strdup(argv[k]);
    }
    exp[cnt] = NULL;
    return exp;
}

/* Libere le tableau d'arguments expandus */
static void free_exp(char **exp, int cnt)
{
    for (int k = 0; k < cnt; k++)
        free(exp[k]);
    free(exp);
}

/* Forke et execute une commande externe */
static int fork_exec(char **exp, int cnt)
{
    pid_t p = fork();
    if (p < 0)
    {
        perror("fork");
        free_exp(exp, cnt);
        return 1;
    }
    if (p == 0)
    {
        execvp(exp[0], exp);
        fprintf(stderr, "minishell: %s: command not found\n", exp[0]);
        _exit(127);
    }
    int st;
    waitpid(p, &st, 0);
    free_exp(exp, cnt);
    return WIFEXITED(st) ? WEXITSTATUS(st) : 1;
}

/* Execute une commande simple (builtin ou externe) */
static int exec_command(char **argv)
{
    if (!argv || !argv[0])
        return 0;
    int cnt = 0;
    while (argv[cnt])
        cnt++;
    char **exp = expand_args(argv, cnt);
    if (!exp)
        return 1;
    if (is_builtin(exp[0]))
    {
        int ret = execute_builtin(exp, g_parser);
        free_exp(exp, cnt);
        return ret;
    }
    return fork_exec(exp, cnt);
}

/* ---- Redirections ---- */

/* Execute une redirection avec commande externe (fork) */
static int exec_redir_external(struct ast_redirection *redir)
{
    pid_t pid = fork();
    if (pid < 0)
        return 1;
    if (pid == 0)
    {
        struct ast temp;
        temp.type = AST_REDIRECTION;
        temp.data.redir = *redir;
        struct ast *c = all_redirections(&temp);
        _exit(exec_ast(c));
    }
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

/* Applique les redirections pour un builtin dans le process courant */
static int apply_redir_builtin(struct ast **c)
{
    int ok = 1;
    while (*c && (*c)->type == AST_REDIRECTION && ok)
    {
        struct ast_redirection *r = &(*c)->data.redir;
        int target = find_targ_fd(r);
        int fd = -1;
        if (r->type == AST_REDIR_IN)
            fd = open(r->file, O_RDONLY);
        else if (r->type == AST_REDIR_OUT)
            fd = open(r->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        else if (r->type == AST_REDIR_APP)
            fd = open(r->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0)
        {
            perror(r->file);
            ok = 0;
            break;
        }
        dup2(fd, target);
        close(fd);
        *c = r->left;
    }
    return ok;
}

/* Execute une redirection avec builtin (sans fork) */
static int exec_redir_builtin(struct ast_redirection *redir)
{
    int saved_in = dup(STDIN_FILENO);
    int saved_out = dup(STDOUT_FILENO);
    int saved_err = dup(STDERR_FILENO);
    struct ast temp_root;
    temp_root.type = AST_REDIRECTION;
    temp_root.data.redir = *redir;
    struct ast *c = &temp_root;
    int ok = apply_redir_builtin(&c);
    int status = ok ? exec_ast(c) : 0;
    dup2(saved_in, STDIN_FILENO);
    dup2(saved_out, STDOUT_FILENO);
    dup2(saved_err, STDERR_FILENO);
    close(saved_in);
    close(saved_out);
    close(saved_err);
    return ok ? status : 1;
}

/* Execute une redirection selon le type de commande */
static int exec_redirection(struct ast_redirection *redir)
{
    struct ast *cmd = redir->left;
    while (cmd && cmd->type == AST_REDIRECTION)
        cmd = cmd->data.redir.left;
    int is_blt =
        (cmd && cmd->type == AST_COMMAND && cmd->data.cmd.words
         && cmd->data.cmd.words[0] && is_builtin(cmd->data.cmd.words[0]));
    if (!is_blt)
        return exec_redir_external(redir);
    return exec_redir_builtin(redir);
}

/* ---- Pipeline ---- */

static void exec_child(struct ast_pipeline *p, size_t i, int prev_fd,
                       int pipefd[2], pid_t *pids)
{
    free(pids);
    if (prev_fd != -1)
    {
        dup2(prev_fd, STDIN_FILENO);
        close(prev_fd);
    }
    if (i + 1 < p->count)
    {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
    }
    _exit(exec_ast(p->cmds[i]));
}

/* Attend la fin de tous les enfants et retourne le status du dernier */
static int wait_pipeline(pid_t *pids, size_t count)
{
    int status = 0;
    for (size_t i = 0; i < count; i++)
    {
        int wstatus;
        waitpid(pids[i], &wstatus, 0);
        if (i + 1 == count && WIFEXITED(wstatus))
            status = WEXITSTATUS(wstatus);
    }
    return status;
}

/* Forke et connecte les commandes du pipeline */
static int fork_pipeline(struct ast_pipeline *p, pid_t *pids)
{
    int prev_fd = -1;
    int pipefd[2];
    for (size_t i = 0; i < p->count; i++)
    {
        if (i + 1 < p->count && pipe(pipefd) < 0)
        {
            perror("pipe");
            return 0;
        }
        pid_t pid = fork();
        
	if (pid < 0)
        {
            perror("fork");
            return 0;
        }
        
	if (pid == 0)
            exec_child(p, i, prev_fd, pipefd, pids);
        
	pids[i] = pid;
        
	if (prev_fd != -1)
            close(prev_fd);
        
	if (i + 1 < p->count)
        {
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }
    }
    return 1;
}

/* Execute un pipeline de commandes */
static int exec_pipeline(struct ast_pipeline *p)
{
    if (!p || p->count == 0)
        return 0;
    if (p->count == 1)
        return exec_ast(p->cmds[0]);
    pid_t *pids = malloc(p->count * sizeof(pid_t));
    if (!pids)
        return 1;
    if (!fork_pipeline(p, pids))
    {
        free(pids);
        return 1;
    }
    int status = wait_pipeline(pids, p->count);
    free(pids);
    return status;
}

/* ---- Entree principale ---- */

/* Execute une liste de commandes separees par ';' */
static int exec_list(struct ast_list *list)
{
    int status = 0;
    for (size_t i = 0; i < list->count; i++)
    {
        status = exec_ast(list->commands[i]);
        if (g_parser && g_parser->spe)
            g_parser->spe->exit_code = status;
    }
    return status;
}

/* Execute un operateur && */

static int exec_and(struct ast_and_or *a)
{
    int status = exec_ast(a->left);
    if (g_parser && g_parser->spe)
        g_parser->spe->exit_code = status;
    if (status == 0)
        return exec_ast(a->right);
    return status;
}

/* Execute un operateur || */
static int exec_or(struct ast_and_or *a)
{
    int status = exec_ast(a->left);
    if (g_parser && g_parser->spe)
        g_parser->spe->exit_code = status;
    if (status != 0)
        return exec_ast(a->right);
    return status;
}

/* Execute recursivement un noeud AST */
int exec_ast(struct ast *ast)
{
    if (g_parser && g_parser->exit)
        _exit(g_parser->ex_code);
    if (g_parser && g_parser->parse_error)
        return 2;
    if (!ast)
        return 0;
    switch (ast->type)
    {
    case AST_COMMAND:
        return exec_command(ast->data.cmd.words);
    case AST_LIST:
        return exec_list(&ast->data.list);
    case AST_PIPELINE:
        return exec_pipeline(&ast->data.pipeline);
    case AST_AND:
        return exec_and(&ast->data.and_or);
    case AST_OR:
        return exec_or(&ast->data.and_or);
    case AST_REDIRECTION:
        return exec_redirection(&ast->data.redir);
    default:
        return 0;
    }
}

void exec_set_parser(struct parser *parser)
{
    g_parser = parser;
}
