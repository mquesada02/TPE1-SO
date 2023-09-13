#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "../headers/slaves.h"

#define SHM_NAME "/app_view_shm"
#define SHM_SIZE 1024
#define rwxrwxrwx 0777

typedef struct pipefd {
        int slaveREADPipeFDs[2];
        int slaveWRITEPipeFDs[2];
        pid_t pid;
} pipefd;

int maximumFD = 0;

char* createSHM(char* shm_name, int size);

void closeForkedFDs(int slaveID, pipefd slaves[]);

void createSlave(int slaveID, pipefd slaves[], fd_set * slaveOUT, fd_set * slaveIN);

void createSlaves(int numberOfSlaves, pipefd slaves[], fd_set * slaveOUT, fd_set * slaveIN);

int main(int argc, char* argv[]) {

    //char* buffer = createSHM(SHM_NAME, SHM_SIZE);
    //printf("%s", SHM_NAME); //lo envio a la salida estandr -> view lo recibe por pipe, o por argumento
                           // select?
    int filesToProcess = argc-1;                    // cantidad de archivos
    int numberOfSlaves = filesToProcess/4+1;        // número elegido arbitrariamente

    pipefd slaves[numberOfSlaves];

    fd_set md5Ready;
    fd_set filesReady;

    FD_ZERO(&md5Ready);
    FD_ZERO(&filesReady);

    createSlaves(numberOfSlaves,slaves, &md5Ready, &filesReady); // crea y conecta los slaves necesarios

    FILE * output = fopen("../output.txt","a+");// abro el archivo del output
    int argvIndex = 1;

    /* Enviamos la carga inicial. Por cada slave, enviamos SLAVE_INITIAL_CAPACITY archivos (no va a enviar
       todos, y aumenta el argvIndex)
     */
    for (int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        for(int file=0;file<SLAVE_INITIAL_CAPACITY;file++) {
            write(slaves[slaveID].slaveREADPipeFDs[WRITE],argv[argvIndex++],MAX_FILE_LEN);
        }
    }
    char md5buffer[BUFFER_MAX_SIZE];
    while(argvIndex<argc) {

        select(maximumFD,&md5Ready,&filesReady,NULL,NULL); // sleep until fd ready (read or write)
        /* hay en total numberOfSlaves fds en cada fd_set */
        for(int slaveID=0;slaveID<numberOfSlaves && argvIndex<argc;slaveID++) {
            if (FD_ISSET(slaves[slaveID].slaveWRITEPipeFDs[READ],&md5Ready)) {
                /* el slave envió el md5 */
                read(slaves[slaveID].slaveWRITEPipeFDs[READ],md5buffer,BUFFER_MAX_SIZE);
                fwrite(md5buffer,sizeof(char),BUFFER_MAX_SIZE,output);
            }

            if (FD_ISSET(slaves[slaveID].slaveREADPipeFDs[WRITE],&filesReady)) {
                /* el slave ya consumió todos los archivos enviados. Le enviamos uno nuevo */
                write(slaves[slaveID].slaveREADPipeFDs[WRITE],argv[argvIndex++],MAX_FILE_LEN);
            }   
        }
    }
    /* ya no quedan más archivos. Cerramos los pipes */
    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        close(slaves[slaveID].slaveREADPipeFDs[WRITE]);
    }
    fclose(output);

    return 0;
}

char* createSHM(char* shm_name, int size){
    int shm_fd = shm_open("sharedMem", O_CREAT | O_RDWR, rwxrwxrwx);
    ftruncate(shm_fd, size);
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
}



void createSlaves(int numberOfSlaves, pipefd slaves[], fd_set * slaveOUT, fd_set * slaveIN) {
    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        createSlave(slaveID,slaves,slaveOUT,slaveIN);
    }
    return;
}

void closeForkedFDs(int slaveID, pipefd slaves[]) {
    for(int i=0;i<slaveID;i++) {
        close(slaves[i].slaveREADPipeFDs[WRITE]);
        close(slaves[i].slaveWRITEPipeFDs[READ]);
    }
}

void createSlave(int slaveID, pipefd slaves[], fd_set * slaveOUT, fd_set * slaveIN) {
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
            FD_SET(slaves[slaveID].slaveWRITEPipeFDs[READ],slaveOUT);
            FD_SET(slaves[slaveID].slaveREADPipeFDs[WRITE],slaveIN);
            if (slaves[slaveID].slaveWRITEPipeFDs[READ]>maximumFD)
                maximumFD = slaves[slaveID].slaveWRITEPipeFDs[READ];
            slaves[slaveID].pid = forkPID;

    }
    return;
}