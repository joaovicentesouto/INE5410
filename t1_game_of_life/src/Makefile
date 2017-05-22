CC=gcc

FLAGS=-O3 -Wno-unused-result
LDFLAGS=-lpthread -lm
#DEBUG=-DDEBUG
RESULT=-DRESULT

all: gol

gol: gol.c
	$(CC) $(DEBUG) $(RESULT) $(FLAGS) gol.c -o gol $(LDFLAGS)

clean:
	rm -rf gol
