#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "shmADT.h"

#define ERROR -1

#define SLAVE 0

typedef enum {READ, WRITE} mode;

#define SHM_NAME "/app_view_shm"

typedef struct pipefd {
        int slaveREADPipeFDs[2];
        int slaveWRITEPipeFDs[2];
        pid_t pid;
    } pipefd;

char* createSHM(char* shm_name, int size);

void closeForkedFDs(int slaveID, pipefd slaves[]);

void createSlave(int slaveID, pipefd slaves[]);

void createSlaves(int numberOfSlaves, pipefd slaves[]);

int main(int argc, char* argv[]) {

    char * buffer = createSHM(SHM_NAME, SHM_SIZE);
    sem_t * write_count = sem_open(SEM_WC, O_CREAT, S_IRWXG, 0);
    sem_t * mutex = sem_open(SEM_MUTEX, O_CREAT, S_IRWXG, 1);
    wait(2);
    printf("%s", SHM_NAME); //lo envio a la salida estandr -> view lo recibe por pipe, o por argumento
                            //select?


    int filesToProcess = argc-1;                    // cantidad de archivos
    int numberOfSlaves = filesToProcess/4+1;        // número elegido arbitrariamente

    pipefd slaves[numberOfSlaves];
    
    createSlaves(numberOfSlaves,slaves); // crea y conecta los slaves necesarios

    write(slaves[0].slaveREADPipeFDs[WRITE],argv[argc-1],32);
    char buff[128];
    read(slaves[0].slaveWRITEPipeFDs[READ],buff,128);
    write(STDOUT_FILENO,buff,128);


    // select for slaveWRITEPipeFDs and maybe slaveREADPipeFDs (consumed?)


    return 0;
}


char* createSHM(char* shm_name, int size){
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, RWXRWXRWX);
    ftruncate(shm_fd, size);
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
}


void createSlaves(int numberOfSlaves, pipefd slaves[]) {
    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        createSlave(slaveID,slaves);
    }
    return;
}

void closeForkedFDs(int slaveID, pipefd slaves[]) {
    for(int i=0;i<slaveID;i++) {
        close(slaves[i].slaveREADPipeFDs[WRITE]);
        close(slaves[i].slaveWRITEPipeFDs[READ]);
    }
}

void createSlave(int slaveID, pipefd slaves[]) {
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
            close(slaves[slaveID].slaveREADPipeFDs[WRITE]);
            close(slaves[slaveID].slaveWRITEPipeFDs[READ]);

            close(STDOUT_FILENO);
            dup(slaves[slaveID].slaveWRITEPipeFDs[WRITE]);
            close(STDIN_FILENO);
            dup(slaves[slaveID].slaveREADPipeFDs[READ]);

            execve("./slave",NULL,NULL);
            exit(1);
            break;
        default:
            /* Parent's process */
            close(slaves[slaveID].slaveREADPipeFDs[READ]);
            close(slaves[slaveID].slaveWRITEPipeFDs[WRITE]);
            slaves[slaveID].pid = forkPID;
            /* le manda la carga inicial de archivos */
            // write(slaveREADPipeFDs[WRITE][slaveID],argv[argc-1],32);
            // char buffer[128];
            // read(slaveWRITEPipeFDs[READ][slaveID],buffer,128);
            // printf(buffer);
    }
    return;
}