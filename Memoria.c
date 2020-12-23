#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>    // srand, rand,...
#include <pthread.h>
#include <time.h>      // time
#include <sys/types.h>
#include <sys/stat.h>
#include "Estructuras.h"
#include "Modifiable_parameters.h"

/*-------------------Variables realacionados con la memoria fisica y virtual---------------------*/

long physical_memory[size*size]; /*Memoria fisica*/
int current_address; /*Para controlar los fallos de pagina*/

int offset; /*Offset*/
int logical_address,physical_address; /*Direccion fisica y logica*/

int page_numer; /*Numero de pagina de la memoria virtual*/
int page_offset; /*Offset de la pagina*/

/*-------------------------------------------------------------*/

/*Variables relacionadas con los ficheros*/
    char *text;
    char *data;

/*Archivos que me pasan como argumento*/
    

void main(int argc,char *argv[]){
    text=argv[1];
    data=argv[2];
    loader();
}

void add_to_pmemory(n_pcb_t proceso){

    /*Miro donde hay sitio en la memoeria fisica*/
    /*Tengo que meter tanto el dato como la instruccion*/
    for(int i=current_address;i<size*size;i++){/*Empiezo desde la ultima posicion de la memoria en la que anadi un dato, para que sea mas eficiente, y no recorra siempre desde el principio la memoria fisica*/

        if(physical_memory[i] == -1){ /*Compruebo si esta libre, pero tambien tengo que comprobar si cabe*/
            physical_address = (i << 8) | offset;
            physical_memory[i]=proceso.n_mm.data;
            /*Falta comprobar si cabe*/
            if(cabe){/*No se como comprobar si cabe*/
                /*Actualizo el current_address*/
                /*Meto el dato o instruccion*/
                /*Tengo que meterlo en la direccion de memoria fisica pero con el mismo offset que el de la direccion de memoria virtual*/
                /*Actualizar la tabla de paginas del pcb*/
                proceso.n_mm.pgb[page_numer][0]=physical_address; /*No estoy seguro de que sea asi*/
            }/*Si no, no hago nada sigo buscando espacio*/
            /*Tengo que hacerlo dos veces, uno para el dato y el otro para la instruccion*/   
        } 
    }
    /*Iprimir tanto la tabla de paginas como el dato introducido y en que direccion
    para comprobar que lo he introducido correctamente.*/
/*-------------------------Que hacer-----------------------------------------*/
 
    /*Aqui tengo que mirar en la memoria fisica
    donde hay sitio para meter el dato. Si no hay
    espacio no se hace nada*/
    /*Una vez hecho eso los paso a seguir son los siguientes
    -Meter el dato en donde he encontrado sitio
    -Despues asociar la direccion de la memoria virtual
    a la direccion de la memoria fisica en la que acabo de 
    meter el dato. Esto es metiendolo en la tabla de paginas
    -Marcar en mi mapa de bits que ese espacio que acabo de 
    rellenar esta ocupado
    -Actualizar el indice que me indica en que direccion de memoria 
    fisica estamos despues de introducir el dato
    -Imprimir por pantalla el lo que haya en las direcciones de memoria
    que acabo de meter el dato para saber si se ha introducido 
    correctamente*/
/*----------------------------------------------------------------------------*/
}
/*Modificarlo en baso a como sean los ficheros*/
void n_create_process(){
    FILE *fp_text,*fp_data;
    /*Abro los archivo*/
    fp_text=fopen(text,"r");
    fp_data=fopen(data,"r");
    /*Leo los archivos*/
    long text_to_read;
    long data_to_read;
    if(fp_text != NULL){
        if(fp_data != NULL){
            while(1){ 
                text_to_read=fgetc(fp_text);
                data_to_read=fgetc(fp_data);
                if(feof(fp_text) || feof(fp_data)){ /*Para controlar que he llegado al final del archivo*/
                    break;
                }
                pcb_t proceso=create_process(); 
                /*Meto en el pcb el code y data*/
                proceso.n_mm.code=text_to_read;
                proceso.n_mm.data=data_to_read;
                /*Ahora aplico las marcaras para sacar lo que me interesa, el offset y la direccion virtual*/
                logical_address=atoi(text_to_read);
                offset=(logical_address & 0x000000ffUL);/*Aplico la marcara para quedarme con el offset*/
                page_numer=(logical_address & 0xffffff00UL) >>  8; /*Lo desplazo 8 y la aplico la mascar para quedarme con la direccion de memoria virtual*/
                proceso.n_mm.pgb[page_numer]=page_numer;
                /*Una vez obtenido el offset saco el registro y la intruccion. NO CREO QUE SEA NECESARIO*/
                //int registro = offset >> 4 & 0x0F;
                //int intruccion = offset & 0x0F;
                /*Ahora ya solo queda llamar a la funcion add_to_memory. Pensar que argumentos tengo que pasarle.*/
                add_to_pmemory(proceso);
            }
        }
        fclose(fp_data);
    }
    fclose(fp_text);
    
    logical_address;
    /*Mascaras para sacar los valores que quiero*/
    

/*-------------------------Que hacer-----------------------------------------*/
    /*Aqui tengo que extraer el data y el text de los archivos
    para meterlos en la estructura mm del pcb.Ademas ya pongo en 
    la tabla de paginas la direcion virtual*/
    /*Cada intruccion tiene 32 bits(4 Bytes), es decir un Word*/
    /*Necesito sacar el offset mediante una mascara, son los 8 primeros
    bits.
    Los ocho primeros bits contienen la intruccion a realizar por ejemplo
    -----> load -> Aplicar mascara para saber este dato -> &0x0f preguntar
    y el registro en el que se va a almacenar
    -----> r1 -> Aplicar mascara para saber este dato -> Desplazo 4 y aplico mascara &0x0f
    el resto de los 24 bits son la direccion de memoria virtual*/
/*-----------------------------------------------------------------------------*/

}

void loader(){
    int i=0;
    n_create_process();
}