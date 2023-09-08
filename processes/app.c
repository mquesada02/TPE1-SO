#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <fcntl.h>

#define ERROR -1

#define SLAVE 0

typedef enum {READ, WRITE} mode;

#define SHM_NAME "/app_view_shm"
#define SHM_SIZE 1024
#define rwxrwxrwx 0777



char* createSHM(char* shm_name, int size);

void createPipes(int numberOfSlaves, int *slaveREADPipeFDs[], int *slaveWRITEPipeFDs[]);

void createSlave(int slaveID, int *slaveREADPipeFDs[], int *slaveWRITEPipeFDs[], pid_t * slavesPIDs);

int main(int argc, char* argv[]) {

    char* buffer = createSHM(SHM_NAME, SHM_SIZE);
    printf("%s", SHM_NAME); //lo envio a la salida estandr -> view lo recibe por pipe, o por argumento
                           // select?
    
    int filesToProcess = argc-1;                    // cantidad de archivos
    int numberOfSlaves = filesToProcess/4+1;        // número elegido arbitrariamente

    pid_t slavesPIDs[numberOfSlaves];               // PIDs de los esclavos a generar

    int slaveREADPipeFDs[2][numberOfSlaves];        // pipe que conecta al STDIN del slave
    int slaveWRITEPipeFDs[2][numberOfSlaves];       // pipe que conecta al STDOUT del slave

    
    createPipes(numberOfSlaves,slaveREADPipeFDs,slaveWRITEPipeFDs);             // crea y cierra los pipes necesarios
    createSlaves(numberOfSlaves,slaveREADPipeFDs,slaveWRITEPipeFDs,slavesPIDs); // crea y conecta los slaves necesarios

    // select for slaveWRITEPipeFDs and maybe slaveREADPipeFDs (consumed?)


    return 0;
}

char* createSHM(char* shm_name, int size){
    int shm_fd = shm_open("sharedMem", O_CREAT | O_RDWR, rwxrwxrwx);
    ftruncate(shm_fd, size);
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
}
/*  Crea 2 pipes para el READ y WRITE amount veces. Cierra ambos FDs en los dos casos. */
void createPipes(int amount, int *READ[], int *WRITE[]) {
    for(int i=0; i<amount;i++) {
        if (pipe(READ[i])==ERROR || pipe(WRITE[i])==ERROR) {
            perror("Pipe error.");
            exit(ERROR);
        }
        close(READ[i][0]);
        close(READ[i][1]);
        close(WRITE[i][0]);
        close(WRITE[i][1]); 
    }
    return;
}

/* Conecta el pipe en el modo MODE (READ / WRITE) en el fd myfd del proceso */
void linkPipe(int myfd, int pipeid, int *pipes[], mode MODE) {
    dup2(myfd,pipes[MODE][pipeid]);
    return;
}

void createSlaves(int numberOfSlaves, int *slaveREADPipeFDs[], int *slaveWRITEPipeFDs[], pid_t * slavesPIDs) {
    for(int slaveID=0;slaveID<numberOfSlaves;slaveID++) {
        createSlave(slaveID,slaveREADPipeFDs,slaveWRITEPipeFDs,slavesPIDs);
    }
    return;
}

void createSlave(int slaveID, int *slaveREADPipeFDs[], int *slaveWRITEPipeFDs[], pid_t * slavesPIDs) {
    pid_t forkPID = fork();
    switch(forkPID) {
        case ERROR:
            perror("Fork error.");
            exit(ERROR);
        case SLAVE:
            /* Slave's process */
            linkPipe(STDOUT_FILENO, slaveID, slaveWRITEPipeFDs, WRITE);
            linkPipe(STDIN_FILENO, slaveID, slaveREADPipeFDs,READ);
            break;
        default:
            /* Parent's process */
            slavesPIDs[slaveID] = forkPID;
    }
    return;
}