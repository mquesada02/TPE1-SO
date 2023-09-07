#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_FILE_LEN 100
#define MD5_LEN 32
#define MAX_PID_LEN 16
#define BUFFER_MAX_SIZE MAX_FILE_LEN+MD5_LEN+MAX_PID_LEN

#define MD5SUM_LEN 7

/* 
    funciona para cuando le llegan parámetros por primera vez, hay que hacer un pipe que envíe el
    padre el path de los archivos, y que detecte cuando haya algo en el fd, lo agarre (man select)
*/

int main(int argc, char* argv[]) {
    FILE * stream = NULL;
    char buffer[BUFFER_MAX_SIZE];
    char pidbuff[MAX_PID_LEN] = " - "; // empiezo a escribir en la posición 3 (zero indexed)
    for (int i=1,j; i<argc; i++){
        char cmd[MAX_FILE_LEN] = "md5sum ";
        for(j=0; argv[i][j] && MD5SUM_LEN+j < MAX_FILE_LEN; j++) {
            cmd[MD5SUM_LEN+j] = argv[i][j];
            buffer[j] = argv[i][j];
        }
        cmd[MD5SUM_LEN+j] = '\0';
        buffer[j++] = ' ';
        buffer[j++] = '-';
        buffer[j++] = ' ';
        // en buffer + j * sizeof(char) está la primera posición donde podría guardase el stream
        
        strcat(cmd," | cut -b -32"); // current command until this line: md5sum filename | cut -b -32
                                        // returns only md5hash + '\n'
        stream = popen(cmd,"r");
        fgets(buffer+j*sizeof(char),MD5_LEN+1,stream);
        // como termina en '\n', el fgets agrega un '\0' en la última posición
        // por lo tanto, el '\0' está en la posición (j+33) de buffer
        sprintf(pidbuff+3,"%d\n",getpid());
        strcat(buffer,pidbuff);
        write(1, buffer,BUFFER_MAX_SIZE);
        pclose(stream);
    }
    return 0;
}