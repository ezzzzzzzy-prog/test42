#define _POSIX_C_SOURCE 200809L
#include "exec.h"
#include "ast.h"
#include "builtin.h"
#include "parser.h"
#include "expansion.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h> 
#include <string.h>

struct parser *g_parser = NULL;

static int exec_command(char **argv)
{
    if (!argv || !argv[0])
        return 0;

    if (is_builtin(argv[0]))
    {
        return execute_builtin(argv,g_parser);
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
    
    //pid_t pid = fork();

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
     for (int i = 0; i < count; i++)
        free(expanded_argv[i]);
    free(expanded_argv);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 1;
}

static void exec_child(struct ast_pipeline *p, size_t i, int prev_fd, int pipefd[2])
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
            int child = exec_ast(p->cmds[i]);
            exit(child);
}


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
		exec_child(p,i,prev_fd, pipefd);

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

/*static int exec_while(struct ast *ast)
{
	 printf("DEBUG: exec_while called\n");
	struct ast_while *w = (struct ast_while *)ast;
        //      struct ast_while *w = (struct ast_while *)ast;
                int status = 0;
                while (1)
                {
			printf("DEBUG: while condition returned %d\n", status);
                        status = exec_ast(w->condition);
                        if (status != 0)
                                break;
                        status = exec_ast(w->body);
			printf("DEBUG: while body returned %d\n", status);
                }
                return status;
}*/
static int exec_while(struct ast *ast)
{
	  if (!ast) return 0;
    struct ast_while *w = (struct ast_while *)ast;
      if (!w || !w->condition || !w->body)
        return 0;

    int cond_status = 0;
    int body_status = 0;

    while (1)
    {
        cond_status = exec_ast(w->condition);  // Évalue la condition
        if (cond_status != 0)                  // Si condition échoue, break
            break;
        body_status = exec_ast(w->body);       // Évalue le corps
    }
    return body_status;  // Retourner le dernier code de retour du corps
}

/*static int exec_until(struct ast *ast)
{
	struct ast_until *u = (struct ast_until *)ast;
                int status = 0;
                while (1)
                {
                        status = exec_ast(u->condition);
                        if (status == 0)
                                break;
                        status = exec_ast(u->body);
                }
                return status;
}*/
static int exec_until(struct ast *ast)
{
    struct ast_until *u = (struct ast_until *)ast;
    int cond_status = 0;
    int body_status = 0;
    
    while (1)
    {
        cond_status = exec_ast(u->condition);
        if (cond_status == 0)  // until s'arrête quand condition réussit
            break;
        body_status = exec_ast(u->body);
    }
    return body_status;
}


/*static int exec_for(struct ast *ast)
{
	struct ast_for *f = (struct ast_for *)ast;
            int status = 0;
           if (!f->words)
                   return 0;
           for (size_t i = 0; f->words[i] != NULL; i++)
           {
                if (g_parser)
                    add_var(g_parser, f->var, f->words[i]);
                status = exec_ast(f->body);
	   }
	   return status;
}*/



static int exec_for(struct ast *ast)
{
    struct ast_for *f = (struct ast_for *)ast;
    int status = 0;
    if (!f->words)
    {
        status = exec_ast(f->body);
        return status;
    }
    for (size_t i = 0; f->words[i] != NULL; i++)
    {
        if (g_parser)
	{
            add_var(g_parser, f->var, f->words[i]);
	   /* struct variable *v = g_parser->var;
            while (v)
            {
                printf("  - %s=%s\n", v->nom, v->value);
                v = v->next;
            }*/
	}

        status = exec_ast(f->body);
    }

    return status;
}
/*static int exec_for(struct ast *ast, struct parser *parser)
{
    struct ast_for *f = (struct ast_for *)ast;
    int status = 0;
    
    if (!f->words)
        return 0;
    
    for (size_t i = 0; f->words[i] != NULL; i++)
    {
        // Assigner la variable pour cette itération
	if (g_parser)
            add_var(g_parser, f->var, f->words[i]);*/
       /* if (parser)
            add_var(parser, f->var, f->words[i]);
        
        status = exec_ast(f->body);
    }
    
    return status;
}*/

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
	struct ast_and_or *or = (struct ast_and_or *)ast;
            int status = exec_ast(or->left);

            if (status != 0)
                return exec_ast(or->right);
            return status;
}

