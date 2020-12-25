#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h> // srand, rand,...
#include <pthread.h>
#include <time.h> // time
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "../Estructuras.h"
#include "../Modifiable_parameters.h"

#define SIZE 256
/*-------------------Variables globales---------------------*/

/*Es una lista de prioridades y dentro de cada lista de prioridades contiene los correspondientes procesos que están en espera para ser ejecutados en la “CPU”.*/

n_pcb_t lista_procesos_espera[PRIORIDAD_MAX][MAX_PROCESS];

/*Variable de numero de procesos creados para llevar saber 
en todo momento si puedo crear un nuevo proceso o no*/

int num_processes_created;

/* Lista de IDs*/

int iD_list[MAX_PROCESS][2];
//En la posicion [x][0] tiene el ID del proceso
//En la posiciom [x][1] tiene el quantum del proceso que luego me servira para recuperarlo cuando lo vuelva a meter en la lista de espera de procesos

/* Creo la CPU */
cpu_t cpu[MAX_CPUS];

/*Variables globales necesarias para que se comuniquen el timer y el clock, y el timer y el scheduler*/

int timer_ready;
int clock_ready;
int hayProceso;
/*Creo un mutex para el clock, timer y el scheduler*/

pthread_mutex_t mutex_ts, mutex_ct, mutex_clock_scheduler;

/*-------------------Variables realacionados con la memoria fisica y virtual---------------------*/

long *memoria;         /*Memoria fisica*/
unsigned char *marcos; /*Lista para saber que marcos estan libres o ocupados*/
int marco_tabla_pagina;
unsigned char leer_programa;
int *lista_marcos_disponibles; /*Aqui voy a almacenar los marcos que esten libre*/
int num_marcos_disponibles;

/*-------------------------------------------------------------*/
long *memoria; /*Memoria fisica*/

/*Funciones*/
void main(int argc, char *argv[]);
void inicializar();
int mensaje_error(char *s);
void *timer(void *arguments);
void *clocker(void *arguments);
void *procesGenerator(void *arguments);
void *loader(void *arguments);
n_pcb_t create_process(int num_marcos);
void look_for_free_frames(int numeros_de_marcos);
void introduce_in_memory(FILE *file, int numero_de_marcos);
void read_create_pcb();
void inicializar();
void imprimir();
void doIntruct(long intruccion, int i, int j, int w);

void main(int argc, char *argv[])
{

    n_pcb_t idprod[5];
    arg_struct_t args_clock, args_timer, args_processGenerator;
    inicializar();

    /* Crear los hilos */

    for (int i = 0; i <= 4; i++)
    {
        idprod[i].id = i;
    }

    /*Relleno la estructura creada para poder pasar los dos argumentos a la rutina que yo quiero*/

    args_clock.arg1 = idprod[1].id;
    args_timer.arg1 = idprod[2].id;
    args_processGenerator.arg1 = idprod[3].id;

    args_clock.arg2 = Clock_Frequency;
    args_timer.arg2 = Timer_cycle;
    args_processGenerator.arg2 = PG_Frequency;

    /*En caso de que en un futuro quisiese implmentar el programa para poder pasarle yo como parametros de entrada
    la frecuencia del clok, del timer y del generador de procesos, simplemente quitar las // de las tres siguientes
    lineas y comentar las tres anteriores*/

    //args_clock.arg2=atoi(argv[1]);
    //args_timer.arg2=atoi(argv[2]);
    //args_processGenerator.arg2=atoi(argv[3]);

    /*Creo los hilos*/

    pthread_create(&(idprod[1].hilo), NULL, clocker, (void *)&args_clock);
    pthread_create(&(idprod[2].hilo), NULL, timer, (void *)&args_timer);
    pthread_create(&(idprod[3].hilo), NULL, procesGenerator, (void *)&args_processGenerator);
    pthread_create(&(idprod[4].hilo), NULL, loader, &(idprod[4].id));

    sleep(WAITING_TO_EXIT);
    pthread_cancel(idprod[1].hilo);
    pthread_cancel(idprod[2].hilo);
    pthread_cancel(idprod[3].hilo);
    pthread_cancel(idprod[4].hilo);

    printf("Fin\n");

    /*Imprime todos los procesos de cada core de cada CPU. Hay procesos con ID -200, eso significa que es un proceso
    muerto, y que cuando se pueda añadir un nuevo proceso, se añadira donde esta ese proceso*/
    /*printf("-------------------------------------------------------------------------------------------------\n");
    for(int i =0;i<MAX_CPUS;i++){
        printf("CPU NUMERO %d\n",i);
        for(int j=0;j<CPU_CORES;j++){
            printf("CORE NUMERO %d\n",j);
            for(int w=0;w<CORE_THREADS;w++){
                printf("PROCESO CON ID %d ----> QUANTUM: %d  ----> TEIMPO DE VIDA: %d\n",cpu[i].cores[j].procesos[w].proceso.id,cpu[i].cores[j].procesos[w].proceso.quantum,cpu[i].cores[j].procesos[w].proceso.quantum);
            }
        }
    }
    printf("-------------------------------------------------------------------------------------------------\n");
    */
    /*Imprime los procesos de las listas de prioridades de la lista de procesos de espera */
    /*for(int i=0;i<PRIORIDAD_MAX;i++){
        printf("PRIORIDAD: %d\n",i);
        for(int j=0;j<MAX_PROCESS;j++){
            printf("-------------- PID = %d\n",lista_procesos_espera[i][j].id);
        }
    }*/
    //imprimir();
    free(lista_marcos_disponibles);
    free(memoria);
    free(marcos);
}

