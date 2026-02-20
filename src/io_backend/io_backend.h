#ifndef IO_BACKEND_H
#define IO_BACKEND_H

/*
 * Initialise le backend d'entree selon les arguments du shell :
 *   - argc == 1         : lecture sur stdin
 *   - argv[1] == "-c"   : lecture depuis argv[2] (chaine)
 *   - sinon             : lecture depuis le fichier argv[1]
 * Retourne 0 en cas de succes, -1 en cas d'erreur.
 */
int io_backend_init(int argc, char **argv);

/* Lit et retourne le prochain caractere (ou EOF) */
int io_backend_next(void);

/* Retourne le prochain caractere sans le consommer (ou EOF) */
int io_backend_peek(void);

/* Ferme le backend (ne ferme pas stdin) */
void io_backend_close(void);

#endif /* IO_BACKEND_H */
