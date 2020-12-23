#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>    // srand, rand,...
#include <pthread.h>
#include <time.h>      // time
#include <sys/types.h>
#include <sys/stat.h>

void main(int argc, char *argv[]){

    long logical=0x67609A;
    printf("LA DIRECCION LOGICA INICIAL ES -----> %08lx \n",logical);
    //long logical_address=(logical &  0x000000ffUL) >> 8;
    //long offset=(logical & 0xffff00) >> 8;
    long rg = (logical & 0xFFFF00) >> 8;
    if(rg == 0x07) {
        printf("COnseguido\n");
    }
    long direccion = (logical & 0x00ffffff);
    //printf("LA DIRECCION LOGICA ES -----> %08lx \n",logical_address);
    printf("EL REGISTRO_TIPO ES -----> %06lx\n",rg);
    printf("LA DIRECCION DEL DATO ES -----> %06lx\n",direccion);

    //int log_off = 0x03A0 | 0x02;
    //printf("La direccion final es -----> %X \n",log_off);
    //long log_off = 512;
    //printf("La direccion final es -----> %08lx \n",log_off);
    long *memoria = (long*)malloc(5*sizeof(long));
    for(int i =0;i<5;i++) memoria[i] =1;
    for(int i =0;i<5;i++) printf("Valor %ld\n",memoria[i]);
    printf("%ld",memoria[7]);
    if (&memoria[6] == NULL) printf("Lo hace bien jejejeje\n");
}