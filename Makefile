CC = gcc
CFLAGS = -Wall

app: app.c
	$(CC) $(CFLAGS) -o app app.c

clean:
	rm -f app
