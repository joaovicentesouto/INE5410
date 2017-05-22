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
* The game plays a number of steps (given by the input), printing to the screen
* each time. 'x' printed means on, space means off.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
typedef unsigned char cell_t;

/* Variáveis globais */
cell_t **prev, **next, **tmp;
int max_threads, steps, size, iterator = 0;
pthread_barrier_t barrier;

typedef struct {
  int row_per_thr, col_per_thr, height, width;
} Rules;

typedef struct {
  int minor_i, minor_j, major_i, major_j;
} Matrix;

/* Funcoes secundarias implementadas apos o main() */
cell_t ** allocate_board(int size);
void free_board(cell_t ** board, int size);
int adjacent_to(cell_t ** board, int size, int i, int j);
void print(cell_t ** board, int size);
void read_file(FILE * f, cell_t ** board, int size);

/* Encontra linhas e colunas por thread e a largura e altura da sub-matriz */
void division_of_work(Rules* r) {
  // Encontra linhas e colunas por thread
  r->row_per_thr = max_threads;
  r->col_per_thr = 1;
  for (int i = 1; i <= sqrt(max_threads); i++) {
    for (int j = sqrt(max_threads); j <= max_threads; j++) {
      if (i*j == max_threads && abs(i-j) < abs(r->row_per_thr-r->col_per_thr)) {
          r->row_per_thr = i;
          r->col_per_thr = j;
      }
    }
  }
  // Altura e largura, cuidando se ocorrer divisao nao inteira
  float precision = size / r->row_per_thr;
  r->height = (int) precision;
  if (r->height * r->row_per_thr != size)
    r->height = precision - r->height >= 0.5 ? r->height : r->height+1;

  precision = size / r->col_per_thr;
  r->width = (int) precision;
  if (r->width * r->col_per_thr != size)
    r->width = precision - r->width >= 0.5 ? r->width : r->width+1;
}

/* Funcao executada pelas threads*/
void* play(void* arg) {
  Matrix* sub = (Matrix*) arg;
  int	i, j, a, only_one;

  while (iterator < steps) {

    for (i = sub->minor_i; i <= sub->major_i; i++) {
      for (j = sub->minor_j; j <= sub->major_j; j++) {
        a = adjacent_to(prev, size, i, j);
        if (a == 2) next[i][j] = prev[i][j];  // Still the same
        if (a == 3) next[i][j] = 1;           // It's Alive!!!
        if (a < 2) next[i][j] = 0;            // Die
        if (a > 3) next[i][j] = 0;            // Die
      }
    }

    only_one = pthread_barrier_wait(&barrier); // Espera todas

    if (only_one == PTHREAD_BARRIER_SERIAL_THREAD) {
      iterator++; // Soma a geração calculada.

      #ifdef DEBUG
      printf("\n----------\n#%d", iterator);
      print(next, size);
      #endif

      tmp = next;
      next = prev;
      prev = tmp;
    }

    pthread_barrier_wait(&barrier); // Espera todas
  }

  free(sub);
  pthread_exit(NULL);
}

int main(int argc, char * argv[]) {

  max_threads = argc > 1? atoi(argv[1]) : 1;

  // Inicializacao
  pthread_t threads[max_threads];
  Rules* rules = malloc(sizeof(Rules));
  pthread_barrier_init (&barrier, NULL, max_threads);

  // Cria tabuleiro e carrega células
  FILE *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);
  prev = allocate_board(size);
  read_file(f, prev, size);
  fclose(f);
  next = allocate_board(size);

  #ifdef DEBUG
  printf("Initial:\n");
  print(prev, size);
  #endif

  division_of_work(rules);

  int i, tmp, count = 0;
  // i serve para as formação das colunas e count para as linhas.
  for (i = 0; i < max_threads; i++) {
    Matrix *sub = malloc(sizeof(Matrix));

    // Count é somado quando todas as threads de uma linha forem criadas.
    count = (i % rules->col_per_thr) == 0 && i != 0 ? count+1 : count;

    // Linha inicial e final
    tmp = (count % rules->row_per_thr) * rules->height;
    sub->minor_i = tmp < size ? tmp : size-1;
    tmp = (count % rules->row_per_thr + 1) * rules->height - 1;
    sub->major_i = tmp < size ? tmp : size-1;

    // Coluna inicial e final
    tmp = (i % rules->col_per_thr) * rules->width;
    sub->minor_j = tmp < size ? tmp : size-1;
    tmp = (i % rules->col_per_thr + 1) * rules->width - 1;
    sub->major_j = tmp < size ? tmp : size-1;

    #ifdef DEBUG
    printf("Matriz: %d, i:[%d, %d] - j:[%d, %d]\n",
            i, sub->minor_i, sub->major_i, sub->minor_j, sub->major_j);
    #endif

    pthread_create(&threads[i], NULL, play, (void*) sub);
  }

  for (i = 0; i < max_threads; i++)
    pthread_join(threads[i], NULL);

  #ifdef RESULT
  printf("Final:\n");
  print(prev, size);
  #endif

  free(rules);
  free_board(prev, size);
  free_board(next, size);

  pthread_barrier_destroy(&barrier);
}

/* allocate memory */
cell_t ** allocate_board(int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  for (int i = 0; i < size; i++)
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

/* unallocate memory */
void free_board(cell_t ** board, int size) {
  int i;
  for (i = 0; i < size; i++)
    free(board[i]);
  free(board);
}

/* return the number of on cells adjacent to the i,j cell */
int adjacent_to(cell_t ** board, int size, int i, int j) {
  int	living_cells = 0;

  int minor_k = (i>0) ? i-1 : i;
  int major_k = (i+1 < size) ? i+1 : i;
  int minor_l = (j>0) ? j-1 : j;
  int major_l = (j+1 < size) ? j+1 : j;

  for (int k = minor_k; k <= major_k; k++)
    for (int l = minor_l; l <= major_l; l++)
      living_cells += board[k][l];
  living_cells -= board[i][j]; // Your own decreased cell position

  return living_cells;
}

/* print the life board */
void print(cell_t ** board, int size) {
  int	i, j;
  for (j = 0; j < size; j++) {
    for (i = 0; i < size; i++)
      printf("%c", board[i][j] ? 'x' : ' ');
    printf("\n");
  }
}

/* read a file into the life board */
void read_file(FILE * f, cell_t ** board, int size) {
  int	i, j;
  char	*s = (char *) malloc(size+10);

  fgets(s, size+10, f); // ignore first line

  for (j = 0; j < size; j++) {
    fgets(s, size+10,f);

    for (i = 0; i < size; i++)
      board[i][j] = s[i] == 'x';
  }
}
