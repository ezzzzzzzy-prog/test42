check_PROGRAMS = \
    test_ast_basic \
    test_ast \
    test_ast_free \
    test_lexer \
    test_parser

AM_CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic

test_parser_SOURCES = \
    test_parser.c \
    ../../src/parser.c \
    ../../src/lexer.c \
    ../../src/ast.c \
    ../../src/io_backend.c

test_lexer_SOURCES = \
    test_lexer.c \
    ../../src/lexer.c

test_AST_SOURCES = \
    test_AST.c \
    ../../src/ast.c

test_builtin_SOURCES = \
    test_builtin.c \
    ../../src/builtin.c

test_exec_SOURCES = \
    test_exec.c \
    ../../src/exec.c

test_expansion_SOURCES = \
    test_expansion.c \
    ../../src/expansion.c