/*static int exec_redirection(struct ast *ast)
{
	struct ast_redirection *r = (struct ast_redirection *) ast;
                int fd = -1;
                int tar_fd = (r->type == AST_REDIR_IN) ? 0 : 1;
                int sav_fd = dup(tar_fd);
                if (r->type == AST_REDIR_IN)
                        fd = open(r->file, O_RDONLY);
                else if (r->type == AST_REDIR_OUT)
                        fd = open(r->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                else if (r->type == AST_REDIR_APP)
                        fd = open(r->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd < 0)
                {
                        if (sav_fd != -1)
                                close(sav_fd);
                        return 1;
                }
                dup2(fd, tar_fd);
                close(fd);
                int status = 0;
                if (r->left)
                        status = exec_ast(r->left);
                if (sav_fd != -1)
                {
                        dup2(sav_fd, tar_fd);
                        close(sav_fd);
                }
                return status;
}*/

static void apply_one_redirection(struct ast_redirection *r)
{
    int tar_fd;
    if (r->file_desc != -1)
        tar_fd = r->file_desc;
    else if (r->type == AST_REDIR_IN || r->type == AST_REDIR_DUP_IN)
        tar_fd = STDIN_FILENO;
    else if (r->type == AST_REDIR_OUT || r->type == AST_REDIR_FORC_OUT || r->type == AST_REDIR_APP)
        tar_fd = STDOUT_FILENO;
    else
        tar_fd = STDERR_FILENO;

    int fd = -1;

    switch (r->type)
    {
        case AST_REDIR_IN:
            fd = open(r->file, O_RDONLY);
            break;

        case AST_REDIR_OUT:
        case AST_REDIR_FORC_OUT:
            fd = open(r->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
                _exit(1);

            dup2(fd, tar_fd);
            close(fd);
            return;

        case AST_REDIR_APP:
            fd = open(r->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            break;

        case AST_REDIR_RW:
            fd = open(r->file, O_RDWR);
            if (fd < 0)
                _exit(1);
            dup2(fd, STDIN_FILENO);
            close(fd);
            return;

        case AST_REDIR_DUP_OUT:
        case AST_REDIR_DUP_IN:
            dup2(atoi(r->file), tar_fd);
            return;

        default:
            _exit(1);
    }

    if (fd < 0)
        _exit(1);

    dup2(fd, tar_fd);
    close(fd);
}

static struct ast *apply_redirections(struct ast *ast)
{
    while (ast->type == AST_REDIRECTION)
    {
        struct ast_redirection *r = (struct ast_redirection *)ast;
        apply_one_redirection(r);
        ast = r->left;
    }
    return ast;
}
static int exec_redirection(struct ast *ast)
{
    pid_t pid = fork();
    if (pid < 0)
        return 1;

    if (pid == 0)
    {
        struct ast *cmd = apply_redirections(ast);
        if (cmd->type == AST_COMMAND)
        {
            struct ast_cmd *c = (struct ast_cmd *)cmd;
            execvp(c->words[0], c->words);
            _exit(127);
        }

        _exit(exec_ast(cmd));
    }

    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
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
		return exec_list(ast);
        }

        case AST_IF:
        {
		return exec_if(ast);
        }
	case AST_WHILE:
        {
		return exec_while(ast);

        }
        case AST_UNTIL:
        {
		return exec_until(ast);
        }
        case AST_FOR:
        {
		return exec_for(ast);
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
		return exec_and(ast);
        }

        case AST_OR:
        {
		return exec_or(ast);
        }
        case AST_REDIRECTION:
        {
		return exec_redirection(ast);
        }
        default:
            return 0;
    }
}




void exec_set_parser(struct parser *parser)
{
    g_parser = parser;
}
