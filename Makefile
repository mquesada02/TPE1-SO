CC = gcc
CFLAGS = -Wall -g

all: app shmADT slave share

app: 
	$(CC) $(CFLAGS) -c -o binaries/app.o processes/app.c
shmADT:
	$(CC) $(CFLAGS) -c -o binaries/shmADT.o processes/shmADT.c
slave: 
	$(CC) $(CFLAGS) -o binaries/slave processes/slave.c
share:
	$(CC) -o binaries/app binaries/app.o binaries/shmADT.o
	rm -f binaries/app.o
	rm -f binaries/shmADT.o

tester:
	$(CC) $(CFLAGS) -o binaries/tester processes/tester.c
	$(CC) $(CFLAGS) -o binaries/slave processes/slave.c

clean:
	rm -f binaries/app
	rm -f binaries/slave
	rm -f binaries/tester
	