n_pcb_t create_process(int num_marcos)
{
    /*Creo un pcb*/
    n_pcb_t created_process;
    /*Le doy valores aleatorio al quantum y al id del proceso creado*/
    created_process.id = rand() % MAX_PROCESS; /*Valor aleatorio entre 0 y el numero de procesos maximos*/
    /*Miro a ver si esa Id esta libre*/
    while (1)
    {
        if (iD_list[created_process.id][0] == -1)
        {
            /*Le doy un valor aleatorio al tiempo de vida,prioridad y quantum*/
            created_process.quantum = rand() % 45 + 5; /*Voy a limitar el quantum de cada proceso hasta un maximo de 3*/
            //created_process.burst_time = rand() % 10 + 5; /*Le doy un valor aleatorio del 1 al 4, no quiero que sea muy grande*/
            created_process.priority = rand() % PRIORIDAD_MAX; /*Le doy un valor aleatorio entre el 0 y el PRIORIDAD MAX porque depende del numero de prioridades que queramos*/
            /*Si esta libre estonces se añadira a la lista de ID su correspondiente ID y Quantum*/
            iD_list[created_process.id][0] = created_process.id;
            iD_list[created_process.id][1] = created_process.quantum;

            break;
        }
        else
        {
            /*Si esa ID no esta libre, entonces le damos otra id aleatoriamente, y volvemos a comprobar*/
            created_process.id = rand() % MAX_PROCESS;
        }
    }
    created_process.mm.num_frames = num_marcos;
    /*Rservo memoria para la tabla de paginas del proceso*/
    created_process.mm.pgb = (long *)malloc(num_marcos * sizeof(long));
    if (created_process.mm.pgb == NULL)
    {
        printf("Memory allocation error"); //Aqui compruebo si el malloc me devuelve null, de ser asi entonces el malloc no ha tenido memoria para reservar, por lo que tendre que ponerlo en una lista de espera para volver a intentar crear el proceso mas tarde.
    }
    /*Inicializo los registros del proceso*/
    for (int l = 0; l < 16; l++)
        created_process.status.Rlist[l] = 0;
    return created_process;
}

