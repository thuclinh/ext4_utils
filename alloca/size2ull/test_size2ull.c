/*
 * test_size2ull.c
 *
 *  Created on: 19/07/2010
 *      Author: pablo
 */

#define DEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "size2ull.h"
short check_fn(const char *str, unsigned long long expected);
short parseerror_fn(const char *str);

struct {
	char *expr;
	unsigned long long result;
} checklist[]={
		{"",0},
		{"0",0},
		{"1234567890",1234567890},
		{"c",1},
		{"9c",9},
		{"b",512},
		{"3b",512*3},
		{"k",1024},
		{"kD",1000},
		{"1k",1024},
		{"1kD",1000},
		{"2k",2048},
		{"2kD",2000},
		{"M",1024*1024},
		{"MD",1e6},
		/*a partir de aquí GCC da warnings, pero la ejecución va bien*/
		{"G",1024*1024*1024},
		{"GD",1e9},
		{"3G",3221225472},
		{"3GD",3*1e9},
		{"T",1099511627776},
		{"TD",1e12},
		{"3T",3298534883328},
		{"3TD",3*1e12},
		{"P",1125899906842624},
		{"PD",1e18},
		{"3P",3377699720527872},
		{"3PD",3*1e18},
/*		{"E",1152921504606846976},
		{"ED",1e24},
		{"3E",3458764513820540928},
		{"3ED",3e24}, //falla
*/

};
/*no es lo mismo que poner 2*1024*1024*1024 que 2147483648. Probablemente el preprocesador trabaje con long. El caso es que con la primera expresión
 * dice que desborda, con la segunda sólo dice que es unsigned en C90 y sólo en C90*/

char *parselist[]={
		"e","1e","k1"
};

short check_fn(const char *str, unsigned long long expected)
{
	unsigned long long num;
	assert(str);

	if(size2ull(&num,str) || num!=expected){
		fprintf(stderr,"check_fn test failed: \"%s\" -> %llu (expected) -> %llu (returned)\n", str, expected, num);
		return -1;
	}else
		return 0;
}

short parseerror_fn(const char *str)
{
	unsigned long long num;
	assert(str);

	if(!size2ull(&num,str)){
		fprintf(stderr,"parseerror_fn test failed: \"%s\"\n", str);
		return -1;
	}else
		return 0;
}

int main(int argc, char **argv)
{
	int i;
	unsigned short failed=0;

	for(i=0;i<sizeof(checklist)/sizeof(checklist[0]);i++)
		if(-1==check_fn(checklist[i].expr,checklist[i].result))
			failed=1;

	for(i=0;i<sizeof(parselist)/sizeof(parselist[0]);i++)
		if(-1==parseerror_fn(parselist[i]))
			failed=1;

	if(!failed){
		printf("All tests passed!\n");
		exit(EXIT_SUCCESS);
	}else{
		exit(EXIT_FAILURE);
	}
}
