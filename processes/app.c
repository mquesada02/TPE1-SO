#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SHMSIZE 1024
#define rwxrwxrwx 0777
#define SHMNAME "/app_view_shm"

char* createSHM(char* shm_name, int size);

int main(int argc, char* argv[]) {

    char* buffer = createSHM(SHMNAME, SHMSIZE);
    printf("%s", SHMNAME); //lo envio a la salida estandr -> view lo recibe por pipe, o por argumento
    
    pid_t pid;
    int pipefd[2];

    if (pipe(pipefd)==-1) {
        perror("pipe");
        exit(-1);
    }
    
    // falta pipe para pasarle los archivos, no pasarle argv directo
    // # de archivos: argc-1

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        /* Child process - c process */
        close(pipefd[0]); // close READ fd of pipe for CHILD
        close(1);         // close STDOUT for CHILD
        dup(pipefd[1]);   // copy WRITE fd of pipe at FD = 1 of CHILD (STDOUT)
        close(pipefd[1]); // close duplicate fd of pipe
        execve("./slave",argv, NULL);
        exit(1);
    } else {
        /* Parent process - p process */
        close(pipefd[1]); // close WRITE fd of pipe for PARENT
        close(0);         // close STDIN for PARENT
        dup(pipefd[0]);   // copy READ fd of pipe at FD = 0 of PARENT (STDIN) 
        close(pipefd[0]); // close duplicate fd of pipe
        wait(-1);         // wait for child processes to finish
        char buff[120];
        read(0,buff,120);
        printf("%s",buff);
    }


    return 0;
}

char* createSHM(char* shm_name, int size){
    int shm_fd = shm_open("sharedMem", O_CREAT | O_RDWR, rwxrwxrwx);
    ftruncate(shm_fd, SHMSIZE);
    return mmap(NULL, SHMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
}