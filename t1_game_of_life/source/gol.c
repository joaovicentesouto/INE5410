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
cell_t ** prev, next, tmp;

struct {
  int minor_i, minor_j, major_i, major_j;
} Submatrix;

//! Calcula divisão das threads
/*  Calcula a divisão da matriz de forma mais adequada
 *  procurando o a menor diferença entre as sub-matrizes.
 *  row+col-2 == quantidade de mutex que será necessário.
 *  Mas será que precisa de mutex? Cada thread escreve em
 *  um lugar diferente.
 */
void division_of_work(int max_threads, int& thr_per_row, int& thr_per_col) {
  thr_per_row = max_threads;
  thr_per_col = 1;
  for (int i = 1; i <= max_threads/2; i++)
    for (int j = sqrt(max_threads); j <= max_threads; j++)
      if (i*j == max_threads && abs(i-j) < abs(thr_per_row-thr_per_col)) {
          thr_per_row = i;
          thr_per_col = j;
      }
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

//! Play de uma thread
void thread_play(void *arg) {
  struct Submatrix *sub = (*Submatrix) arg;
  int	i, j, a;

  /* for each cell, apply the rules of Life */
  for (i=sub->minor_i; i<=sub->major_i; i++)
    for (j=sub->minor_j; j<=sub->major_j; j++) {
      a = adjacent_to(board, size, i, j);
      if (a == 2) newboard[i][j] = board[i][j]; // Still the same
      if (a == 3) newboard[i][j] = 1;           // It's Alive!!! FRANKENSTEIN
      if (a < 2) newboard[i][j] = 0;            // Dies
      if (a > 3) newboard[i][j] = 0;            // Dies
    }
}

void play(cell_t ** board, cell_t ** newboard, int size) {

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

  int max_threads = argc > 1? atoi(argv[1]) : 1;

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

  // Parâmetros iniciais para cada threads
  int thr_per_row, thr_per_col, n_row, n_col;
  division_of_work(amount_of_threads, thr_per_row, thr_per_col);
  n_row = (int) n/row;
  n_row = n_row*row == n? n_row : n_row+1;
  n_col = (int) n/col;
  n_col = n_col*col == n? n_col : n_col+1;

  // Cálculo das gerações
  for (i=0; i<steps; i++) {

    pthread_t threads[amount_of_threads];

    // For para criar todas as threads.
    // Cada threads tem sua submatriz.
    int count = 0; // Auxiliar
    for (int k = 0; k < max_threads; ++k) {
      struct Submatrix sub;
      count = k%col == 0 && k!=0? count+1 : count;

      // Linha inicial e final
      sub.minor_i = (count%row)*n_row;
      int major_i = (count%row+1)*n_row-1;
      sub.major_i = major_i < n? major_i : n-1;

      // Coluna inicial e final
      sub.minor_j = (k%col)*n_col;
      int major_j = (k%col+1)*n_col-1;
      sub.major_j = major_j < n? major_j : n-1;

      // Cria threads para processar a submatriz
      pthread_create(&threads[i], NULL, thread_play, (*void) &sub);
    }

    for (int t = 0; t < amount_of_threads; ++t) {
      pthread_join(threads[i], NULL);
    }

    free(threads);

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
