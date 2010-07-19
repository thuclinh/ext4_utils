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

#include "size2ull.h"

typedef struct {
	short verb;
	char *file;
	unsigned long long size;
	mode_t perm;
} config_t;
#define VERB_MAX 1


int parse_args(int argc, char **argv, config_t *cfg)
{
	char ch;
	unsigned long long sz;

	assert(cfg && argv);

	cfg->verb=0;
	cfg->size=0;
	cfg->file=NULL;
	cfg->perm=0;

	while(-1!=(ch=getopt(argc, argv, "hqvs:"))){
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
			printf("%s\n", optarg);
			if(-1==size2ull(&sz, optarg)){
				fprintf(stderr,"Bad arguments: size is bogus: \"%s\"\n", optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			printf("texto de ayuda\n"); //FIX
			exit(EXIT_SUCCESS);
			break;
		case '?':
			fprintf(stderr,"usa la ayuda\n");//FIX
			exit(EXIT_FAILURE);
			break;
		}
	}

	return optind;

	return 0;
}

int main(int argc, char **argv)
{
	char *file=argv[1];
	off_t len=1024*1024*1024;
	int fd;
	int ret;
	config_t config;

	parse_args(argc, argv, &config);

	printf("config.verb = %d\n",config.verb);

	exit(EXIT_SUCCESS);

	fd=open(file,O_CREAT|O_EXCL|O_WRONLY,S_IREAD|S_IWRITE);
	if(-1==fd){
		perror("Error al crear el fichero");
		exit(EXIT_FAILURE);
	}

	ret=posix_fallocate(fd,0,len);
	if(ret){
		errno=ret;
		perror("Error al reservar espacio para el fichero");
		exit(EXIT_FAILURE);
	}


	/*ret=fallocate(fd,0,0,len);
	if(-1==ret){
		perror("Error al reservar espacio para el fichero");
		exit(EXIT_FAILURE);
	}
	*/

	if(-1==close(fd)){
		perror("Error al cerrar el fichero");
		exit(EXIT_FAILURE);
	}

	printf("%s: fichero creado con éxito\n", file);

}
