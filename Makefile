CC = gcc
CFLAGS = -Wall

app: app.c
	$(CC) $(CFLAGS) -o bin/app app.c

clean:
	rm -f bin/app
