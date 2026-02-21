#ifndef IO_BACKEND_H
#define IO_BACKEND_H

// Init le backend (stdin, -c chaine ou fichier) selon argc/argv
//  0 si ok, -1 si err
int io_backend_init(int argc, char **argv);

// Lit et renvoie le prochain char (ou EOF)
int io_backend_next(void);

// Peek : voit le prochain char sans le consommer (ou EOF)
int io_backend_peek(void);

// Ferme le backend (sauf si c est stdin)
void io_backend_close(void);

#endif /* IO_BACKEND_H */
