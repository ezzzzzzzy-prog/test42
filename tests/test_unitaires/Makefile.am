AM_CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -Wvla

check_PROGRAMS = \
    test_ast_basic \
    test_ast \
    test_ast_free \
    test_lexer \
    test_parser \
    test_builtin \
    test_expansion \
    test_exec

test_ast_basic_SOURCES = \
    test_ast_basic.c \
    ../../src/ast/ast.c \
    ../../src/ast/ast_create.c

test_ast_SOURCES = \
    test_ast.c \
    ../../src/ast/ast.c \
    ../../src/ast/ast_create.c

test_ast_free_SOURCES = \
    test_ast_free.c \
    ../../src/ast/ast.c \
    ../../src/ast/ast_create.c

test_lexer_SOURCES = \
    test_lexer.c \
    ../../src/lexer/lexer.c \
    ../../src/io_backend/io_backend.c

test_parser_SOURCES = \
    test_parser.c \
    ../../src/parser/parser.c \
    ../../src/lexer/lexer.c \
    ../../src/ast/ast.c \
    ../../src/ast/ast_create.c \
    ../../src/io_backend/io_backend.c \
    ../../src/expansion/expansion.c \
    ../../src/special/special.c

test_builtin_SOURCES = \
    test_builtin.c \
    ../../src/builtin/builtin.c \
    ../../src/parser/parser.c \
    ../../src/lexer/lexer.c \
    ../../src/ast/ast.c \
    ../../src/ast/ast_create.c \
    ../../src/io_backend/io_backend.c \
    ../../src/expansion/expansion.c \
    ../../src/special/special.c

test_expansion_SOURCES = \
    test_expansion.c \
    ../../src/expansion/expansion.c \
    ../../src/parser/parser.c \
    ../../src/lexer/lexer.c \
    ../../src/ast/ast.c \
    ../../src/ast/ast_create.c \
    ../../src/io_backend/io_backend.c \
    ../../src/special/special.c

test_exec_SOURCES = \
    test_exec.c \
    ../../src/exec/exec.c \
    ../../src/parser/parser.c \
    ../../src/lexer/lexer.c \
    ../../src/ast/ast.c \
    ../../src/ast/ast_create.c \
    ../../src/io_backend/io_backend.c \
    ../../src/builtin/builtin.c \
    ../../src/expansion/expansion.c \
    ../../src/special/special.c \
    ../../src/debug/debug.c

TESTS = $(check_PROGRAMS)
