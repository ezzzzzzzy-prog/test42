#include "exec.h"
#include "ast.h"
#include "builtin.h"

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h> 


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

static int exec_while(struct ast_while *w)
{
    int status = 0;
    while (1)
    {
        int cond = exec_ast(w->condition);
        if (cond != 0)
            break;
        status = exec_ast(w->body);
    }
    return status;
}

static int exec_until(struct ast_until *u)
{
    int status = 0;
    while (1)
    {
        int cond = exec_ast(u->condition);
        if (cond == 0)
            break;
        status = exec_ast(u->body);
    }
    return status;
}

static int exec_for(struct ast_for *f)
{
    int status = 0;
    if (!f->words || !f->words[0])
        return exec_ast(f->body);
    for (size_t i = 0; f->words[i]; i++)
    {
        status = exec_ast(f->body);
    }
    return sta
static int exec_pipeline(struct ast_pipeline *p)
{
        if(!p || p->count == 0)
        {
                return 0;
        }
        if(p->count == 1)
        {
                return exec_ast(p->cmds[0]);
        }
    int prev_fd = -1;
    //int status = 0;
    pid_t *pids = malloc(p->count * sizeof(pid_t));
    if (!pids)
        return 1;
    int pipefd[2];
    for (size_t i = 0; i < p->count; i++)
    {
        //int pipefd[2] = { -1, -1 };

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
            if( i + 1 < p->count)
            {
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);
                    close(pipefd[0]);
            }
            if(prev_fd != -1)
            {
                    close(prev_fd);
            }

            /*if (pipefd[1] != -1)
            {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            if (pipefd[0] != -1)
                close(pipefd[0]);
             */
            /*struct ast_cmd *cmd = (struct ast_cmd *)p->cmds[i];
            execvp(cmd->words[0], cmd->words);
            perror(cmd->words[0]);
            _exit(127);*/
            int child = exec_ast(p->cmds[i]);
            exit(child);
        }

        pids[i] = pid;

        if (prev_fd != -1)
            close(prev_fd);

        if (i +1 < p->count)
        {
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }
    }
    int status = 0;
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
static int exec_negation(struct ast_negation *n)
{
    int status = exec_ast(n->child);

    if (status == 0)
        return 1;
    return 0;
}
      /*   if (!parser || !parser->curr_tok)
       return NULL;
    parser->curr_tok = pop(parser->lex);
    struct parser if () int fd = open(parser->curr_tok, O_CREATE | O_WRONLY);
    int new_stdout = dup(STDOUT_FILENO);
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }
    close(fd);
    // la je dois executer la commande dans le stdout
    if (dup2(new_stdout, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
	}
    close(new_stdout);
*/
static int redir_out(struct ast_redirection *redir, int *new_stdout)
{
    *new_stdout = dup(STDOUT_FILENO);
    if (*new_stdout < 0)
        return 1;
    int fd = open(redir->file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
        return 1;
    if(dup2(fd, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }
/*    if (dup2(new_stdout, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
	}*/
    close(fd);
    return 0;
}
static int redir_in(struct ast_redirection *redir, int *new_stdin)
{
    *new_stdin = dup(STDIN_FILENO);
    if (*new_stdin < 0)
        return 1;

    int fd = open(redir->file, O_RDONLY);
    if (fd < 0)
        return 1;

    if(dup2(fd, STDIN_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }
    close(fd);
    return 0;
}
static int redir_app(struct ast_redirection *redir, int *new_stdout)
{
    *new_stdout = dup(STDOUT_FILENO);
    if (*new_stdout < 0)
        return 1;

    int fd = open(redir->file, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd < 0)
        return 1;
    if(dup2(fd, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }

    close(fd);
    return 0;
}
static int redir_rw(struct ast_redirection *redir, int *new_stdout, int *new_stdin)
{
    *new_stdout = dup(STDOUT_FILENO);
    *new_stdin = dup(STDIN_FILENO);
    if (*new_stdout < 0 || *new_stdin < 0)
        return 1;

    int fd = open(redir->file, O_CREAT | O_RDWR, 0644);
    if (fd < 0)
        return 1;
    if(dup2(fd, STDIN_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }
    if(dup2(fd, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }

    close(fd);
    return 0;
}
static int redir_force_out(struct ast_redirection *redir, int *new_stdout)
{
    *new_stdout = dup(STDOUT_FILENO);
    if (*new_stdout < 0)
        return 1;

    int fd = open(redir->file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
        return 1;
    if(dup2(fd, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }

    close(fd);
    return 0;
}
static int redir_dup_out(struct ast_redirection *redir, int *new_stdout,int *new_stderr)
{
    *new_stdout = dup(STDOUT_FILENO);
    *new_stderr = dup(STDERR_FILENO);
    if (*new_stdout < 0)
        return 1;

    int fd = open(redir->file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
        return 1;

    if(dup2(fd, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }
    if(dup2(fd, STDERR_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }
    close(fd);
    return 0;
}
/*static int redir_dup_in(struct ast_redirection *redir, int *new_stdin)
{
    *new_stdin = dup(STDIN_FILENO);
    if (*new_stdout < 0)
        return 1;

    int fd = open(redir->file, O_RDONLY);
    if (fd < 0)
        return 1;
    if(dup2(fd, STDIN_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }

    close(fd);
    return 0;
}*/
static int exec_redirection(struct ast_redirection *redir)
{

    int new_stdin  = -1;
    int new_stdout = -1;
    int new_stderr = -1;
    int stat = 0;

    if (redir->type == AST_REDIR_OUT)
        stat = redir_out(redir, &new_stdout);
    else if (redir->type == AST_REDIR_IN)
        stat = redir_in(redir, &new_stdin);
    else if (redir->type == AST_REDIR_APP)
        stat = redir_app(redir, &new_stdout);
    else if (redir->type == AST_REDIR_RW)
        stat = redir_rw(redir, &new_stdout, &new_stdin);
    else if (redir->type == AST_REDIR_FORC_OUT)
        stat = redir_force_out(redir, &new_stdout);
    else if (redir->type == AST_REDIR_DUP_OUT)
        stat = redir_dup_out(redir, &new_stdout, &new_stderr);
    else
        return 1;

    if (stat != 0)
        return stat;
    stat = exec_ast(redir->left);
    if (new_stdin != -1)
    {
        dup2(new_stdin, STDIN_FILENO);
        close(new_stdin);
    }
    if (new_stdout != -1)
    {
        dup2(new_stdout, STDOUT_FILENO);
        close(new_stdout);
    }

    if (new_stderr != -1)
    {
        dup2(new_stderr, STDERR_FILENO);
        close(new_stderr);
    }
    return stat;
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
	case AST_WHILE:
        {
                printf("DEBUG: Executing WHILE\n");
                struct ast_while *w = (struct ast_while *)ast;
                return exec_while(w);
        }
        case AST_UNTIL:
        {
                printf("DEBUG: Executing UNTIL\n");
                struct ast_until *u = (struct ast_until *)ast;
                return exec_until(u);
        }
        case AST_FOR:
        {
                struct ast_for *f = (struct ast_for *)ast;
            return exec_for(f);
        }
        case AST_PIPELINE:
            return exec_pipeline((struct ast_pipeline *)ast);
        case AST_NEGATION:
        {
            struct ast_negation *n = (struct ast_negation *)ast;
            return exec_negation(n);
        }
        case AST_AND:
        {
            struct ast_and_or *and = (struct ast_and_or *)ast;
            int status = exec_ast(and->left);

            if (status == 0)
                return exec_ast(and->right);
            return status;
        }

        case AST_OR:
        {
            struct ast_and_or *or = (struct ast_and_or *)ast;
            int status = exec_ast(or->left);

            if (status != 0)
                return exec_ast(or->right);
            return status;
        }
        case AST_REDIRECTION:
        {
            int status = exec_redirection((struct ast_redirection *)ast);
            return status;
        }
        default:
            return 0;
    }
}
