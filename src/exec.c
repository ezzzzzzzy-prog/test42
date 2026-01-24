#define _POSIX_C_SOURCE 200809L
#include "exec.h"

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast.h"
#include "builtin.h"
#include "expansion.h"
#include "parser.h"

static int exec_redirection(struct ast *ast);
struct parser *g_parser = NULL;

static int exec_command(char **argv)
{
    if (!argv || !argv[0])
        return 0;

    if (is_builtin(argv[0]))
    {
        int ret = execute_builtin(argv, g_parser);
        if (g_parser && g_parser->exit)
            _exit(g_parser->ex_code);
        return ret;
    }
    char **expanded_argv = malloc(sizeof(char *) * 64);
    if (!expanded_argv)
        return 1;

    int count = 0;

    for (int i = 0; argv[i] != NULL && i < 63; i++)
    {
        char *expanded = expand(g_parser, g_parser->spe, argv[i]);
        if (expanded)
        {
            expanded_argv[count++] = expanded;
        }
        else
        {
            expanded_argv[count++] = strdup(argv[i]);
        }
    }
    expanded_argv[count] = NULL;
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        execvp(argv[0], argv);
        fprintf(stderr, "%s command not found\n", argv[0]);
        _exit(127);
    }

    int status;
    waitpid(pid, &status, 0);
    for (int i = 0; i < count; i++)
        free(expanded_argv[i]);
    free(expanded_argv);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 1;
}

static void exec_child(struct ast_pipeline *p, size_t i, int prev_fd,
                       int pipefd[2])
{
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
    if (prev_fd != -1)
    {
        close(prev_fd);
    }
    int child = exec_ast(p->cmds[i]);
    exit(child);
}

static int exec_pipeline_simple(struct ast_pipeline *p)
{
    if (!p || p->count == 0)
    {
        return 0;
    }
    if (p->count == 1)
    {
        struct ast *cmd = p->cmds[0];
        if (cmd->type == AST_REDIRECTION)
            return exec_redirection(cmd);
        return exec_ast(cmd);
    }
    return -1;
}

static int exec_pipeline_fork(struct ast_pipeline *p, pid_t *pids)
{
    int prev_fd = -1;
    int pipefd[2];
    for (size_t i = 0; i < p->count; i++)
    {
        if (i + 1 < p->count)
        {
            if (pipe(pipefd) < 0)
            {
                perror("pipe");
                return 0;
            }
        }
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return 0;
        }
        if (pid == 0)
        {
            exec_child(p, i, prev_fd, pipefd);
        }
        pids[i] = pid;
        if (prev_fd != -1)
        {
            close(prev_fd);
        }
        if (i + 1 < p->count)
        {
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }
    }
    return 1;
}

static int exec_pipeline_wait(pid_t *pids, size_t count)
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

static int exec_pipeline(struct ast_pipeline *p)
{
    int simple = exec_pipeline_simple(p);
    if (simple != -1)
    {
        return simple;
    }
    pid_t *pids = malloc(p->count * sizeof(pid_t));
    if (!pids)
    {
        return 1;
    }
    if (!exec_pipeline_fork(p, pids))
    {
        free(pids);
        return 1;
    }
    int status = exec_pipeline_wait(pids, p->count);
    free(pids);
    return status;
}

static int exec_negation(struct ast_negation *n)
{
    int status = exec_ast(n->child);

    if (status == 0)
        return 1;
    return 0;
}

static int exec_list(struct ast *ast)
{
    struct ast_list *list = (struct ast_list *)ast;
    int status = 0;

    for (size_t i = 0; i < list->count; i++)
        status = exec_ast(list->commands[i]);

    return status;
}

static int exec_if(struct ast *ast)
{
    struct ast_if *ifn = (struct ast_if *)ast;
    int cond = exec_ast(ifn->condition);

    if (cond == 0)
        return exec_ast(ifn->then_body);
    else if (ifn->else_body)
        return exec_ast(ifn->else_body);

    return cond;
}

