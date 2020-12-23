#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>    // srand, rand,...
#include <pthread.h>
#include <time.h>      // time
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "../Estructuras.h"
#include "../Modifiable_parameters.h"

/*DEFINES*/

#define SIZE 256

/*Variables globales*/

long *memoria; /*Memoria fisica*/
unsigned char *marcos; /*Lista para saber que marcos estan libres o ocupados*/
int *lista_marcos_disponibles;/*Aqui voy a almacenar los marcos que esten libre*/
int num_marcos_disponibles;

int iD_list[200][2];

/*Funciones*/
n_pcb_t create_process(int num_marcos);
void look_for_free_frames(int numeros_de_marcos);
void introduce_in_memory(FILE *file,int numero_de_marcos);
void read_create_pcb();
void inicializar();
void imprimir();

void main(int argc,char *argv[]){
    inicializar();
    read_create_pcb();
    imprimir();
}

void inicializar(){
    /*Inicializo la memoria y la lista de marcos*/
    int longitud_array=SIZE*SIZE;
    memoria = (long*)malloc(sizeof(long)*longitud_array);
    longitud_array = SIZE;
    marcos = (unsigned char*)malloc(longitud_array*sizeof(unsigned char)); //unsigned char
    for(int i =0;i<SIZE;i++){
        marcos[i] = '0';
    }
    num_marcos_disponibles = SIZE;
    /*Inicializo la lista de ids*/
    for(int l=0;l<MAX_PROCESS;l++){
        iD_list[l][0] = -1;
        //iD_list[l][1] = -1;
    }
}


n_pcb_t create_process(int num_marcos){
    /*Creo un pcb*/
    n_pcb_t created_process;
    /*Le doy valores aleatorio al quantum y al id del proceso creado*/
    created_process.id = rand() % MAX_PROCESS;/*Valor aleatorio entre 0 y el numero de procesos maximos*/
    /*Miro a ver si esa Id esta libre*/
    while(1){
        if(iD_list[created_process.id][0] == -1){
            /*Le doy un valor aleatorio al tiempo de vida,prioridad y quantum*/
            created_process.quantum = rand() % 45 + 5;/*Voy a limitar el quantum de cada proceso hasta un maximo de 3*/
            //created_process.burst_time = rand() % 10 + 5; /*Le doy un valor aleatorio del 1 al 4, no quiero que sea muy grande*/
            created_process.priority = rand() % PRIORIDAD_MAX; /*Le doy un valor aleatorio entre el 0 y el PRIORIDAD MAX porque depende del numero de prioridades que queramos*/
            /*Si esta libre estonces se añadira a la lista de ID su correspondiente ID y Quantum*/
            iD_list[created_process.id][0] = created_process.id;
            iD_list[created_process.id][1] = created_process.quantum;

            break;

        }else{
            /*Si esa ID no esta libre, entonces le damos otra id aleatoriamente, y volvemos a comprobar*/
            created_process.id = rand() % MAX_PROCESS;

        }
    }
    for(int i =0 ;i < 16;i++) created_process.status.Rlist[i]=0;
    printf("NUM MARCOS %d\n", num_marcos);
    created_process.mm.num_frames = num_marcos;
    created_process.mm.pgb = (long*)malloc(num_marcos*sizeof(long));
    return created_process;
}

void look_for_free_frames(int numero_de_marcos){

    int num_ceros=0;/**Contador para llevar la cuenta del numero de ceros encontrados, es decir, el numero de marcos libres*/
    int i = 0;/*Contador del bucle*/
    lista_marcos_disponibles = (int *)malloc(numero_de_marcos*sizeof(int)); /*Contiene que marcos estan libres*/
    int contador_lista=0;
    while (i<SIZE && num_ceros < numero_de_marcos){
        if ( marcos[i] == '0'){
            /**/
            printf("Ha entrado\n");
            lista_marcos_disponibles[contador_lista] = i * 256;
            contador_lista++;
            num_ceros++;
            marcos[i]='1';
        }
        i++;
    }
    printf("Valor de i ---> %d",i);
}


