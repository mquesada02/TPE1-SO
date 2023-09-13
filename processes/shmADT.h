#define RWXRWXRWX 0777
#define SHM_SIZE 1024
#define SEM_WC "/semaphore_wc"
#define SEM_MUTEX "/semaphore_mutex"

/*
recibe como argumento la direccion del buffer de la shm y un string (null-terminated) de lo que quiere escribir
el separador para la informacion escrita en el buffer es 0
*/
void write_data(char* buffer, char* data);

/*
recibe como argumento la direccion del buffer de la shm y un puntero a donde guardar la informacion leida
la guarda como un string null-terminated
*/
void read_data(char* buffer, char* data); 
