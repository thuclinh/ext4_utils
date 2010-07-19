/*
 * size2ull.c
 *
 *  Created on: 19/07/2010
 *      Author: pablo
 */

#define DEBUG
#include <assert.h>
#include <ctype.h>

//#include <stdio.h>

#include "size2ull.h"

/*
 * Aún no implementados: E, Z, Y. E no daba problemas, ED sí
 * La opción x tampoco está implementada
 * La opción w no se va a implementar, por problemas de compatibilidad
 * T y P dan warnings debido al tamaño. Usando C99 (-std=c99) se evitan
 **/

/*
 * De man dd:
 * Las opciones de más abajo con valores numéricos (bytes y bloques) pueden ir seguidas por un factor multiplicador: `k'=1024, `b'=512, `w'=2, `c'=1 (`w' y `c' son extensiones de GNU;
       `w' nunca debería utilizarse: significa 2 en System V y 4 en 4.2BSD).  Dos o más de tales expresiones numéricas pueden multiplicarse poniendo una `x' (equis minúscula) entre ellas.
       La  versión  fileutils-4.0 de GNU también permite los siguientes sufijos multiplicativos al especificar tamaños de bloque (en bs=, cbs=, ibs=, obs=): M=1048576, G=1073741824, y así
       para T, P, E, Z, Y. Un sufijo `D' los convierte en decimal: kD=1000, MD=1000000, GD=1000000000, etc.  (Dese cuenta que para ls, df, du, el tamaño de M, etc., viene determinado  por
       variables de entorno, pero para dd es fijo.)
 *
 * 123MD:
 * MD equivale a pone 1000000, o 1MD
 * */


int size2ull(unsigned long long *res, const char *string)
{
	unsigned long long sum=0, mult=0;
	unsigned short st=0;
	assert(res && string);

	while(1){
		switch(st){
		/* initial state*/
		case 0:
			switch(*string){
			case 0:
				st=50;
				string--;
				break;
			case 'c':
				mult=1;
				st=50;
				break;
			case 'b':
				mult=512;
				st=50;
				break;
			case 'k':
				if('D'==*(string+1)) {
					mult=1000;
					string++;
				}else
					mult=1024;
				st=50;
				break;
			case 'M':
				if('D'==*(string+1)) {
					mult=1e6;
					string++;
				}else
					mult=1024*1024;
				st=50;
				break;
			case 'G':
				if('D'==*(string+1)) {
					mult=1e9;
					string++;
				}else
					mult=1024*1024*1024;
				st=50;
				break;
			case 'T':
				if('D'==*(string+1)) {
					mult=1e12;
					string++;
				}else
					mult=1099511627776;
				st=50;
				break;
			case 'P':
				if('D'==*(string+1)) {
					mult=1e18;
					string++;
				}else
					mult=1125899906842624;
				st=50;
				break;
/*			case 'E':
				if('D'==*(string+1)) {
					mult=1e24;
					string++;
				}else
					mult=1152921504606846976;
				st=50;
				break;
			case 'Z':
				if('D'==*(string+1)) {
					mult=1e30;
					string++;
				}else
					mult=1024*1024*1024*1024*1024*1024*1024;
				st=50;
				break;
			case 'Y':
				if('D'==*(string+1)) {
					mult=1e36;
					string++;
				}else
					mult=1024*1024*1024*1024*1024*1024*1024*1024;
				st=50;
				break;
*/
			default:
				if(isdigit(*string)){
					sum*=10;
					sum+=*string-'0';
				}else
					return -1;
			}
			break;
		/* final state */
		case 50:
			switch(*string){
			case 0:
//				printf("%d,%d\n",sum,mult);
				if(!sum) /*M, kD, T*/
					if(!mult)
						*res=0; /* 0, ""*/
					else
						*res=mult;
				else
					if(!mult) /*1234567890*/
						*res=sum;
					else /*3M*/
						*res=sum*mult;
				return 0;
				break;
			default:
				return -1;
			}
			break;
		}
		string++;
	}

	return 0;
}
