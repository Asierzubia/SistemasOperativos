#include "Estructuras.h"

#ifndef _COLA_H
#define _COLA_H

/*Estrucutra de la cola*/

typedef struct cola_pcb{ 
    pcb_t pcb; 
    struct cola_pcb *next;
} cola_pcb;


struct cola_pcb *raiz=NULL;
struct cola_pcb *fondo=NULL;

/*Funcion para comprobar que la cola esta vacia*/

int vacia()
{
    if (raiz == NULL)
        return 1;
    else
        return 0;
}

/*Funcion para insertar(push()) un pcb_t en la cola*/

void insertar(pcb_t *pcb)
{
    struct cola_pcb *nuevo;
    nuevo=malloc(sizeof(struct cola_pcb));
    nuevo->pcb=*(pcb_t *)pcb;
    nuevo->next=NULL;
    if (vacia())
    {
        raiz = nuevo;
        fondo = nuevo;
    }
    else
    {
        fondo->next = nuevo;
        fondo = nuevo;
    }
}

/*Fucnion para extraer (pop()) el primer pcb_t de la cola*/
pcb_t extraer()
{
    if (!vacia())
    {
        pcb_t informacion = raiz->pcb;
        struct cola_pcb *bor = raiz;
        if (raiz == fondo)
        {
            raiz = NULL;
            fondo = NULL;
        }
        else
        {
            raiz = raiz->next;
        }
        free(bor);
        return informacion;
    }
    else{
    	pcb_t salida;
    	salida.id=3;
    	return salida;
    }
}


#endif
