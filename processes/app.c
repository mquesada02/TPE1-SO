// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "../headers/shmADT.h"
#include "../headers/slaves.h"

#define SHM_NAME "/app_view_shm"
#define SHM_LEN 14
#define DEV_SHM_LEN (SHM_LEN + 15)

typedef struct pipefd {
        int slaveREADPipeFDs[2];
        int slaveWRITEPipeFDs[2];
        pid_t pid;
} pipefd;

int maximumFD = 0;
char * parameters[] = {"./binaries/slave", (char*)0};

shmADT createSHM(char* shm_name);

void closeForkedFDs(int slaveID, pipefd * slaves);

void createSlave(int slaveID, pipefd * slaves);

void createSlaves(int numberOfSlaves, pipefd * slaves);

int main(int argc, char* argv[]) {
    char initSHM[DEV_SHM_LEN];
    sprintf(initSHM,"rm -f /dev/shm/%s",SHM_NAME);
    system(initSHM);

    sem_t * write_count = sem_open(SEM_WC,O_CREAT,0777,0);

    int filesToProcess = argc-1;
    int numberOfSlaves = filesToProcess/SLAVE_INITIAL_CAPACITY;

    write(STDOUT_FILENO, SHM_NAME,SHM_LEN);

    sleep(2);

    pipefd slaves[numberOfSlaves];

    fd_set md5Ready;
    
    FD_ZERO(&md5Ready);

    createSlaves(numberOfSlaves,slaves); //crea y conecta los slaves necesarios

    FILE * output = fopen("./output.txt","w");// abro el archivo del output
    if (output==NULL) {
        perror("Cannot open the file.");
        exit(ERROR);
    }
        
    int argvIndex = 1;

    char md5buffer[BUFFER_MAX_SIZE];
    int len = 0;

    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        FD_SET(slaves[slaveID].slaveWRITEPipeFDs[READ],&md5Ready);
    }

    for (int slaveID=0; slaveID<numberOfSlaves && argvIndex<=filesToProcess;slaveID++) {
        for (int file=0; file<SLAVE_INITIAL_CAPACITY; file++) {
            dprintf(slaves[slaveID].slaveREADPipeFDs[WRITE],"%s\n",argv[argvIndex]);
            argvIndex++;
        }
    }

    shmADT buffer = openSHM(SHM_NAME);
    if(buffer != MAP_FAILED) {
        set_file_amount(buffer, filesToProcess);
    }
                   
    int filesProcessed = 0;

    while(filesProcessed<filesToProcess) { // hasta que se escribieron todos los files
        for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
            FD_SET(slaves[slaveID].slaveWRITEPipeFDs[READ],&md5Ready);
        }
        select(maximumFD+1,&md5Ready,NULL,NULL,NULL);
        
        for (int slaveID=0; slaveID < numberOfSlaves; slaveID++) {
            if (FD_ISSET(slaves[slaveID].slaveWRITEPipeFDs[READ],&md5Ready)) {
                len = read(slaves[slaveID].slaveWRITEPipeFDs[READ],md5buffer,BUFFER_MAX_SIZE);
                if (len<1 || len==BUFFER_MAX_SIZE) continue;
                md5buffer[len] = '\0';
                int k = len - 1;

                while (k >= 0) {
                    if (md5buffer[k--] == '\n') {
                        filesProcessed++;
                    }
                }
                fputs(md5buffer,output);
                if(buffer != MAP_FAILED) {
                    write_data(buffer,md5buffer);
                    sem_post(write_count);
                }
                if (argvIndex <= filesToProcess) {
                    dprintf(slaves[slaveID].slaveREADPipeFDs[WRITE],"%s\n",argv[argvIndex]);
                    argvIndex++;  
                }
            }
        }
    }
    fclose(output);
    
    /* ya no quedan más archivos. Cerramos los pipes */
    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        close(slaves[slaveID].slaveREADPipeFDs[WRITE]);
        close(slaves[slaveID].slaveREADPipeFDs[READ]);
    }

    return 0;
}



void createSlaves(int numberOfSlaves, pipefd * slaves) {
    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        createSlave(slaveID,slaves);
    }
    return;
}

void closeForkedFDs(int slaveID, pipefd * slaves) {
    for(int i=0;i<=slaveID;i++) {
        close(slaves[i].slaveREADPipeFDs[WRITE]);
        close(slaves[i].slaveWRITEPipeFDs[READ]);
        
    }
}

void createSlave(int slaveID, pipefd * slaves) {
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

            close(STDIN_FILENO);
            dup(slaves[slaveID].slaveREADPipeFDs[READ]);
            close(slaves[slaveID].slaveREADPipeFDs[READ]); //
            close(STDOUT_FILENO);
            dup(slaves[slaveID].slaveWRITEPipeFDs[WRITE]);  
            close(slaves[slaveID].slaveWRITEPipeFDs[WRITE]); //
            execv("./binaries/slave",parameters);
            exit(1);
            
        default:
            /* Parent's process */
            close(slaves[slaveID].slaveREADPipeFDs[READ]);
            close(slaves[slaveID].slaveWRITEPipeFDs[WRITE]);
            if (slaves[slaveID].slaveWRITEPipeFDs[READ]>maximumFD)
                maximumFD = slaves[slaveID].slaveWRITEPipeFDs[READ];
            if (slaves[slaveID].slaveREADPipeFDs[WRITE]>maximumFD)
                maximumFD = slaves[slaveID].slaveREADPipeFDs[WRITE];
            slaves[slaveID].pid = forkPID;

    }
}