void introduce_in_memory(FILE *file,int numero_de_marcos){
    
    char *ptr;
    rewind(file);/*Pungo el indicar mirando al comienzo del fichero para volverlo a leer*/
    char text_to_read[50];
    int firs_two = 0;
    long code_data;
                    printf("-----------------------------------------------------------------\n");

    /*Reserva de memoria de la tabla de paginas del pcb*/
    n_pcb_t pcb = create_process(numero_de_marcos);
    long * bar = (long*)malloc(8);
    if(bar == NULL) {
        printf("Memory allocation error");//Aqui compruebo si el malloc me devuelve null, de ser asi entonces el malloc no ha tenido memoria para reservar, por lo que tendre que ponerlo en una lista de espera para volver a intentar crear el proceso mas tarde.
    }else{
        pcb.mm.pgb = bar;
    }
    /*Asigno la primera relacion de direccion fisica y virtual en la tabla de paginas*/

    long direccion_virtual = (pcb.mm.code & 0xffff00) >> 8; /*Aplico mascara y desplazo*/
    long direccion_fisica = (lista_marcos_disponibles[0] & 0xff00) >> 8;
    //pcb.mm.pgb[0]= direccion_virtual;
    printf("Falla\n");
    pcb.mm.pgb[0] = direccion_fisica;

    //0x001223
    //0x0012
    //pcb.mm.pgb[0x0012]; --> 0xA234
    //0xA23400 + 0x23 = 0xA23423
    long variable = 0;
    int i,j,w;
    /*Contadores para la tabla de pagina*/
    i = 1;
    j = 0;
    /*Contador para la lista de marcos disponibles*/
    w = 0;
    while(1){
        fgets(text_to_read,50,file);
        if(feof(file)) break;/*Para controlar que he llegado al final del archivo*/
        else{
            if(firs_two < 2){ /*Esto es para meter el .text y el .code en el pcb*/
                char * token = strtok(text_to_read ," ");
                token = strtok(NULL ," ");
                code_data = strtol(token, &ptr, 16);
                if (firs_two == 1) pcb.mm.code = code_data;
                else pcb.mm.data = code_data;
                firs_two++;
            }else{ /*Ahora tengo que hacer la traduccion de las direcciones y meterlo en la tabla de paginas*/
                /*Primero meto la traduccion en la tabla de paginas, para ello necesito aplicar una mascara que me deuelva solo los ultimos 6 bytes*/
                code_data = strtol(text_to_read, &ptr, 16);
                if(variable == 256){ /*significa que hay que hacer uso del siguiente marco disponible*/
                    w++;
                    //direccion_virtual = direccion_virtual + 1;
                    direccion_fisica = (lista_marcos_disponibles[w] & 0xff00) >> 8;;
                    pcb.mm.pgb[i] = direccion_virtual;
                    pcb.mm.pgb[w] = direccion_fisica;
                    i++; 
                    j = j+2;
                    variable=0;
                }
                long direccion_fisica_completa = lista_marcos_disponibles[w] + variable;
                memoria[direccion_fisica_completa] = code_data;
                variable = variable + 4;
            }                
        }
    }
}

void read_create_pcb(){

    //lseek(2) a partir de la segunda,tiene que ser multiplo de 9
    int vueltas,num_lineas_archivo,num_marcos;
    num_marcos=0;
    //Voy abrir el escritotio donde tengo los ficheros
    char path[200] = "../Ficheros/"; //Explicar que tiene que meter los ficheros van a estar en la carptea .elf
    struct dirent *dp;
    DIR *dir = opendir(path);
    FILE *fp;
    if (!dir) return; 
    while ((dp = readdir(dir)) != NULL)
    {
        /*Estos dos if es porque yo uso Linux, y me cogia los dos ficheros: . .. que se encuentran en todas la carpetas
        De esta manera avito que me coja eso dos ficheros*/
        if ( !strcmp( dp->d_name, "."   )) continue;
        if ( !strcmp( dp->d_name, ".."  )) continue;
        /*Solo me interan los archivos con extension .elf*/
        size_t len = strlen(dp->d_name);
        if(len > 4 && strcmp(dp->d_name + len - 4, ".elf") == 0) {
            /*La condicion >4 es para que is encuentra un fichero que no tenga la longitud de al menos la extension,
            el programa no se rompa*/
            char nombre_fichero[300];
            strcpy(nombre_fichero,path);
            strcat(nombre_fichero,dp->d_name);
            /*Abro el archivo*/
            vueltas=0;
            num_lineas_archivo=0;
            fp = fopen(nombre_fichero,"r");
            if(fp!=NULL){
                printf("El fichero que estoy leyendo es ---> %s\n",nombre_fichero);
                printf("-----------------------------------------------------------------\n");
                char text_to_read[50];
                while(1){
                    fgets(text_to_read,50,fp);
                    if(!feof(fp)){ /*Para controlar que he llegado al final del archivo*/
                    num_lineas_archivo++;
                    }else break;
                }
                num_lineas_archivo = num_lineas_archivo - 2;//Le quito dos porque las dos primeras lineas nos las voy a meter en la memoeria fisica
                if( num_lineas_archivo % 64 == 0) num_marcos = num_lineas_archivo / 64;
                else num_marcos = num_lineas_archivo / 64 + 1;

                while(1){/*Espero hasta que haya marcos libres*/

                    if(num_marcos_disponibles >= num_marcos){
                        
                        look_for_free_frames(num_marcos);  
                        introduce_in_memory(fp,num_marcos);
                        free(lista_marcos_disponibles);
                        num_marcos_disponibles = num_marcos_disponibles - num_marcos;
                        break;
                    }
                }
            }
        }
    }
}


void imprimir(){
    /*Imprimo la memoria*/
    long que_marco=0x0000;
    printf("------|\n");
    printf("------|\n");
    for(long i=0;i < 64*SIZE;i = i + 4){
        if(i % 256 == 0) {
            printf("Marco -----> %04lx\n",que_marco);
            printf("------|\n");
            printf("------|\n");
            que_marco = que_marco + 256;

        }
        long valor = memoria[i];
        printf("|------Posicion > %04lx --- Valor > %08lx\n",i,valor);
        printf("|");
    }
}