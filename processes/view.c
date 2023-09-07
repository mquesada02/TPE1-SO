#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#define rwxrwxrwx 0777


#define BUFFERSIZE 50
#define PARAMETROS 1 
int main(int argc, char* argv[]){ //el parametro es el nombre de la memoria comparitda

     int shm_fd = shm_open(argv[1], O_RDWR, rwxrwxrwx);

      int pipefd[2]; 
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(-1);
    }
    pid_t appPid = fork();

    if (appPid == -1) {
        perror("fork");
        exit(-1);
    }

    if (appPid == 0) {  
        close(pipefd[1]); 
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execl("./vista", "vista", (char *)NULL);
        perror("Error al ejecutar vista");
        
    } else {  
        close(pipefd[0]); 
       
        pid_t viewPid = fork(); 

        if (viewPid == -1) {
            perror("fork");
            exit(-1);
        }

        if (viewPid == 0) { 
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
            execl("./md5", "md5", argv[1], (char *)NULL);
            perror("Error al ejecutar md5");
            exit(-1);
        } else { 
            
            wait(NULL);
            wait(NULL);
        }
    }

    

    return 0;
}
 