void inicializar()
{
    srand(time(NULL));
    num_processes_created = 0;
    hayProceso = 0;
    marco_tabla_pagina = 0;
    /*Inicializo los mutex*/
    pthread_mutex_init(&mutex_ts, NULL);
    pthread_mutex_init(&mutex_ct, NULL);
    pthread_mutex_init(&mutex_clock_scheduler, NULL);
    int longitud_array = SIZE * SIZE;
    memoria = (long *)malloc(sizeof(long) * longitud_array);
    longitud_array = SIZE;
    marcos = (unsigned char *)malloc(longitud_array * sizeof(unsigned char)); //unsigned char
    for (int i = 0; i < SIZE; i++)
    {
        marcos[i] = '0';
    }
    num_marcos_disponibles = SIZE;
    /*Inicializo las variables que van a servir como señales*/
    clock_ready = 0;
    timer_ready = 0;
    /*Inicializo la lista de ids*/
    for (int l = 0; l < MAX_PROCESS; l++)
    {
        iD_list[l][0] = -1;
        //iD_list[l][1] = -1;
    }
    /*Inicializar la lista de procesos de espera*/
    for (int p = 0; p < PRIORIDAD_MAX; p++)
    {
        for (int i = 0; i < MAX_PROCESS; i++)
        {
            n_pcb_t pro;
            pro.id = -1;
            lista_procesos_espera[p][i] = pro;
        }
    }
    leer_programa = '1';
    //Rellenar los arrays de los cores de procesos y tambien me los imprime por pantalla
    printf("-------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < MAX_CPUS; i++)
    {
        printf("CPU NUMERO %d\n", i);
        for (int j = 0; j < CPU_CORES; j++)
        {
            printf("CORE NUMERO %d\n", j);
            for (int w = 0; w < CORE_THREADS; w++)
            {

                n_pcb_t proceso = create_process(1);
                proceso.id = PROCESO_MUERTO;
                cpu[i].cores[j].procesos[w].proceso = proceso;
                //printf("PROCESO CON ID %d ----> QUANTUM: %d \n",cpu[i].cores[j].procesos[w].proceso.id,cpu[i].cores[j].procesos[w].proceso.quantum);
            }
        }
    }
    //printf("-------------------------------------------------------------------------------------------------\n");
}

unsigned char doIntrusct(int i, int j, int w)
{

    unsigned char fin = '0';
    /*Codigo*/
    long dir_instruccion = cpu[i].cores[j].procesos[w].PC;
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("                            INSTRUCCION A EJECUTAR ---> %08lx                                       \n", dir_instruccion);
    printf("----------------------------------------------------------------------------------------------------\n");

    long tipo_instruccion = (dir_instruccion & 0xF0000000) >> 28;
    long direccion_virtual = (dir_instruccion & 0x00FFFFFF);
    long offset = (direccion_virtual & 0x0000FF);
    long dir_virt_pgt = (direccion_virtual & 0xFFFF00) >> 8;
    long dir_fis_pgt = cpu[i].cores[j].procesos[w].proceso.mm.pgb[dir_virt_pgt];
    long direccion_fisica = dir_fis_pgt + offset;
    if (tipo_instruccion == 0 || tipo_instruccion == 1)
    { /*LD o ST*/

        long reg = (dir_instruccion & 0x0F000000) >> 24;

        if (tipo_instruccion == 0)
        {
            printf("----------------------------------------------------------------------------------------------------\n");
            printf("                            TIPO DE INTRUCCION ---> LD                                             \n");
            printf("----------------------------------------------------------------------------------------------------\n");
            cpu[i].cores[j].procesos[w].Rlist[reg] = memoria[direccion_fisica];
        }
        else
        {
            printf("----------------------------------------------------------------------------------------------------\n");
            printf("                            TIPO DE INTRUCCION ---> ST                                             \n");
            printf("----------------------------------------------------------------------------------------------------\n");
            memoria[direccion_fisica] = cpu[i].cores[j].procesos[w].Rlist[reg];
        }
    }
    else if (tipo_instruccion == 2)
    { /*ADD*/
        printf("----------------------------------------------------------------------------------------------------\n");
        printf("                            TIPO DE INTRUCCION ---> ADD                                             \n");
        printf("----------------------------------------------------------------------------------------------------\n");
        /*Hay que mirar los 3 registros que necesito*/
        long r1 = (dir_instruccion & 0x0F000000) >> 24;
        long r2 = (dir_instruccion & 0x00F00000) >> 20;
        long r3 = (dir_instruccion & 0x000F0000) >> 16;
        cpu[i].cores[j].procesos[w].Rlist[r1] = r2 + r3;
    }
    else
    { /*EXIT*/
        printf("----------------------------------------------------------------------------------------------------\n");
        printf("                            TIPO DE INTRUCCION ---> EXIT                                             \n");
        printf("----------------------------------------------------------------------------------------------------\n");
        fin = '1';
        cpu[i].cores[j].procesos[w].proceso.id = PROCESO_MUERTO;
        marco_tabla_pagina = 0;
    }
    if (fin == '0')
    {
        /*Tengo que modificar el estado del proceso, para que en la siguiente vuelta no ejecute 
        la misma instruccion y para mas cosas*/
        cpu[i].cores[j].procesos[w].IR = cpu[i].cores[j].procesos[w].IR + 1;
        if (cpu[i].cores[j].procesos[w].IR % 64 == 0)
        {
            marco_tabla_pagina++;
        }
        cpu[i].cores[j].procesos[w].PC = memoria[lista_marcos_disponibles[marco_tabla_pagina] + 4];
        printf("----------------------------------------------------------------------------------------------------\n");
        printf("                            SIGUIENTE INTRUCCION ---> %08lx                                             \n", cpu[i].cores[j].procesos[w].IR);
        printf("----------------------------------------------------------------------------------------------------\n");
    }
}

void revisar_procesos()
{

    pthread_mutex_lock(&mutex_clock_scheduler);
    for (int i = 0; i < MAX_CPUS; i++)
    {

        for (int j = 0; j < CPU_CORES; j++)
        {

            for (int w = 0; w < CORE_THREADS; w++)
            {
                /*Si es un proceso muerto no me interesa mirar ni su tiempo de vida ni su quantum*/
                if (cpu[i].cores[j].procesos[w].proceso.id != PROCESO_MUERTO)
                {
                    /*Solo me interesa mirar todos aquellos que tengan tiempo de vida y quantum superior a 0*/

                    if (cpu[i].cores[j].procesos[w].proceso.quantum > 0)
                    {
                        /*Creo un hilo para que se ejecute, fijo que si pero a ver como lo hago*/
                        cpu[i].cores[j].procesos[w].proceso.quantum -= 1;
                    }

                    //printf("He bajado el quantum al proces: %d ---> QUANTUM: %d  ---> PRIORIDAD: %d\n",cpu[i].cores[j].procesos[w].proceso.id,cpu[i].cores[j].procesos[w].proceso.quantum,cpu[i].cores[j].procesos[w].proceso.priority);
                }
                doIntrusct(i, j, w);
            }
        }
    }
    pthread_mutex_unlock(&mutex_clock_scheduler);
}

void *clocker(void *arguments)
{
    struct arg_struct *args = arguments;
    //printf("El ha clock empezado a funcionar. Hilo %d \n",args -> arg1);
    int frequency = args->arg2;
    int i;
    int conteo = 0;
    int frecuencia_final = 1000000 / frequency;
    while (1)
    {
        i = 0;
        while (i < frecuencia_final)
        {
            i++;
        }
        pthread_mutex_lock(&mutex_ct);
        /*Avisa al timer de que ha terminado un ciclo de reloj*/
        clock_ready = 1;
        pthread_mutex_unlock(&mutex_ct);
        conteo++;
        //printf("El clock ha generado un ciclo ---> CICLO NUMERO: %d\n",conteo);
        /*Como ha terminado un ciclo de reloj procede a revisar los procesos de las CPUS*/
        if (hayProceso == 1)
        {
            revisar_procesos();
        }
    }
}

void *timer(void *arguments)
{
    struct arg_struct *args = arguments;
    //printf("El timer ha empezado a funcionar. Hilo %d \n",args -> arg1);
    int frecuencia = args->arg2;
    int duerme = 0;
    int conteo = 0;
    while (1)
    {
        pthread_mutex_lock(&mutex_ct);
        if (clock_ready == 1)
        {
            clock_ready = 0;
            /*Si el clock ha realizado frecuencia veces entonces avisa el scheduler*/
            if (conteo == frecuencia)
            {
                //printf("El timer ha recibido la señal del clock \n");
                duerme = 1;
                conteo = 0;
            }
            else
            {
                conteo++;
            }
        }
        pthread_mutex_unlock(&mutex_ct);
        if (duerme == 1)
        {
            duerme = 0;
            //printf("El timer ha terminado de contar y procede a enviar la señal al scheduler \n");
            pthread_mutex_lock(&mutex_ts);
            timer_ready = 1;
            pthread_mutex_unlock(&mutex_ts);
        }
    }
}
void look_for_free_frames(int numero_de_marcos)
{

    int num_ceros = 0;                                                        /**Contador para llevar la cuenta del numero de ceros encontrados, es decir, el numero de marcos libres*/
    int i = 0;                                                                /*Contador del bucle*/
    lista_marcos_disponibles = (int *)malloc(numero_de_marcos * sizeof(int)); /*Contiene que marcos estan libres*/
    int contador_lista = 0;
    while (i < SIZE && num_ceros < numero_de_marcos)
    {
        if (marcos[i] == '0')
        {
            /**/
            printf("Ha entrado\n");
            lista_marcos_disponibles[contador_lista] = i * 256;
            contador_lista++;
            num_ceros++;
            marcos[i] = '1';
        }
        i++;
    }
    printf("Valor de i ---> %d", i);
}

void introduce_in_memory(FILE *file, int numero_de_marcos)
{

    char *ptr;
    rewind(file); /*Pungo el indicar mirando al comienzo del fichero para volverlo a leer*/
    char text_to_read[50];
    int firs_two = 0;
    long code_data;
    printf("-----------------------------------------------------------------\n");

    /*Reserva de memoria de la tabla de paginas del pcb*/
    n_pcb_t pcb = create_process(numero_de_marcos);
    pcb.status.IR = 0;
    long *bar = (long *)malloc(8);
    if (bar == NULL)
    {
        printf("Memory allocation error"); //Aqui compruebo si el malloc me devuelve null, de ser asi entonces el malloc no ha tenido memoria para reservar, por lo que tendre que ponerlo en una lista de espera para volver a intentar crear el proceso mas tarde.
    }
    else
    {
        pcb.mm.pgb = bar;
    }
    /*Asigno la primera relacion de direccion fisica y virtual en la tabla de paginas*/

    long direccion_virtual = (pcb.mm.code & 0xffff00) >> 8; /*Aplico mascara y desplazo*/
    long direccion_fisica = (lista_marcos_disponibles[0] & 0xff00) >> 8;
    pcb.mm.pgb[0] = direccion_fisica;
    pcb.status.PC = memoria[lista_marcos_disponibles[0] + 0x00];
    long variable = 0;
    int i, j, w;
    /*Contadores para la tabla de pagina*/
    i = 1;
    /*Contador para la lista de marcos disponibles*/
    w = 0;
    while (1)
    {
        fgets(text_to_read, 50, file);
        if (feof(file))
            break; /*Para controlar que he llegado al final del archivo*/
        else
        {
            if (firs_two < 2)
            { /*Esto es para meter el .text y el .code en el pcb*/
                char *token = strtok(text_to_read, " ");
                token = strtok(NULL, " ");
                code_data = strtol(token, &ptr, 16);
                if (firs_two == 1)
                    pcb.mm.code = code_data;
                else
                    pcb.mm.data = code_data;
                firs_two++; //pcb.mm.pgb[0]= direccion_virtual;
            }
            else
            { /*Ahora tengo que hacer la traduccion de las direcciones y meterlo en la tabla de paginas*/
                /*Primero meto la traduccion en la tabla de paginas, para ello necesito aplicar una mascara que me deuelva solo los ultimos 6 bytes*/
                code_data = strtol(text_to_read, &ptr, 16);
                if (variable == 256)
                { /*significa que hay que hacer uso del siguiente marco disponible*/
                    w++;
                    //direccion_virtual = direccion_virtual + 1;
                    direccion_fisica = (lista_marcos_disponibles[w] & 0xff00) >> 8;
                    ;
                    pcb.mm.pgb[i] = direccion_fisica;
                    i++;
                    variable = 0;
                }
                long direccion_fisica_completa = lista_marcos_disponibles[w] + variable;
                memoria[direccion_fisica_completa] = code_data;
                variable = variable + 4;
            }
        }
    }
    hayProceso = 1;
    /*Meto el proceso en la lista de espera*/
    for (int i = 0; i < MAX_CPUS; i++)
    {

        for (int j = 0; j < CPU_CORES; j++)
        {

            for (int w = 0; w < CORE_THREADS; w++)
            {

                if (cpu[i].cores[j].procesos[w].proceso.id == PROCESO_MUERTO)
                {
                    cpu[i].cores[j].procesos[w].proceso = pcb;
                }
            }
        }
    }
    imprimir();
}
void read_create_pcb()
{

    //lseek(2) a partir de la segunda,tiene que ser multiplo de 9
    int vueltas, num_lineas_archivo, num_marcos;
    num_marcos = 0;
    //Voy abrir el escritotio donde tengo los ficheros
    char path[200] = "../Ficheros/"; //Explicar que tiene que meter los ficheros van a estar en la carptea .elf
    struct dirent *dp;
    DIR *dir = opendir(path);
    FILE *fp;
    if (!dir)
        return;
    while ((dp = readdir(dir)) != NULL)
    {
        /*Estos dos if es porque yo uso Linux, y me cogia los dos ficheros: . .. que se encuentran en todas la carpetas
        De esta manera avito que me coja eso dos ficheros*/
        if (!strcmp(dp->d_name, "."))
            continue;
        if (!strcmp(dp->d_name, ".."))
            continue;
        /*Solo me interan los archivos con extension .elf*/
        size_t len = strlen(dp->d_name);
        if (len > 4 && strcmp(dp->d_name + len - 4, ".elf") == 0)
        {
            /*La condicion >4 es para que is encuentra un fichero que no tenga la longitud de al menos la extension,
            el programa no se rompa*/
            char nombre_fichero[300];
            strcpy(nombre_fichero, path);
            strcat(nombre_fichero, dp->d_name);
            /*Abro el archivo*/
            vueltas = 0;
            num_lineas_archivo = 0;
            fp = fopen(nombre_fichero, "r");
            if (fp != NULL)
            {
                printf("El fichero que estoy leyendo es ---> %s\n", nombre_fichero);
                printf("-----------------------------------------------------------------\n");
                char text_to_read[50];
                while (1)
                {
                    fgets(text_to_read, 50, fp);
                    if (!feof(fp))
                    { /*Para controlar que he llegado al final del archivo*/
                        num_lineas_archivo++;
                    }
                    else
                        break;
                }
                num_lineas_archivo = num_lineas_archivo - 2; //Le quito dos porque las dos primeras lineas nos las voy a meter en la memoeria fisica
                if (num_lineas_archivo % 64 == 0)
                    num_marcos = num_lineas_archivo / 64;
                else
                    num_marcos = num_lineas_archivo / 64 + 1;
                while (1)
                { /*Espero hasta que haya marcos libres*/
                    printf("Ha llegado hasta aqui\n");

                    if (num_marcos_disponibles >= num_marcos && leer_programa == '1')
                    {
                        leer_programa = '0';
                        printf("Hasta aqui si llega");
                        look_for_free_frames(num_marcos);
                        introduce_in_memory(fp, num_marcos);
                        free(lista_marcos_disponibles);
                        num_marcos_disponibles = num_marcos_disponibles - num_marcos;
                        break;
                    }
                }
            }
        }
    }
}

void *procesGenerator(void *arguments)
{ /*Leer todos de una vez, o cada vez que salta el process genertor leer un programa*/
    struct arg_struct *args = arguments;
    //printf("El generador de procesos ha empezado a funcionar. Hilo %d \n",args -> arg1);
    int frecuencia = args->arg2;
    int i;
    read_create_pcb();
    while (1)
    {

        while (i < PG_Frequency)
        {
            i++;
        }
        leer_programa = '1';
        i = 0;
        /*Cada cierta frecuencia intenta crear un nuevo proceso y añadirlo a la lista de espera de procesos*/
    }
}

void add_process_to_queue(int i, int j, int k)
{

    int salir = 0;
    for (int o = 0; o < PRIORIDAD_MAX; o++)
    {
        for (int t = 0; t < MAX_PROCESS; t++)
        {
            /*Si hay sitio en alguna lista de procesos de algun core de alguna cpu, lo mete*/
            if (lista_procesos_espera[o][t].id != -1 && lista_procesos_espera[o][t].id != PROCESO_MUERTO)
            {
                cpu[i].cores[j].procesos[k].proceso = lista_procesos_espera[o][t];
                lista_procesos_espera[o][t].id = -1;
                num_processes_created--;
                //printf("Se ha anadido el proceso: %d \n",cpu[i].cores[j].procesos[k].proceso.id);
                /*Poner el proceso del hilo en el estado en que esta el proceso*/
                cpu[i].cores[j].procesos[k].IR = cpu[i].cores[j].procesos[k].proceso.status.IR;
                cpu[i].cores[j].procesos[k].PC = cpu[i].cores[j].procesos[k].proceso.status.PC;
                for (int l = 0; l < 16; l++)
                    cpu[i].cores[j].procesos[k].Rlist[l] = cpu[i].cores[j].procesos[k].proceso.status.Rlist[l];
                salir = 1;
                break;
            }
        }
        if (salir == 1)
        {
            salir = 0;
            break;
        }
    }
}

void remove_from_queue(int i, int j, int w)
{

    /*Lo meto en la lista de procesos en espera*/
    /*Tengo que mirar a ver si hay sitio en la lista de procesos de la prioridad del proceso que voy a meter en la
    lista de espera*/

    for (int h = 0; h < MAX_PROCESS; h++)
    {
        if (lista_procesos_espera[cpu[i].cores[j].procesos[w].proceso.priority][h].id == -1)
        {
            /*Aqui le bajo la prioridad al proceso cada vez que vuelve a la lista de procesos de espera*/
            //printf("PRIORIDAD ANTES ---> %d\n",cpu[i].cores[j].procesos[w].proceso.priority);
            int prioridad = cpu[i].cores[j].procesos[w].proceso.priority;
            if (prioridad != (PRIORIDAD_MAX - 1))
            {
                prioridad = prioridad + 1;
                cpu[i].cores[j].procesos[w].proceso.priority = prioridad;
            }
            lista_procesos_espera[cpu[i].cores[j].procesos[w].proceso.priority][h] = cpu[i].cores[j].procesos[w].proceso;
            //printf("PRIORIDAD DESPUES ---> %d\n",lista_procesos_espera[cpu[i].cores[j].procesos[w].proceso.priority][h].priority);
            //Para poder recuperar el quantum del proceso
            lista_procesos_espera[cpu[i].cores[j].procesos[w].proceso.priority][h].quantum = iD_list[h][1];
            /*Tengo que copiar el estado del hilo al proceso*/
            cpu[i].cores[j].procesos[w].proceso.status.IR = cpu[i].cores[j].procesos[w].IR;
            cpu[i].cores[j].procesos[w].proceso.status.PC = cpu[i].cores[j].procesos[w].PC;
            for (int l = 0; l < 16; l++)
                cpu[i].cores[j].procesos[w].proceso.status.Rlist[l] = cpu[i].cores[j].procesos[w].Rlist[l];

            //printf("Se ha quitado de la cola de procesos el proceso: %d ----> QUANTUM: %d\n",lista_procesos_espera[cpu[i].cores[j].procesos[w].proceso.priority][h].id,lista_procesos_espera[cpu[i].cores[j].procesos[w].proceso.priority][h].quantum);
            break;
        }
    }
}

void delete_process(int i, int j, int w)
{

    //printf("Se ha eliminado el proceso: %d\n",cpu[i].cores[j].procesos[w].proceso.id);
    iD_list[cpu[i].cores[j].procesos[w].proceso.id][0] = -1;
    cpu[i].cores[j].procesos[w].proceso.id != PROCESO_MUERTO;
    int num_frames_proceso = cpu[i].cores[j].procesos[w].proceso.mm.num_frames;
    num_marcos_disponibles = num_marcos_disponibles + num_frames_proceso;
    for (int i = 0; i < num_frames_proceso; i++)
    {
        lista_marcos_disponibles[cpu[i].cores[j].procesos[w].proceso.mm.pgb[i]] = 0;
    }
    free(cpu[i].cores[j].procesos[w].proceso.mm.pgb);
    /*Esta parte solo se encarga de imprimir todos los procesos de las cpu cada vez que se elimina por completo un proceso*/
    /*for(int i=0;i<MAX_CPUS;i++){
        for(int j=0;j<CPU_CORES;j++){
            for(int w=0;w<CORE_THREADS;w++){
                //printf("Proceso: %d ---> QUANTUM: %d \n",cpu[i].cores[j].procesos[w].proceso.id,cpu[i].cores[j].procesos[w].proceso.quantum);
            }
        }
    }*/
}

void check_processes()
{
    /*Es una lista de listas, cada uno de los componentes de la lista tienes otra lista de longitud 3, en la que voy a guardar el valor de la
    posicion del proceso eliminado, para luego poder añadir otro en dicha posicion y no tener que recorrer otra vez toda las listas de procesos
    de los cores de los cpus*/
    int processes_to_add[CPU_CORES * MAX_CPUS * CORE_THREADS][3];
    int number_processes_add = 0;
    //printf("Ha entrado en Check_processes\n");
    /*---------------SOLO PRINTEA EL NUMERO DE PROCESOS QUE HAY EN LA LISTA DE ESPERA DE PROCESOS------------------*/
    int cont = 0;
    for (int g = 0; g < PRIORIDAD_MAX; g++)
    {
        for (int q = 0; q < MAX_PROCESS; q++)
        {
            if (lista_procesos_espera[g][q].id != -1)
            {
                cont = cont + 1;
            }
        }
    }
    //printf("LA LISTA DE ESPERA TIENE ---- %d ----- DISPONIBLES \n",cont);
    /*-------------------------------------------------------------------------------------------------------------*/
    for (int i = 0; i < MAX_CPUS; i++)
    {

        for (int j = 0; j < CPU_CORES; j++)
        {

            for (int w = 0; w < CORE_THREADS; w++)
            {

                if (cpu[i].cores[j].procesos[w].proceso.id != PROCESO_MUERTO)
                {

                    if (cpu[i].cores[j].procesos[w].proceso.quantum == 0)
                    {
                        remove_from_queue(i, j, w);
                        processes_to_add[number_processes_add][0] = i;
                        processes_to_add[number_processes_add][1] = j;
                        processes_to_add[number_processes_add][2] = w;
                        number_processes_add += 1;
                    }
                }
                else
                {
                    delete_process(i, j, w);
                }
            }
        }
    }
    //printf("NUMERO DE PROCESOS A ELIMINADOS O MOVIDOS -----> %d\n", number_processes_add);
    for (int i = 0; i < number_processes_add; i++)
    {
        add_process_to_queue(processes_to_add[i][0], processes_to_add[i][1], processes_to_add[i][2]);
    }
}

void *loader(void *arguments)
{
    struct arg_struct *args = arguments;
    //printf("El scheduler ha empezado a funcionar. Hilo %d \n",args -> arg1);
    while (1)
    {
        if (timer_ready == 1)
        {
            /*Cuando el timer le avisa, el scheduler procede a mirar los procesos*/
            //printf("El scheduler ha recibido la señal del timer: %d \n",timer_ready);
            pthread_mutex_lock(&mutex_ts);
            timer_ready = 0;
            pthread_mutex_unlock(&mutex_ts);
            pthread_mutex_lock(&mutex_clock_scheduler);
            check_processes();
            pthread_mutex_unlock(&mutex_clock_scheduler);
        }
    }
}

void imprimir()
{
    /*Imprimo la memoria*/
    long que_marco = 0x0000;
    printf("------|\n");
    printf("------|\n");
    for (long i = 0; i < 64 * SIZE; i = i + 4)
    {
        if (i % 256 == 0)
        {
            printf("Marco -----> %04lx\n", que_marco);
            printf("------|\n");
            printf("------|\n");
            que_marco = que_marco + 256;
        }
        long valor = memoria[i];
        printf("|------Posicion > %04lx --- Valor > %08lx\n", i, valor);
        printf("|");
    }
}