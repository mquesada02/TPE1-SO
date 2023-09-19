
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SLAVE 0
#define ERROR -1

char * parameters[] = {"/home/mquesada/Documents/ITBA/SO/TPE1-SO/binaries/slave","\0"};
int maximumFD = 0;

typedef enum {READ,WRITE};

typedef struct pipefd {
        int slaveREADPipeFDs[2];
        int slaveWRITEPipeFDs[2];
        pid_t pid;
} pipefd;

void createSlave(pipefd * slave) {
    if (pipe(slave->slaveREADPipeFDs)==ERROR || pipe(slave->slaveWRITEPipeFDs)==ERROR) {
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
            //closeForkedFDs(slaveID, slaves);
            //close(slaves[slaveID].slaveREADPipeFDs[WRITE]);
            //close(slaves[slaveID].slaveWRITEPipeFDs[READ]);

            close(STDIN_FILENO);
            dup(slave->slaveREADPipeFDs[READ]);
            close(STDOUT_FILENO);
            dup(slave->slaveWRITEPipeFDs[WRITE]);

            execv("./binaries/slave",parameters);
            exit(1);
            
        default:
            /* Parent's process */
            //close(slaves[slaveID].slaveREADPipeFDs[READ]);
            //close(slaves[slaveID].slaveWRITEPipeFDs[WRITE]);
            if (slave->slaveWRITEPipeFDs[READ]>maximumFD)
                maximumFD = slave->slaveWRITEPipeFDs[READ];
            if (slave->slaveREADPipeFDs[WRITE]>maximumFD)
                maximumFD = slave->slaveREADPipeFDs[WRITE];

    }
}



int main() {

    pipefd slave;
    int len = 0;
    char buffer[150];
    createSlave(&slave);

    write(slave.slaveREADPipeFDs[WRITE],"./files/file1",14);
    len = read(slave.slaveWRITEPipeFDs[READ],buffer,150);
    buffer[len] = '\0';
    printf("%s\n",buffer);

    write(slave.slaveREADPipeFDs[WRITE],"./files/file2",14);
    len = read(slave.slaveWRITEPipeFDs[READ],buffer,150);
    buffer[len] = '\0';
    printf("%s\n",buffer);

    write(slave.slaveREADPipeFDs[WRITE],"./files/file3",14);
    len = read(slave.slaveWRITEPipeFDs[READ],buffer,150);
    buffer[len] = '\0';
    printf("%s\n",buffer);

    write(slave.slaveREADPipeFDs[WRITE],"./files/file4",14);
    len = read(slave.slaveWRITEPipeFDs[READ],buffer,150);
    buffer[len] = '\0';
    printf("%s\n",buffer);

    return 0;
}


