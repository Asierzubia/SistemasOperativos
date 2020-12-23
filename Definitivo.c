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
#include "Estructuras.h"
#include "Modifiable_parameters.h"

int look_for_free_frames(int numeros_de_marcos){

}

void read_create_pcb(){
    int vueltas,num_lineas_archivo,num_marcos;
    num_marcos=0;
    //Voy abrir el escritotio donde tengo los ficheros
    char path[200] = "/home/asier/Documents/Ingenieria/SO/Proyecto/Memorias/Ficheros/";
    struct dirent *dp;
    DIR *dir = opendir(path);
    FILE *fp;
    if (!dir) 
        return; 
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
            while(vueltas < 2 ){ /* Lo tengo que recorrer dos veces, una para contar el numero de lineas y otra para ir metiendo una a una las instrucciones y datos*/
                fp=fopen(nombre_fichero,"r");
                char text_to_read[50];
                /*Leo los archivos*/
                if(vueltas == 0 ){
                    if(fp != NULL){
                        while(1){
                            fgets(text_to_read,50,fp);
                            if(!feof(fp)){ /*Para controlar que he llegado al final del archivo*/
                                num_lineas_archivo++;
                            }else {
                                vueltas++;
                                break;
                            }
                        }
                        /*Saco el numero de marcos que necesita el programa*/
                        if( num_lineas_archivo % 64 == 0) num_marcos = num_lineas_archivo / 64;
                        else num_marcos = num_lineas_archivo / 64 + 1;
                        int libre = look_for_free_frames(num_marcos);
                        if (libre == 1){/*Significa que se puede meter el programa*/
                            
                        }
                        else {/*Significa que no se puede meter el programa*/

                        }
                    }
                }
                else{
                    int num_lineas = 1;  /*Creo un contador porque las dos primeras lineas me interesan, para meterlas en el pcb*/
                    if(fp != NULL){
                        printf("El fcihero que estoy leyendo es ---> %s\n",nombre_fichero);
                        /*Creo el pcb*/
                        n_pcb_t pcb;
                        int datos = 0; /*Variable para saber cuando empiezan los datos*/
                        while(1){
                            fgets(text_to_read,50,fp);
                            if(feof(fp)){ /*Para controlar que he llegado al final del archivo*/
                                break;
                            }
                            char *ptr;
                            if(num_lineas == 1 || num_lineas == 2){ /*La primera linea de los archivos contiene la direccion el que empieza el text en la memoria virtual*/
                                char * token = strtok(text_to_read ," ");
                                token = strtok(NULL ," ");
                                long code_data = strtol(token, &ptr, 16);
                                if (num_lineas == 1) pcb.mm.code = code_data;
                                else pcb.mm.data = code_data;
                            }else{
                                long code_data = strtol(text_to_read, &ptr, 16);
                                if ( datos == 1){

                                }
                                else{ /*Compruebo si es EXIT para saber cuando empiezan los datos*/
                                    if( strcmp(text_to_read, "F0000000")) datos = 1;
                                }
                            }   
                            num_lineas++;
                        }
                    }
                    vueltas++;
                }
            }
        }
    }
    closedir(dir);
}