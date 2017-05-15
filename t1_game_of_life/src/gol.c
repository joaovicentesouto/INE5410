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
* The game plays a number of STEPS (given by the input), printing to the screen each time.  'x' printed
* means on, space means off.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
typedef unsigned char cell_t;

cell_t **prev, **next, **tmp;
pthread_mutex_t critical_section, barrier;
sem_t game_calculation;
int MAX_THREADS, STEPS, SIZE, ITERATOR = 0, LAST = 0;

typedef struct {
  int row_per_thr, col_per_thr, height, width;
} Rules;

typedef struct {
  int minor_i, minor_j, major_i, major_j;
} Matrix;

void division_of_work(Rules* r)
{
  r->row_per_thr = MAX_THREADS;
  r->col_per_thr = 1;
  for (int i = 1; i <= sqrt(MAX_THREADS); i++)
    for (int j = sqrt(MAX_THREADS); j <= MAX_THREADS; j++)
      if (i*j == MAX_THREADS && abs(i-j) < abs(r->row_per_thr-r->col_per_thr)) {
          r->row_per_thr = i;
          r->col_per_thr = j;
      }

  float precision = SIZE / r->row_per_thr;
  r->height = (int) precision;
  if (r->height * r->row_per_thr != SIZE)
    r->height = precision - r->height >= 0.5 ? r->height : r->height+1;

  precision = SIZE / r->col_per_thr;
  r->width = (int) precision;
  if (r->width * r->col_per_thr != SIZE)
    r->width = precision - r->width >= 0.5 ? r->width : r->width+1;
}

cell_t ** allocate_board(int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  int	i;
  for (i = 0; i < size; i++)
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board(cell_t ** board, int size) {
  int i;
  for (i = 0; i < size; i++)
    free(board[i]);
  free(board);
}

/* return the number of on cells adjacent to the i,j cell */
int adjacent_to(cell_t ** board, int size, int i, int j) {
  int	k, l, living_cells = 0;

  // Limits
  int minor_k = (i>0) ? i-1 : i;
  int major_k = (i+1 < size) ? i+1 : i;
  int minor_l = (j>0) ? j-1 : j;
  int major_l = (j+1 < size) ? j+1 : j;

  for (k = minor_k; k <= major_k; k++)
    for (l = minor_l; l <= major_l; l++)
      living_cells += board[k][l];
  living_cells -= board[i][j]; // Your own decreased cell position

  return living_cells;
}

void* play(void* arg) {
  Matrix* sub = (Matrix*) arg;
  int	i, j, a;

  pthread_mutex_lock(&critical_section);
  while (ITERATOR < STEPS) {
    pthread_mutex_unlock(&critical_section);

    for (i = sub->minor_i; i <= sub->major_i; i++) {
      for (j = sub->minor_j; j <= sub->major_j; j++) {
        a = adjacent_to(prev, SIZE, i, j);
        if (a == 2) next[i][j] = prev[i][j]; // Still the same
        if (a == 3) next[i][j] = 1;           // It's Alive!!!
        if (a < 2) next[i][j] = 0;            // Die
        if (a > 3) next[i][j] = 0;            // Die
      }
    }

    pthread_mutex_lock(&critical_section);
    LAST++;
    if (LAST == MAX_THREADS || MAX_THREADS == 1) {
      LAST = 0;
      ITERATOR++;

      #ifdef DEBUG
      printf("%d\n----------\n", ITERATOR);
      //print(next, SIZE);
      #endif

      tmp = next;
      next = prev;
      prev = tmp;

      for (i = 0; i < MAX_THREADS; i++)
        sem_post(&game_calculation);
    }
    pthread_mutex_unlock(&critical_section);

    pthread_mutex_lock(&critical_section);
    sem_wait(&game_calculation);
  }
  pthread_mutex_unlock(&critical_section);

  pthread_exit(NULL);
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

  pthread_mutex_init(&critical_section, NULL);
  pthread_mutex_init(&barrier, NULL);
  sem_init(&game_calculation, 0, 0); // inicia fechado

  MAX_THREADS = argc > 1? atoi(argv[1]) : 1;
  printf("N# threads: %d\n", MAX_THREADS);

  // Cria tabuleiro e carrega células
  FILE    *f;
  f = stdin;
  fscanf(f,"%d %d", &SIZE, &STEPS);
  prev = allocate_board(SIZE); // Uma thread aloca o prev
  read_file(f, prev, SIZE);
  fclose(f);
  next = allocate_board(SIZE); // Uma thread aloca o next

  #ifdef DEBUG
  printf("Initial:\n");
  print(prev, SIZE);
  #endif

  Rules* rules= malloc(sizeof(Rules));
  division_of_work(rules);

  pthread_t threads[MAX_THREADS];

  // Cálculo das gerações
  int i, count = 0;
  for (i = 0; i < MAX_THREADS; i++) {
    int tmp;
    Matrix *sub = malloc(sizeof(Matrix));

    count = (i % rules->col_per_thr) == 0 && i != 0 ? count+1 : count;
    // Linha inicial e final
    tmp = (count % rules->row_per_thr) * rules->height;
    sub->minor_i = tmp < SIZE ? tmp : SIZE-1;
    tmp = (count % rules->row_per_thr + 1) * rules->height - 1;
    sub->major_i = tmp < SIZE ? tmp : SIZE-1;

    // Coluna inicial e final
    tmp = (i % rules->col_per_thr) * rules->width;
    sub->minor_j = tmp < SIZE ? tmp : SIZE-1;
    tmp = (i % rules->col_per_thr + 1) * rules->width - 1;
    sub->major_j = tmp < SIZE ? tmp : SIZE-1;

    pthread_create(&threads[i], NULL, play, (void*) sub);
  }

  for (i = 0; i < MAX_THREADS; i++)
    pthread_join(threads[i], NULL);

  #ifdef RESULT
  printf("Final:\n");
  print(prev, SIZE);
  #endif

  free_board(prev, SIZE); // Desaloca memória
  free_board(next, SIZE); // Desaloca memória

  pthread_mutex_destroy(&critical_section);
  pthread_mutex_destroy(&barrier);
  sem_destroy(&game_calculation);
}
