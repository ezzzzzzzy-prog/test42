#include "exec.h"
#include "ast.h"
#include "builtin.h"

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

static int exec_command(char **argv)
{
    if (!argv || !argv[0])
        return 0;

    if (is_builtin(argv[0]))
        return execute_builtin(argv);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        execvp(argv[0], argv);
        perror(argv[0]);
        _exit(127);
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 1;
}
static int exec_pipeline(struct ast_pipeline *p)
{
    int prev_fd = -1;
    int status = 0;
    pid_t *pids = calloc(p->count, sizeof(pid_t));
    if (!pids)
        return 1;

    for (size_t i = 0; i < p->count; i++)
    {
        int pipefd[2] = { -1, -1 };

        if (i + 1 < p->count)
        {
            if (pipe(pipefd) < 0)
            {
                perror("pipe");
                free(pids);
                return 1;
            }
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            free(pids);
            return 1;
        }

        if (pid == 0)
        {
            if (prev_fd != -1)
            {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            if (pipefd[1] != -1)
            {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            if (pipefd[0] != -1)
                close(pipefd[0]);

            struct ast_cmd *cmd = (struct ast_cmd *)p->cmds[i];
            execvp(cmd->words[0], cmd->words);
            perror(cmd->words[0]);
            _exit(127);
        }

        pids[i] = pid;

        if (prev_fd != -1)
            close(prev_fd);

        if (pipefd[1] != -1)
            close(pipefd[1]);

        prev_fd = pipefd[0];
    }

    for (size_t i = 0; i < p->count; i++)
    {
        int wstatus;
        waitpid(pids[i], &wstatus, 0);
        if (i + 1 == p->count && WIFEXITED(wstatus))
            status = WEXITSTATUS(wstatus);
    }

    free(pids);
    return status;
}

int exec_ast(struct ast *ast)
{
    if (!ast)
        return 0;

    switch (ast->type)
    {
        case AST_COMMAND:
        {
            struct ast_cmd *cmd = (struct ast_cmd *)ast;
            return exec_command(cmd->words);
        }

        case AST_LIST:
        {
            struct ast_list *list = (struct ast_list *)ast;
            int status = 0;

            for (size_t i = 0; i < list->count; i++)
                status = exec_ast(list->commands[i]);

            return status;
        }

        case AST_IF:
        {
            struct ast_if *ifn = (struct ast_if *)ast;
            int cond = exec_ast(ifn->condition);

            if (cond == 0)
                return exec_ast(ifn->then_body);
            else if (ifn->else_body)
                return exec_ast(ifn->else_body);

            return cond;
        }
        case AST_PIPELINE:
            return exec_pipeline((struct ast_pipeline *)ast);

        default:
            return 0;
    }
}
