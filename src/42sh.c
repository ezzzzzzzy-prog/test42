#include <stdlib.h>
#include "io_backend.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	if(io_backend_init(argc,argv) < 0)
	{
		return 1;
	}
	int c;
	while ((c = io_backend_next()) != EOF)
	{
		putchar(c);
	}
	io_backend_close();
    return 0; 
}

