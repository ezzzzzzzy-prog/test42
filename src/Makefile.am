bin_PROGRAMS = minishell

minishell_SOURCES = \
    minishell.c \
    ast/ast.c \
    ast/ast_create.c \
    lexer/lexer.c \
    parser/parser.c \
    exec/exec.c \
    builtin/builtin.c \
    expansion/expansion.c \
    io_backend/io_backend.c \
    special/special.c \
    debug/debug.c \
    ast/ast.h \
    ast/ast_create.h \
    lexer/lexer.h \
    parser/parser.h \
    exec/exec.h \
    builtin/builtin.h \
    expansion/expansion.h \
    io_backend/io_backend.h \
    special/special.h \
    debug/debug.h

AM_CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -Wvla
