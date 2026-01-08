#define _POSIX_C_SOURCE 200809L
#include "io_backend.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static FILE *f = NULL;

int io_backend_init(int argc, char **argv)
{
	if(argc == 1)
	{
		f = stdin;
		return 0;
	}
	if(argc >= 3 && strcmp(argv[1], "-c") == 0)
	{
		char *script = argv[2];
		f = fmemopen(script, strlen(script), "r");
		return f ? 0 : -1;
	}
	f = fopen(argv[1] , "r");
	if(!f)
	{
		perror("42sh");
		return -1;
	}
	return 0;
}

int io_backend_next(void)
{
	if(!f)
	{
		return EOF;
	}
	int res = fgetc(f);
	return res;
}

int io_backend_peek(void)
{
	if(!f)
	{
		return EOF;
	}
	int res = fgetc(f);
	if(!res)
	{
		ungetc(res,f);
	}
	return res;
}

void io_backend_close(void)
{
	if(f && f != stdin)
	{
		fclose(f);
	}
	f = NULL;
}

