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
#include <pthread.h>
typedef unsigned char cell_t;

// Variáveis goblais
unsigned int quant_threads;
cell_t ** prev, next, tmp;
struct Submatrix {
  int minor_i, minor_j, major_i, major_j;
};

//! Calcula divisão das threads
/*  Calcula a divisão da matriz de forma mais adequada
 *  procurando o a menor diferença entre as sub-matrizes.
 *  row+col-2 == quantidade de mutex que será necessário.
 *  Mas será que precisa de mutex? Cada thread escreve em
 *  um lugar diferente.
 */
void division_of_work(int number_threads, int &row, int &col) {
  int i=number_threads, j=1;
  while (i >= j) {
    if (i*j == number_threads) {
      row = i;
      col = j;
    }
    --i;
    ++j;
  }
}

//! Play de uma thread
void thread_play(void *stru) {
  struct Thread *th = (*Thread) stru;

  /* for each cell, apply the rules of Life */
  for (i=0; i<size; i++)
    for (j=0; j<size; j++) {
      a = adjacent_to(board, size, i, j);
      if (a == 2) newboard[i][j] = board[i][j]; // Still the same
      if (a == 3) newboard[i][j] = 1;           // It's Alive!!! FRANKENSTEIN
      if (a < 2) newboard[i][j] = 0;            // Dies
      if (a > 3) newboard[i][j] = 0;            // Dies
    }
}

void play(cell_t ** board, cell_t ** newboard, int size) {
  int	i, j, a, row, col;

  division_of_work(quant_threads, row, col);
  pthread_t threads[quant_threads];
  int init_line = 0;
  int init_col = 0;
  int end_line = size/row;
  int end_col = size/col;
  for (int i = 0; i < quant_threads; ++i) {
    struct Thread th;
    pthread_create(&threads[i], NULL, thread_play, (* void) th);
  }

  /* for each cell, apply the rules of Life */
  for (i=0; i<size; i++)
    for (j=0; j<size; j++) {
      a = adjacent_to(board, size, i, j);
      if (a == 2) newboard[i][j] = board[i][j]; // Still the same
      if (a == 3) newboard[i][j] = 1;           // It's Alive!!! FRANKENSTEIN
      if (a < 2) newboard[i][j] = 0;            // Dies
      if (a > 3) newboard[i][j] = 0;            // Dies
    }
}

cell_t ** allocate_board(int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  int	i;
  for (i=0; i<size; i++)
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board(cell_t ** board, int size) {
  int i;
  for (i=0; i<size; i++)
    free(board[i]);
  free(board);
}

/* return the number of on cells adjacent to the i,j cell */
int adjacent_to(cell_t ** board, int size, int i, int j) {
  int	k, l, count=0;

  // Limits
  int sk = (i>0) ? i-1 : i;
  int ek = (i+1 < size) ? i+1 : i;
  int sl = (j>0) ? j-1 : j;
  int el = (j+1 < size) ? j+1 : j;

  for (k=sk; k<=ek; k++)
    for (l=sl; l<=el; l++)
      count+=board[k][l];
  count-=board[i][j]; // Your own decreased cell position

  return count;
}

/* print the life board */
void print(cell_t ** board, int size) {
  int	i, j;
  /* for each row */
  for (j=0; j<size; j++) {
    /* print each column position... */
    for (i=0; i<size; i++)
    printf("%c", board[i][j] ? 'x' : ' ');
    /* followed by a carriage return */
    printf("\n");
  }
}

/* read a file into the life board */
void read_file(FILE * f, cell_t ** board, int size) {
  int	i, j;
  char	*s = (char *) malloc(size+10);

  /* read the first new line (it will be ignored) */
  fgets(s, size+10,f);

  /* read the life board */
  for (j=0; j<size; j++) {
    /* get a string */
    fgets(s, size+10,f);
    /* copy the string to the life board */
    for (i=0; i<size; i++)
      board[i][j] = s[i] == 'x';
  }
}

int main(int argc, char * argv[]) {

  // Cria tabuleiro e carrega células
  int size, steps;
  FILE    *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);
  prev = allocate_board(size); // Uma thread aloca o prev
  read_file(f, prev,size);
  fclose(f);
  next = allocate_board(size); // Uma thread aloca o next
  int i;

  #ifdef DEBUG
  printf("Initial:\n");
  print(prev,size);
  #endif

  // Cálculo das gerações
  for (i=0; i<steps; i++) {
    play (prev,next,size);

    #ifdef DEBUG
    printf("%d\n----------\n", i + 1);
    print(next,size);
    #endif

    tmp = next;
    next = prev;
    prev = tmp;
  }

  #ifdef RESULT
  printf("Final:\n");
  print (prev,size);
  #endif

  free_board(prev,size); // Desaloca memória
  free_board(next,size); // Desaloca memória
}
