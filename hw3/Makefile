OUTPUT=shell
CFLAGS=-g -Wall -Wvla -fsanitize=address
LFLAGS=-lm -lreadline

%: %.c %.h
	gcc $(CFLAGS) -o $@ $< $(LFLAGS)

%: %.c
	gcc $(CFLAGS) -o $@ $< $(LFLAGS)

all: $(OUTPUT)

clean:
	rm -f *.o $(OUTPUT)
