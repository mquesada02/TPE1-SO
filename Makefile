CC = gcc
CFLAGS = -Wall -g

all: share slave

app.o: 
	$(CC) $(CFLAGS) -c -o binaries/app.o processes/app.c
shmADT.o:
	$(CC) $(CFLAGS) -c -o binaries/shmADT.o processes/shmADT.c
view.o:
	$(CC) $(CFLAGS) -c -o binaries/view.o processes/view.c
slave: 
	$(CC) $(CFLAGS) -o binaries/slave processes/slave.c
share: app.o shmADT.o view.o
	$(CC) -o binaries/app binaries/app.o binaries/shmADT.o
	$(CC) -o binaries/view binaries/view.o binaries/shmADT.o
	rm -f binaries/app.o
	rm -f binaries/view.o
	rm -f binaries/shmADT.o


clean:
	rm -f binaries/app
	rm -f binaries/slave
	rm -f binaries/view
	

