// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../headers/slaves.h"

/* 
    funciona para cuando le llegan parámetros por primera vez, hay que hacer un pipe que envíe el
    padre el path de los archivos, y que detecte cuando haya algo en el fd, lo agarre (man select)
*/

// slave lo toma mal idk why

int main(int argc, char* argv[]) {
    FILE * stream = NULL;
    char * fileName = NULL;
    char md5Buffer[MD5_LEN + 1];
    char outBuffer[BUFFER_MAX_SIZE];
    int len = 0;
    char cmd[MAX_FILE_LEN] = "md5sum ";
    ssize_t size;
    while ((len = getline(&fileName, &size, stdin)) != EOF) {

        fileName[len-1] = '\0';

        // {fileName} - {md5hash} - {pid}
        // %s - %s - %d
        
        // md5sum {fileName} | cut -b -32
        
        char buff[BUFFER_MAX_SIZE] = "";
        strcat(buff,cmd);
        strcat(buff, fileName);
        strcat(buff," | cut -b -32");
        
        stream = popen(buff,"r");   
        md5Buffer[MD5_LEN] = '\0';
        fgets(md5Buffer,MD5_LEN+1,stream);
        
        pclose(stream);
        

        // como termina en '\n', el fgets agrega un '\0' en la última posición
        // por lo tanto, el '\0' está en la posición (j+33) de buffer
        len = sprintf(outBuffer,"%s - %s - %d\n",fileName,md5Buffer,getpid());
        write(STDOUT_FILENO, outBuffer,len);
        free(fileName);
        fileName = NULL;
    }
    
    return 0;
}

