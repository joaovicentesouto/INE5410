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

/* Funcoes executadas por ambos */
cell_t ** allocate_board (int size);
void free_board (cell_t ** board, int size);

/* Funcoes executadas apenas pelo mestre */
void print (cell_t ** board, int size);
void read_file (FILE * f, cell_t ** board, int size);

/* Funcoes executadas apenas pelos escravos */
int adjacent_to (cell_t ** board, int size, int i, int j);
void play (cell_t ** board, cell_t ** newboard, int size);

int main (int argc, char *argv[]) {
  int processes, rank;
  MPI_Init(argc, argv);
  MPI_Comm_size(MPI_COMM_WORLD, &processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) { // master
    int size, steps;
    FILE *f;
    f = stdin;
    fscanf(f,"%d %d", &size, &steps);
    cell_t ** prev = allocate_board (size);
    read_file (f, prev,size);
    fclose(f);
    cell_t ** next = allocate_board (size);
    cell_t ** tmp;
    int i;
    #ifdef DEBUG
    printf("Initial:\n");
    print(prev,size);
    #endif

    // Quantidade de linhas para cada um
    float precision = size/processes;
    int integer_part, lines_amount;
    lines_amount = integer_part = (int) precision;
    lines_amount += precision - integer_part <= 0.5 ? 0 : 1;

    // Avisando pra construirem as matrizes com os respectivos tamanhos
    MPI_Send(&lines_amount, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Tem que verificar se o ultimo esta com o tamanho correto
    // Se size - line..... for == 0 entao ocorreu divisao inteira
    int last_process = lines_amount + (size - lines_amount*processes);
    //if (last_process != lines_amount) // Ultimo processo com o tamanho errado
    MPI_Send(&last_process, 1, MPI_INT, processes, MPI_COMM_WORLD);

    // Matrizes auxiliares
    cell_t first_and_last[lines_amount+1][size];
    cell_t others[lines_amount+2][size];

    /* Talvez nem precise pegar os ponteiros, pensando que a matriz
       esta disposta em linhas, posso dizer qual lugar comecar a 
       pegar entao mandar size * linhas e tal???
       malloc eh dinamico, mas coloca tudo sequencial?
    */

    // Primeiro processo
    for (int i = 0; i < lines_amount+1; ++i)
      first_and_last[i] = prev[i];
    MPI_Send(&first_and_last, (lines_amount+1)*size, MPI_UNSIGNED_CHAR, 1, MPI_COMM_WORLD);

    /***** Ver pra usar SendI pra nao ficar esperando enviar  *****/

    // processos do meio
    int aux = 0;
    for (int i = 1; i < processes-2; ++i) {
      aux = (i*lines_amount-1); // em funcao do processo (p-1 * linhas - 1)
      for (int j = 0; j < lines_amount+2; ++j)
        others[j] = prev[aux + j];
      MPI_Send(&others, (lines_amount+2)*size, MPI_UNSIGNED_CHAR, i+1, MPI_COMM_WORLD);
    }

    // ultimo processo
    aux = (processes-1)*lines_amount-1;
    for (int i = 0; i < lines_amount+1; ++i)
      first_and_last[i] = prev[aux + i];
    MPI_Send(&first_and_last, (lines_amount+1)*size, MPI_UNSIGNED_CHAR, 1, MPI_COMM_WORLD);

    for (int i = 0; i < processes-1; ++i) {
      MPI_Recv(); // espera todos terminarem
    }

  } else { // slave

  }

  for (i=0; i<steps; i++) {
    play (prev,next,size);
    #ifdef DEBUG
    printf("%d ----------\n", i + 1);
    print (next,size);
    #endif
    tmp = next;
    next = prev;
    prev = tmp;
  }

  #ifdef RESULT
  printf("Final:\n");
  print (prev,size);
  #endif

  free_board(prev,size);
  free_board(next,size);

  MPI_Finalize();
  return 0;
}

cell_t ** allocate_board (int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*) * size);
  for (int i = 0; i < size; ++i)
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board (cell_t ** board, int size) {
  for (int i = 0; i < size; ++i)
    free(board[i]);
  free(board);
}


/* return the number of on cells adjacent to the i,j cell */
int adjacent_to (cell_t ** board, int size, int i, int j) {
  int k, l, count=0;

  int sk = (i>0) ? i-1 : i;
  int ek = (i+1 < size) ? i+1 : i;
  int sl = (j>0) ? j-1 : j;
  int el = (j+1 < size) ? j+1 : j;

  for (k=sk; k<=ek; k++)
    for (l=sl; l<=el; l++)
      count+=board[k][l];
  count-=board[i][j];

  return count;
}

void play (cell_t ** board, cell_t ** newboard, int size) {
  /* for each cell, apply the rules of Life */
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      a = adjacent_to (board, size, i, j);
      if (a == 2) newboard[i][j] = board[i][j];
      if (a == 3) newboard[i][j] = 1;
      if (a < 2) newboard[i][j] = 0;
      if (a > 3) newboard[i][j] = 0;
    }
  }
}

/* print the life board */
void print (cell_t ** board, int size) {
  /* for each row */
  for (int j = 0; j < size; ++j) {
    /* print each column position... */
    for (int i = 0; i < size; ++i)
      printf ("%c", board[i][j] ? 'x' : ' ');
    /* followed by a carriage return */
    printf ("\n");
  }
}

/* read a file into the life board */
void read_file (FILE * f, cell_t ** board, int size) {
  char  *s = (char *) malloc(size+10);
  fgets (s, size+10,f); // read the first new line (it will be ignored)

  /* read the life board */
  for (int j=0; j<size; j++) {
    fgets (s, size+10,f);           // get a string
    for (int i = 0; i < size; ++i)  // copy the string to the life board
      board[i][j] = s[i] == 'x';
  }
}