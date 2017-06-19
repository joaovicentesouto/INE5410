/*
* The Game of Life
*
* a cell is born, if it has exactly three neighbours
* a cell dies of loneliness, if it has less than two neighbours
* a cell dies of overcrowding, if it has more than three neighbours
* a cell survives to the next generation, if it does not die of loneliness
* or overcrowding
*
* In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
* The game plays a number of steps (given by the input), printing to the screen each time.  'x' printed
* means on, space means off.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
typedef unsigned char cell_t;

/* Functions performed by the master */
void print(cell_t * board, int size);
void read_file(FILE * f, cell_t * board, int size);

/* Functions performed by the slaves */
int adjacent_to(cell_t * board, int size, int i, int j);
void play(cell_t * board, cell_t * newboard, int size);

int main () {
  int processes, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    /* 1: Alocar memória e ler o tabuleiro */
    int size, steps;
    FILE *f;
    f = stdin;
    fscanf(f, "%d %d", &size, &steps);

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&steps, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /* Avisa para os filhos os tamanho das matrizes deles */
    float precision = size/processes;
    int lines = (int) precision;
    lines += precision - integer_part <= 0.5 ? 0 : 1;
    MPI_Bcast(&lines, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Ultimo processo pode ter linhas a mais ou a menos.
    int last_lines += line + (size - line*(processes-1));
    MPI_Send(&last_lines, 1, MPI_INT, processes-1, 0, MPI_COMM_WORLD);

    cell_t * prev = (cell_t *) malloc(sizeof(cell_t) * size * size);

    read_file (f, prev, size);
    fclose(f);

    #ifdef DEBUG
    printf("Initial:\n");
    print(prev, size);
    #endif
    /* Fim 1  */

    /* 2: Separação de trabalho e envio para os escravos */
    // Primeiro
    MPI_Send(prev, (lines+1)*size, MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD);

    // Intermediários: i => n# processo-1
    int i;
    for (i = 1; i < processes-2; i++)
      MPI_Send((prev + (i*lines-1)*size), (lines+2)*size, MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD);

    // Último
    MPI_Send((prev + (i*lines-1)*size), (last_lines+1)*size, MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD);
    /* Fim 2 */

    /* 3: Espera pelo cálculo, imprime resultado e desaloca memória */
    MPI_Recv(prev, (lines*size), MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD, NULL);

    // Intermediários: i => n# processo-1
    for (i = 1; i < processes-2; i++)
      MPI_Recv((prev + i*lines*size), (lines*size), MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD, NULL);

    // Último
    MPI_Recv((prev + i*lines*size), (last_lines*size), MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD, NULL);

    #ifdef RESULT
    printf("Final:\n");
    print (prev, size);
    #endif

    free(prev);
    free(next);
    /* Fim 3 */

  } else {
    /* 1: Recebe tamanhos e alocagem de memória */
    int size, steps, lines;
    cell_t *prev, *next, *tmp;
    MPI_Recv(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
    MPI_Recv(&steps, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
    MPI_Recv(&lines, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);

    /** MUITAS PARTES IGUAIS, DA PRA ABSTRAIR??? **/

    if (rank == 1) {
      prev = (cell_t *) malloc(sizeof(cell_t) * (lines+1) * size);
      next = (cell_t *) malloc(sizeof(cell_t) * (lines+1) * size);
      MPI_Recv(prev, (lines+1)*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, NULL);

      for (int i = 0; i < steps; ++i) {
        play(prev, next, 0, lines, size);

        tmp = next;
        next = prev;
        prev = tmp;

        MPI_Send((prev + (lines-1)*size), size, MPI_UNSIGNED_CHAR, 2, 0, MPI_COMM_WORLD);
        MPI_Recv((prev + (lines-1)*size), size, MPI_UNSIGNED_CHAR, 2, 0, MPI_COMM_WORLD, NULL);
        // irecv???
      }

      // devolve resultado
      MPI_Recv(prev, lines*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, NULL);

    } else of (rank == processes-1) {
      MPI_Recv(&lines, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
      prev = (cell_t *) malloc(sizeof(cell_t) * (lines+1) * size);
      next = (cell_t *) malloc(sizeof(cell_t) * (lines+1) * size);
      MPI_Recv(prev, (lines+1)*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, NULL);

      for (int i = 0; i < steps; ++i) {
        play(prev, next, 1, lines, size);

        tmp = next;
        next = prev;
        prev = tmp;

        MPI_Send(prev, size, MPI_UNSIGNED_CHAR, rank-1, 0, MPI_COMM_WORLD);
        MPI_Recv(prev, size, MPI_UNSIGNED_CHAR, rank-1, 0, MPI_COMM_WORLD, NULL);
        // irecv???
      }

      // devolve resultado
      MPI_Recv(prev, lines*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, NULL);

    } else {
      prev = (cell_t *) malloc(sizeof(cell_t) * (lines+2) * size);
      next = (cell_t *) malloc(sizeof(cell_t) * (lines+2) * size);
      MPI_Recv(prev, (lines+2)*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, NULL);

      for (int i = 0; i < steps; ++i) {
        play(prev, next, 1, lines, size);

        tmp = next;
        next = prev;
        prev = tmp;

        MPI_Send((prev + (lines-1)*size), size, MPI_UNSIGNED_CHAR, rank-1, 0, MPI_COMM_WORLD);
        MPI_Send(prev, size, MPI_UNSIGNED_CHAR, rank+1, 0, MPI_COMM_WORLD);
        MPI_Recv(prev, size, MPI_UNSIGNED_CHAR, rank+1, 0, MPI_COMM_WORLD, NULL);
        MPI_Recv((prev + (lines-1)*size), size, MPI_UNSIGNED_CHAR, rank-1, 0, MPI_COMM_WORLD, NULL);
        // irecv???
      }

      // devolve resultado
      MPI_Recv(prev, lines*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, NULL);

    }
    /* Fim 1 */

    /*for (int i = 0; i < steps; ++i) {
      play (prev,next,size);

      #ifdef DEBUG
      printf("%d ----------\n", i + 1);
      print (next,size);
      #endif

      tmp = next;
      next = prev;
      prev = tmp;
    }*/

  }

  MPI_Finalize();
  return 0;
}

/* return the number of on cells adjacent to the i,j cell */
int adjacent_to(cell_t * board, int size, int i, int j) {
  // Fonte: João Meyer
  int count = 0,
      linha = i*size,
      linha_prox = linha + size;

  if (i-1 > 0) {
      int linha_ant = linha - size;
      count += board[linha_ant + j];
      if (j - 1 > 0)
          count += board[linha_ant + j-1];
      if (j+1 < size)
          count += board[linha_ant + j+1];
  }

  if (j-1 > 0) {
      count += board[linha + j-1];
      if (i+1 < size)
          count += board[linha_prox + j-1];
  }

  if (i+1 < size) {
      count += board[linha_prox + j];
      if (j+1 < size)
          count += board[linha_prox + j+1];
  }

  if (j+1 < size)
      count += board[linha + j+1];

  return count;
}

void play(cell_t * board, cell_t * newboard, int beg, int end, int size) {
  int	a, position;
  /* for each cell, apply the rules of Life */
  for (int i = beg; i <= end; ++i) {
    for (int j = 0; j < size; ++j) {
      position = i*size + j;
      a = adjacent_to (board, size, i, j);
      // switch is better???
      if (a == 2)
        newboard[position] = board[position];
      else if (a == 3)
        newboard[position] = 1;
      else if (a < 2)
        newboard[position] = 0;
      else //if (a > 3)
        newboard[position] = 0;
    }
  }
}

/* print the life board */
void print(cell_t * board, int size) {
  for (int j = 0; j < size; ++j) {  // For each row
    for (int i = 0; i < size; ++i)  // Print each column position...
      printf ("%c", board[i*size + j] ? 'x' : ' ');
    printf ("\n");  // End line
  }
}

/* read a file into the life board */
void read_file(FILE * f, cell_t * board, int size) {
  char	*s = (char *) malloc(size+10);
  fgets (s, size+10,f);                 // Ignore first line

  for (int j = 0; j < size; ++j) {      // For to read board file
    fgets (s, size+10,f);               // Get string
    for (int i = 0; i < size; ++i)      // For to set cells
      board[i*size + j] = s[i] == 'x';  // Set cells
  }
}
