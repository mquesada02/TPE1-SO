#include "shmADT.h"

void write_data(char* buffer, char* data){ //data es un string null terminated
    static int write_offset = 0;
    while (*data){
        *(buffer+write_offset)= *data;
        data++;
        write_offset++; //error si se pasa del tamaño
    }
    *(buffer+write_offset) = '\0';
    write_offset++;
}

void read_data(char* buffer, char* data){
    static int read_offset = 0;
    while (*(buffer+read_offset)){
        *data = *(buffer+read_offset);
        data++;
        read_offset++;
    }
    *data = '\0';
    read_offset++;
}

