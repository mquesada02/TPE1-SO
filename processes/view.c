// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "../headers/shmADT.h"

#define PARAMETROS 1 
#define LINESIZE 100
#define ERROR -1
int main(int argc, char* argv[]){ //los parametros son el nombre de la memoria comparitda y el tamaño

    shmADT buffer = openSHM(argv[1]);
    if (buffer == MAP_FAILED) {
        perror("Error accessing shared memory");
        exit(ERROR);
    }
    //sem_t * write_count = sem_open(SEM_WC, 0);
    
    //sem_t * mutex = sem_open(SEM_MUTEX, 0);
    //sem_t * mutex_rc = 1; //para que un read a la vez modifique rc (read count) y no se interrumpa justo entre la modificacion y el if

    char data[LINESIZE];

    //no es lo que normalmente pasa con readers y writers, pero en este caso en particular
    //el writer nunca vuelve a escribir en el mismo lugar de memoria (va avanzando un puntero
    //y llenando el buffer), y el reader no accede a ningun lugar de la memoria que no este
    //escrito, por lo que reader y writer nunca se podrian "chocar" y no habria problema en
    //que se lea y escriba en la memoria compartida a la vez
    //lo que si generaria problema es que se quiera leer cuando no hay nada escrito, o que dos
    //procesos quieran leer a la vez o dos quieran escribir a la vez, porque estarian levantando
    //o escribiendo con el puntero informacion intercalada (aunque en este caso hay solo un proceso
    //que escribe y uno que lee, por lo que el problema no se generaria)
    setvbuf(stdout,NULL,_IONBF,SHM_SIZE);
    while (get_files_left(buffer)){ //hay solo un proceso que lee de la shm, entonces no habria problemas de intercalacion creeria
        //read_data(buffer, data);
        printf("%s\n", data);
        file_read(buffer);
    }

    return 0;
}
 
