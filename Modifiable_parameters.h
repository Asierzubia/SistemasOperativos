#ifndef _VARIABLES_H
#define _VARIABLES_H
#include "Estructuras.h"


/*Parametros realaciones con el la frecuencia del Clock, del timer y del Generador de procesos*/

//-------------------------------------------------------

#define Clock_Frequency 3
#define Timer_cycle 9
#define PG_Frequency 10

//-------------------------------------------------------

/*Es el numero maximo de prioridades. Si es 4 entonces solo habra cuatro prioridades(0,1,2,3).Las prioridades
en mi caso son decrecientes. Es decir, la 0 es la prioridad mas superior y la 3 la mas inferior.*/

#define PRIORIDAD_MAX 4

/*Es el tiempo durante el cual se va a ejecutar el programa.El valor 1 indica 1 segundo.*/

#define WAITING_TO_EXIT 1

/*Numero de procesos maximos son los procesos que se almacenan en la
lista de espera de procesos mas los procesos de todos los cores de todas las cpus*/

#define MAX_PROCESS 200

/*Numero de cpus*/

#define MAX_CPUS 2

/*Numero de procesos de cada CORE*/

#define CORE_THREADS 5

/*Numero de cores en cada CPU*/

#define CPU_CORES 3

/*Variable para definir los procesos muertos*/

#define PROCESO_MUERTO -200

/*Variables relacionadas con la memoria*/

#define size 256 /*indica el numero de marcos*/

#define frameFree -1 /*Parametros que indica si un marco esta libre*/
/*Mover a main*/

#endif