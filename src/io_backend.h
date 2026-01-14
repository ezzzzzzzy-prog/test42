#ifndef IO_BACKEND_H
#define IO_BACKEND_H

#include <stdio.h>
int io_backend_init(int argc, char **argv);
int io_backend_next(void);
int io_backend_peek(void);
void io_backend_close(void);

#endif /* IO_BACKEND_H */
