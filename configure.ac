# Init the minishell project
AC_INIT([minishell], [1.0], [minishell@assistants.epita.fr])

# Setup Automake
AM_INIT_AUTOMAKE([subdir-objects] [foreign])

# Pretty display of Makefile rules
AM_SILENT_RULES([yes])

# Enable ar for Makefile
AM_PROG_AR

# Check if ranlib is available
AC_PROG_RANLIB

# Check if a C compiler is available
AC_PROG_CC

# Check if a compiler has this list of flags
#AX_COMPILER_FLAGS([], [], [], [-std=c99 -pedantic -Werror -Wall -Wextra -Wvla])

# List Makefiles in subdirectories
AC_CONFIG_FILES([
                    Makefile
                    src/Makefile
                    tests/Makefile
                    tests/test_unitaires/Makefile
                    ])

AC_OUTPUT
