CC=gcc

FLAGS=-O3 -Wno-unused-result
LDFLAGS=-lpthread
#DEBUG=-DDEBUG
RESULT=-DRESULT

all: gol

gol: gol_template.c
	$(CC) $(DEBUG) $(RESULT) $(FLAGS) gol_template.c -o gol -g

clean:
	rm -rf gol
