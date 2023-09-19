#define SHM_SIZE 4096
#define SEM_WC "/semaphore_wc"

typedef struct shmCDT* shmADT;

shmADT createSHM(char* shm_name);

shmADT openSHM(char* shm_name);

int closeSHM(shmADT buffer);

void set_file_amount(shmADT buffer, int amount);

void file_read(shmADT buffer);

int get_files_left(shmADT buffer);

/*
recibe como argumento la direccion del buffer de la shm y un string (null-terminated) de lo que quiere escribir
el separador para la informacion escrita en el buffer es 0
*/
void write_data(shmADT buffer, char* data);

/*
recibe como argumento la direccion del buffer de la shm, lee de la shared memory el string al que se esta apuntando, 
e imprime en salida estandar lo que leyo
*/
void read_data(shmADT buffer); 