static int exec_until(struct ast *ast)
{
    struct ast_until *u = (struct ast_until *)ast;
    int status = 0;
    while (1)
    {
        status = exec_ast(u->condition);
        if (status == 0)
            break;
        status = exec_ast(u->body);
        if (status == EXEC_BREAK)
        {
            status = 0;
            break;
        }
        if (status == EXEC_CONTINUE)
        {
            status = 0;
            continue;
        }
    }
    return status;
}

static int exec_while(struct ast *ast)
{
    struct ast_while *w = (struct ast_while *)ast;
    int status = 0;

    while (1)
    {
        status = exec_ast(w->condition);
        if (status != 0)
            break;
        status = exec_ast(w->body);
        if (status == EXEC_BREAK)
        {
            status = 0;
            break;
        }
        if (status == EXEC_CONTINUE)
        {
            status = 0;
            continue;
        }
    }

    return status;
}

static struct variable *get_var(struct parser *parser, const char *name)
{
    struct variable *cur = parser->var;

    while (cur)
    {
        if (strcmp(cur->nom, name) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

static char *save_before_value(struct parser *parser, const char *name,
                               int *defined)
{
    struct variable *var = get_var(parser, name);
    if (var && var->value)
    {
        *defined = 1;
        return strdup(var->value);
    }
    *defined = 0;
    return NULL;
}

static void restore_before(struct parser *parser, const char *name,
                           char *before_value, int defined)
{
    struct variable *cur = parser->var;
    struct variable *prev = NULL;
    if (defined && before_value)
    {
        char *name_copy = strdup(name);
        char *value_copy = strdup(before_value);
        add_var(parser, name_copy, value_copy);

        free(before_value);
        return;
    }
    while (cur)
    {
        if (strcmp(cur->nom, name) == 0)
        {
            if (prev)
            {
                prev->next = cur->next;
            }
            else
            {
                parser->var = cur->next;
            }
            free(cur->nom);
            free(cur->value);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static void update(struct parser *parser, const char *name, const char *value)
{
    add_var(parser, name, value);
    struct variable *var = get_var(parser, name);
    if (var)
    {
        var->exported = 0;
    }
}

static int exec_for(struct ast *ast)
{
    struct ast_for *res = (struct ast_for *)ast;
    int status = 0;
    char *before_value = NULL;
    int defined = 0;
    if (g_parser)
    {
        before_value = save_before_value(g_parser, res->var, &defined);
    }
    if (!res->words)
    {
        status = exec_ast(res->body);
    }
    else
    {
        for (size_t i = 0; res->words[i]; i++)
        {
            if (g_parser)
            {
                update(g_parser, res->var, res->words[i]);
            }
            status = exec_ast(res->body);
            if (status == EXEC_BREAK)
            {
                status = 0;
                break;
            }
            if (status == EXEC_CONTINUE)
            {
                status = 0;
                continue;
            }
        }
    }

    if (g_parser)
    {
        restore_before(g_parser, res->var, before_value, defined);
    }
    return status;
}

static int exec_and(struct ast *ast)
{
    struct ast_and_or *and = (struct ast_and_or *)ast;
    int status = exec_ast(and->left);

    if (status == 0)
        return exec_ast(and->right);
    return status;
}

static int exec_or(struct ast *ast)
{
    struct ast_and_or * or = (struct ast_and_or *)ast;
    int status = exec_ast(or->left);

    if (status != 0)
        return exec_ast(or->right);
    return status;
}
// trouver quelle fd on redirect
static int find_targ_fd(struct ast_redirection *r)
{
    int target_fd;
    if (r->redir_nb != -1)
        target_fd = r->redir_nb;
    else if (r->type == AST_REDIR_IN || r->type == AST_REDIR_DUP_IN)
        target_fd = STDIN_FILENO;
    else if (r->type == AST_REDIR_OUT || r->type == AST_REDIR_FORC_OUT
             || r->type == AST_REDIR_APP)
        target_fd = STDOUT_FILENO;
    else
        target_fd = STDERR_FILENO;
    return target_fd;
}

static void redirect(struct ast_redirection *r)
{
    int target_fd = find_targ_fd(r);
    int fd;

    fd = -1;

    if (r->type == AST_REDIR_IN)
    {
        fd = open(r->file, O_RDONLY);
        if (fd < 0)
            _exit(1);
        // remplace le terminal avec le fd quon veut (stdin pointe vers out.txt
        // pas termianl)
        dup2(fd, target_fd);
        // ferme pour pas avoir de bugs
        close(fd);
    }
    // pour linstant jai mis force et out pareil
    else if (r->type == AST_REDIR_OUT || r->type == AST_REDIR_FORC_OUT)
    {
        fd = open(r->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
            _exit(1);
        dup2(fd, target_fd);
        close(fd);
    }
    else if (r->type == AST_REDIR_APP)
    {
        fd = open(r->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0)
            _exit(1);
        dup2(fd, target_fd);
        close(fd);
    }
    else if (r->type == AST_REDIR_RW)
    {
        fd = open(r->file, O_RDWR);
        if (fd < 0)
            _exit(1);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    // trouve ou ecrire avec atoi
    else if (r->type == AST_REDIR_DUP_OUT || r->type == AST_REDIR_DUP_IN)
        dup2(atoi(r->file), target_fd);
    else
        _exit(1);
}
// pour multi redir
static struct ast *all_redirections(struct ast *ast)
{
    while (ast->type == AST_REDIRECTION)
    {
        struct ast_redirection *r = (struct ast_redirection *)ast;
        // change ou ca pointe
        redirect(r);
        // ou le faire
        ast = r->left;
    }
    return ast;
}

static int exec_redirection(struct ast *ast)
{
    // create child process
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0)
        return 1;
    // dans lenfant
    if (pid == 0)
    {
        struct ast *cmd;
        // trouve la cmd de redir ligne et changer fd en ce que tu veux
        // (out.txt)
        cmd = all_redirections(ast);
        // si cmd lexecuter
        if (cmd->type == AST_COMMAND)
        {
            struct ast_cmd *c;

            c = (struct ast_cmd *)cmd;
            execvp(c->words[0], c->words);
            _exit(127);
        }

        _exit(exec_ast(cmd));
    }
    // attendre que process enfant finis
    waitpid(pid, &status, 0);
    // exit code de lenfant si finis
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return 1;
}
static int exec_subshell(struct ast *ast)
{
    struct ast_subshell *s = (struct ast_subshell *)ast;
    // child
    pid_t pid = fork();
    if (pid < 0)
        return 1;

    if (pid == 0)
    {
        // in child
        // current status on if exit or not
        int saved_exit = g_parser->exit;
        // code from global shell
        int saved_code = g_parser->ex_code;

        // set exit to not exit immediatly
        g_parser->exit = 0;
        // execute whats inside paran or brackets
        int status = exec_ast(s->body);

        fflush(stdout);
        // restore all previous
        g_parser->exit = saved_exit;
        g_parser->ex_code = saved_code;
        // exit from child and return status to par
        _exit(status);
    }

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return 1;
}

int exec_ast(struct ast *ast)
{
    if (g_parser && g_parser->exit)
    {
        fflush(stdout);
        _exit(g_parser->ex_code);
    }
    if (!ast)
        return 0;

    switch (ast->type)
    {
    case AST_COMMAND: {
        struct ast_cmd *cmd = (struct ast_cmd *)ast;
        return exec_command(cmd->words);
    }

    case AST_LIST: {
        return exec_list(ast);
    }

    case AST_IF: {
        return exec_if(ast);
    }
    case AST_WHILE: {
        return exec_while(ast);
    }
    case AST_UNTIL: {
        return exec_until(ast);
    }
    case AST_FOR: {
        return exec_for(ast);
    }
    case AST_PIPELINE:
        return exec_pipeline((struct ast_pipeline *)ast);
    case AST_NEGATION: {
        struct ast_negation *n = (struct ast_negation *)ast;
        return exec_negation(n);
    }
    case AST_AND: {
        return exec_and(ast);
    }

    case AST_OR: {
        return exec_or(ast);
    }
    case AST_REDIRECTION: {
        return exec_redirection(ast);
    }
    case AST_SUBSHELL: {
        return exec_subshell(ast);
    }
    default:
        return 0;
    }
}

void exec_set_parser(struct parser *parser)
{
    g_parser = parser;
}
