CC = gcc
CFLAGS = -g -Wall -Wvla -fsanitize=address
LFLAGS = -lm
BD = build

bin = memperf

all: $(bin)

rebuild: clean all

# EXECUTABLE 1

obj = memperf.o mymalloc.o
objbd = $(obj:%=$(BD)/%)

$(bin): $(objbd)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

# CLEAN UP

clean:
	rm -rf "$(bin)" "$(BD)"

# DEPENDENCIES

$(BD)/memperf.o: mymalloc.h
$(BD)/mymalloc.o: mymalloc.h

# COMPILE TO OBJECTS

$(BD)/%.o: %.c
	@mkdir -p "$(BD)"
	$(CC) $(CFLAGS) -c -o $@ $<

