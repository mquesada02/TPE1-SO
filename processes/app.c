#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include "shmADT.h"

#include "../headers/slaves.h"

#define SHM_NAME "/app_view_shm"

typedef struct pipefd {
        int slaveREADPipeFDs[2];
        int slaveWRITEPipeFDs[2];
        pid_t pid;
} pipefd;

int maximumFD = 0;
char * parameters[] = {"./slave","\0"};

shmADT createSHM(char* shm_name);

void closeForkedFDs(int slaveID, pipefd * slaves);

void createSlave(int slaveID, pipefd * slaves, fd_set * slaveOUT, fd_set * slaveIN);

void createSlaves(int numberOfSlaves, pipefd * slaves, fd_set * slaveOUT, fd_set * slaveIN);

int main(int argc, char* argv[]) {

    shmADT buffer = createSHM(SHM_NAME);
    if (buffer == MAP_FAILED) {
        perror("Error creating shared memory");
        exit(ERROR);
    }
    sem_t * write_count = sem_open(SEM_WC, O_CREAT, S_IRWXG, 0);
    //sem_t * mutex = sem_open(SEM_MUTEX, O_CREAT, S_IRWXG, 1);
    wait(2);
    printf("%s", SHM_NAME); //lo envio a la salida estandr -> view lo recibe por pipe, o por argumento
                            //select?


    int filesToProcess = argc-1;                    // cantidad de archivos
    int numberOfSlaves = filesToProcess/4+1;        // número elegido arbitrariamente

    set_file_amount(buffer, filesToProcess);

    pipefd slaves[numberOfSlaves];

    fd_set md5Ready;
    fd_set filesReady;
    
    FD_ZERO(&md5Ready);
    FD_ZERO(&filesReady);

    createSlaves(numberOfSlaves,slaves, &md5Ready, &filesReady); // crea y conecta los slaves necesarios

    FILE * output = fopen("../output.txt","w");// abro el archivo del output
    int argvIndex = 1;

    /* Enviamos la carga inicial. Por cada slave, enviamos SLAVE_INITIAL_CAPACITY archivos (no va a enviar
       todos, y aumenta el argvIndex)
     */

    
    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        for(int file=0; file<SLAVE_INITIAL_CAPACITY;file++) {
            write(slaves[slaveID].slaveREADPipeFDs[WRITE],argv[argvIndex],strlen(argv[argvIndex])+1);
            argvIndex++;
        }
    }

    char md5buffer[BUFFER_MAX_SIZE];

    /*
    for(int i=0;i<numberOfSlaves;i++) {
        fcntl(slaves[i].slaveWRITEPipeFDs[READ], F_SETFL, fcntl(slaves[i].slaveWRITEPipeFDs[READ], F_GETFL, 0) | O_NONBLOCK);
        fcntl(slaves[i].slaveREADPipeFDs[WRITE], F_SETFL, fcntl(slaves[i].slaveREADPipeFDs[WRITE], F_GETFL, 0) | O_NONBLOCK);
    }

    
    while(argvIndex<=filesToProcess) {
        for(int i=0;i<numberOfSlaves;i++) {
            read(slaves[i].slaveWRITEPipeFDs[READ],md5buffer,BUFFER_MAX_SIZE);
            fputs(md5buffer,output);
            if (argvIndex<=filesToProcess) {
                write(slaves[i].slaveREADPipeFDs[WRITE],argv[argvIndex],strlen(argv[argvIndex])+1);
                argvIndex++;
            }
        }
    }
    */

    
    while(argvIndex<=filesToProcess) { // hasta que se escribieron todos los files
        for (int i=0;i<numberOfSlaves;i++) {
            FD_SET(slaves[i].slaveWRITEPipeFDs[READ],&md5Ready);
            FD_SET(slaves[i].slaveREADPipeFDs[WRITE],&filesReady);
        }
        select(maximumFD,&md5Ready,&filesReady,NULL,NULL); // sleep until fd ready (read or write)
        // hay en total numberOfSlaves fds en cada fd_set 
        for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
            if (FD_ISSET(slaves[slaveID].slaveWRITEPipeFDs[READ],&md5Ready)) {
                // el slave envió el md5 
                read(slaves[slaveID].slaveWRITEPipeFDs[READ],md5buffer,BUFFER_MAX_SIZE);
                fputs(md5buffer,output);
            }
            if (FD_ISSET(slaves[slaveID].slaveREADPipeFDs[WRITE],&filesReady) && argvIndex<=filesToProcess) {
                write(slaves[slaveID].slaveREADPipeFDs[WRITE],argv[argvIndex],strlen(argv[argvIndex])+1);
                argvIndex++;
            }
            
        }
    }
    
    
    /* ya no quedan más archivos. Cerramos los pipes */
    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        close(slaves[slaveID].slaveREADPipeFDs[READ]);
        close(slaves[slaveID].slaveREADPipeFDs[WRITE]);
    }
    fclose(output);
    int close_ret = closeSHM(buffer);
    if (close_ret < 0){
        perror("Error cerrando la memoria compartida");
        exit(ERROR);
    }

    return 0;
}



void createSlaves(int numberOfSlaves, pipefd * slaves, fd_set * slaveOUT, fd_set * slaveIN) {
    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        createSlave(slaveID,slaves,slaveOUT,slaveIN);
    }
    return;
}

void closeForkedFDs(int slaveID, pipefd * slaves) {
    for(int i=0;i<slaveID;i++) {
        close(slaves[i].slaveREADPipeFDs[WRITE]);
        close(slaves[i].slaveWRITEPipeFDs[READ]);
    }
}

void createSlave(int slaveID, pipefd * slaves, fd_set * slaveOUT, fd_set * slaveIN) {
    if (pipe(slaves[slaveID].slaveREADPipeFDs)==ERROR || pipe(slaves[slaveID].slaveWRITEPipeFDs)==ERROR) {
            perror("Pipe error.");
            exit(ERROR);
    }

    pid_t forkPID = fork();
    switch(forkPID) {
        case ERROR:
            perror("Fork error.");
            exit(ERROR);
        case SLAVE:
            /* Slave's process */
            closeForkedFDs(slaveID, slaves);
            //close(slaves[slaveID].slaveREADPipeFDs[WRITE]);
            //close(slaves[slaveID].slaveWRITEPipeFDs[READ]);

            close(STDIN_FILENO);
            dup(slaves[slaveID].slaveREADPipeFDs[READ]);
            close(STDOUT_FILENO);
            dup(slaves[slaveID].slaveWRITEPipeFDs[WRITE]);

            execv("slave",parameters);
            exit(1);
            
        default:
            /* Parent's process */
            close(slaves[slaveID].slaveREADPipeFDs[READ]);
            close(slaves[slaveID].slaveWRITEPipeFDs[WRITE]);
            FD_SET(slaves[slaveID].slaveWRITEPipeFDs[READ],slaveOUT);
            if (slaves[slaveID].slaveWRITEPipeFDs[READ]>maximumFD)
                maximumFD = slaves[slaveID].slaveWRITEPipeFDs[READ];
            FD_SET(slaves[slaveID].slaveREADPipeFDs[WRITE],slaveIN);
            if (slaves[slaveID].slaveREADPipeFDs[WRITE]>maximumFD)
                maximumFD = slaves[slaveID].slaveREADPipeFDs[WRITE];
            slaves[slaveID].pid = forkPID;

    }
    return;
}