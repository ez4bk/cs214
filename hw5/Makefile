CC = gcc
OUTPUT = client server
CFLAGS = -g -ggdb -Wall -Wvla -I inc -D_REENTRANT
LFLAGS =  -lSDL2 -lSDL2_image -lSDL2_ttf -ljpeg -lpthread

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(OUTPUT)

runclient: $(OUTPUT)
	LD_LIBRARY_PATH=lib ./client

client:
	$(CC) $(CFLAGS) client.c server.c -o client $(LFLAGS) -D_LIBSERVER

server: server.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)


clean:
	rm -f $(OUTPUT) *.o
