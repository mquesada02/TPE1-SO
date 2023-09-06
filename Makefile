CC = gcc
CFLAGS = -Wall

app: app.c
	$(CC) $(CFLAGS) -o binaries/app app.c

clean:
	rm -f binaries/app
