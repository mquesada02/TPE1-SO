CC = gcc
CFLAGS = -Wall -g

all: app slave

app: 
	$(CC) $(CFLAGS) -o binaries/app processes/app.c
slave: 
	$(CC) $(CFLAGS) -o binaries/slave processes/slave.c

clean:
	rm -f binaries/app
	rm -f binaries/slave
