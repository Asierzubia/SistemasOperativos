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


void read_create_pcb();
void main(int argc,char *argv[]){
    read_create_pcb();
}

void read_create_pcb(){

    //Voy abrir el escritotio donde tengo los ficheros
    char path[200] = "/home/asier/Documents/Ingenieria/SO/Proyecto/Memorias/Ficheros/";
    struct dirent *dp;
    DIR *dir = opendir(path);
    FILE *fp;
    if (!dir) 
        return; 
    while ((dp = readdir(dir)) != NULL)
    {
        if ( !strcmp( dp->d_name, "."   )) continue;
        if ( !strcmp( dp->d_name, ".."  )) continue;
        /*Solo me interan los archivos con extension .elf*/
        size_t len = strlen(dp->d_name);
        if(len > 4 && strcmp(dp->d_name + len - 4, ".elf") == 0) {
            /*Abro los archivo*/
            
            char nombre_fichero[300];
            strcpy(nombre_fichero,path);
            strcat(nombre_fichero,dp->d_name);
            fp=fopen(nombre_fichero,"r");
            /*Leo los archivos*/
            char text_to_read[50];
            int num_lineas = 1;
            if(fp != NULL){
                printf("El fcihero que estoy leyendo es ---> %s\n",nombre_fichero);
                while(1){
                    fgets(text_to_read,50,fp);
                    if(feof(fp)){ /*Para controlar que he llegado al final del archivo*/
                        break;
                    }
                    if(num_lineas == 1 || num_lineas == 2){ /*La primera linea de los archivos contiene la direccion el que empieza el text en la memoria virtual*/
                        char * token = strtok(text_to_read ," ");
                        token = strtok(NULL ," ");
                        strcpy(text_to_read,token);
                    }
                    num_lineas++;
                    char *ptr;
                    long code_data = strtol(text_to_read, &ptr, 16);
                    printf("The number(unsigned long integer) is %08lx\n", code_data);
                } 
            }
        }
    }
    closedir(dir);
}