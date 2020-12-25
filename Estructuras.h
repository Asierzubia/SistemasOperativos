#ifndef _ESTRUCTURAS_H
#define _ESTRUCTURAS_H
#include "Modifiable_parameters.h"

/*Estructura mm*/

typedef struct mm
{
    int num_frames; /*Guardo el numero de marcos*/
    long code; /*Guardo la direccion de inicio del code*/
    long data; /*Guardo la direccion de inicio del data*/
    long * pgb; /*Tabla de paginas*/
} mm_t;

/*Estructura para guardar el estado del pcb*/
typedef struct pcb_status {
    
    long IR; /*La instruccion en la que me he quedado*/
    long PC; /*La direccion virtual de la instruccion IR*/
    long Rlist[16]; /*Los 16 registros*/

} pcb_status_t;

/*Estructura pcb*/
typedef struct n_pcb
{
    int id; /*Identificador del pcb*/
    pthread_t hilo; /*Mantengo este atributo de la estructura para poder lanzar los hilos al principio, ya que no hago uso del mismo en ningun otro momento en el codigo*/
    int priority; /*Prioridad del pcb*/
    int quantum; /*Quantum del pcb*/
    mm_t mm;
    pcb_status_t status;
} n_pcb_t;

/*Estructura para pasarle dos argumentos a una rutina*/

typedef struct arg_struct {
    int arg1;
    int arg2;
}arg_struct_t;

/*----------------------------------ESTRUCTURA DE LA CPU---------------------------------------------------------*/

/*Estrucutra de los procesos del CORE*/
typedef struct process{
    n_pcb_t proceso;
    long IR; /*La instruccion en la que me he quedado*/
    long PC; /*La direccion virtual de la instruccion IR*/
    long * PTBR; /*La tabla de paginas de ese proceso*/
    long Rlist[16]; /*Los 16 registros*/

} process_t;

/*Estrucutra de los CORES de la CPU*/
typedef struct core{
    process_t procesos[CORE_THREADS];
} core_t;

/*Estrucutra de la CPU*/
typedef struct cpu{
    core_t cores[CPU_CORES];
} cpu_t;

#endif