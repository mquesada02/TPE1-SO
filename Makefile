CC = gcc
CFLAGS = -Wall

app: app.c
	$(CC) $(CFLAGS) -o bin/app processes/app.c

clean:
	rm -f bin/app
