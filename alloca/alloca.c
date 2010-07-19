/*
 * alloc.c
 *
 *  Created on: 18/07/2010
 *      Author: pablo
 */

/* FIXes
 * en caso de error, cerrar el fichero antes de salir
 * en caso de error, ¿borrar el fichero antes de salir?
 * trapear SIGs?
 * leer argumentos de la línea de órdenes: {-h, -s <size>, -q, -p <permisos> } [lista nombres...]
 *
 * usar fallocate (linux specific) o posix_fallocate?? Con las .h y fallocate hay algún problemilla
 * con posix_fallocate hay fragmentación (9 extents para 10M, 280 para 1G), y tarda un rato
 * la implementación de ext3 siempre reserva 0
 * el argumento a fallocate(), si es FALLOC_FL_KEEP_SIZE mantiene el tamaño del fichero, y si es diferente puede agrandarlo de ser necesario. Es poco elegante esta implementación
 * con fallocate() sobre ext4, es instantáneo y sólo usa 10 extents para 1G
 * las md5 de los tres métodos son idénticas. Métodos: dd, posix_fallocate, fallocate
 * posix_fallocate funciona bien sobre ext4: instantáneo, sólo 10 extents y reserva 1G
 * */

#define DEBUG
//#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//#include <linux/falloc.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "size2ull/size2ull.h"

typedef struct {
	short verb;
	char *file;
	unsigned long long size;
	mode_t perm;
} config_t;
enum {VERB_QUIET=-1, VERB_NORMAL, VERB_VERB, VERB_MAX};

int parseperm(mode_t *perm, const char *string);
int alloca_file(const config_t *cfg);
int parse_args(int argc, char **argv, config_t *cfg);

int parseperm(mode_t *perm, const char *string)
{
	int i=0;
	mode_t tmp=0;
	assert(perm && string);

	while(*string){
		if(4==i || !isdigit(*string) || *string>'7')
			return -1;

		tmp<<=3;
		tmp|=*string-'0';

		string++;
		i++;
	}
	if(i<3)
		return -1;

	*perm=tmp;
	return 0;
}

/*
 * Reads argv and argc, and stores configuration by reference in cfg: verbosity, file size and permissions.
 * Return 0 if everythung is ok, exit() if not.
 */
int parse_args(int argc, char **argv, config_t *cfg)
{
	char ch;
	unsigned long long sz;
	mode_t perm;

	assert(cfg && argv);

	cfg->verb=0;
	cfg->size=0;
	cfg->file=NULL;
	parseperm(&cfg->perm,"600");

	while(-1!=(ch=getopt(argc, argv, "hqvs:p:"))){
		switch(ch){
		case 'q':
			if(cfg->verb > 0){
				fprintf(stderr,"Bad arguments: -v and -q are not compatible\n");
				exit(EXIT_FAILURE);
			}
			cfg->verb=-1;
			break;
		case 'v':
			if(cfg->verb < 0){
				fprintf(stderr,"Bad arguments: -v and -q are not compatible\n");
				exit(EXIT_FAILURE);
			}
			cfg->verb++;
			if(cfg->verb > VERB_MAX)
				cfg->verb--;
			break;
		case 's':
			if(-1==size2ull(&sz, optarg)){
				fprintf(stderr,"Bad arguments: size is bogus: \"%s\"\n", optarg);
				exit(EXIT_FAILURE);
			}
			if(!sz){
				fprintf(stderr,"Bad arguments: size must be greater than zero\n");
				exit(EXIT_FAILURE);
			}
			cfg->size=sz;
			break;
		case 'p':
			if(-1==parseperm(&perm, optarg)){
				fprintf(stderr,"Bad arguments: creation mode is bogus: \"%s\"\n", optarg);
				exit(EXIT_FAILURE);
			}
			cfg->perm=perm;
			break;
		case 'h':
			printf("texto de ayuda\n"); //FIXME
			exit(EXIT_SUCCESS);
			break;
		case '?':
			fprintf(stderr,"usa la ayuda\n");//FIXME
			exit(EXIT_FAILURE);
			break;
		}
	}

	return optind;
}

/*
 * Receives parameters by reference with cfg: permissions, size, filename and verbosity
 * Creates cfg->file with cfg->perm permissions and allocates cfg->size bytes.
 *
 * Return 0 if everything is ok, -1 if not. Prints error/success messages depending on cfg->verb
 */
int alloca_file(const config_t *cfg)
{
	int fd;

	assert(cfg);

	fd=open(cfg->file,O_CREAT|O_EXCL|O_WRONLY,cfg->perm);
	if(-1==fd){
		if(cfg->verb >= VERB_QUIET) perror("alloca_file():open(): error creating file");
		return -1;
	}

	errno=posix_fallocate(fd,0,cfg->size);
	if(errno){
		if(cfg->verb >= VERB_QUIET) perror("alloca_file():posix_fallocate(): error asking for space");
		return -1;
	}

	if(-1==close(fd)){
		if(cfg->verb > VERB_QUIET) perror("alloca_file():close()");
		return -1;
	}

	if(cfg->verb>VERB_QUIET)
		printf("%s: allocated successfully.\n", cfg->file);

	return 0;
}


int main(int argc, char **argv)
{
	int i;
	config_t config;

	/* take options and validate them */
	i=parse_args(argc, argv, &config);
	if(argc==i){
		fprintf(stderr,"Bad arguments: no files given\n");
		exit(EXIT_FAILURE);
	}
	if(!config.size){
		fprintf(stderr,"Bad arguments: size (-s) is mandatory\n");
		exit(EXIT_FAILURE);
	}

	/* for each asked file, allocate it */
	for(;i<argc;i++){
		config.file=argv[i];
		if(-1==alloca_file(&config)){
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}
