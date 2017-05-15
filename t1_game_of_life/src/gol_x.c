/*
* The Game of Life
*
* a cell is born, if it has exactly three neighbours
* a cell dies of loneliness, if it has less than two neighbours
* a cell dies of overcrowding, if it has more than three neighbours
* a cell survives to the next GENERATION, if it does not die of loneliness
* or overcrowding
*
* In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
* The game plays a number of steps (given by the input), printing to the screen each time.  'x' printed
* means on, space means off.
*
*/

/*  Alguns aspectos e problemas ainda não vistos ou solucionados:
 *  - Com a matriz muito pequena e um numero grande threads (11x11 com 7 threads)
 *    ocorre de mais de uma thread calcular a mesma linha.
 *  - É possível otimizar em outros aspectos? Na hora de alocar e desalocar?
 *    Na hora de ler?
 *  - IMPORTANTE: Estudar a idéia do Ricardo que é criar apenas uma vez
 *    as threads e usar semáforos pra fazer com que elas trabalhem quando
 *    a thread main informar.
 *  - IMPORTANTE: melhorar a legibilidade do código.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
typedef unsigned char cell_t;

// Variáveis goblais e estruturas
cell_t **prev, **next, **tmp;
int LAST, STEPS, GENERATION = 0;
pthread_mutex_t critical_section;
sem_t game_calculation;

typedef struct {
  int minor_i, minor_j, major_i, major_j, total_size, max_threads;
} Rules;

// Declaração das funções. Implementação após main()
void division_of_work(int max_threads,
                      int size,
                      int *row_per_thr,
                      int *col_per_thr,
                      int *height,
                      int *width);
int adjacent_to(cell_t ** board, int size, int i, int j);
void* play(void *arg);
cell_t ** allocate_board(int size);
void free_board(cell_t ** board, int size);
void print(cell_t ** board, int size);
void read_file(FILE * f, cell_t ** board, int size);

int main(int argc, char * argv[]) {

  pthread_mutex_init(&critical_section, NULL);
  sem_init(&game_calculation, 0, 0); // inicia fechado

  int max_threads = argc > 1? atoi(argv[1]) : 1;
  printf("max_t:%d\n", max_threads);

  // Cria tabuleiro e carrega células
  int size;
  FILE *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &STEPS);
  prev = allocate_board(size);
  read_file(f, prev, size);
  fclose(f);
  next = allocate_board(size);

  #ifdef RESULT
  printf("Initial:\n");
  print(prev, size);
  #endif

  #ifdef DEBUG
  printf("Initial:\n");
  print(prev, size);
  #endif

  // Parâmetros iniciais para cada threads
  int row_per_thr, col_per_thr, height, width;
  division_of_work(max_threads, size, &row_per_thr, &col_per_thr, &height, &width);

  #ifdef DEBUG
  printf("Divisão das linhas: %d\n", row_per_thr);
  printf("Divisão das colunas: %d\n", col_per_thr);
  printf("Tamanho relativo de cada submatriz: %d x %d\n\n ", height, width);
  #endif

  pthread_t threads[max_threads];

  int count; // Auxiliar
  for (int k = 0; k < max_threads; k++) {

    int tmp;
    Rules *rules = malloc(sizeof(Rules));

    count = (k % col_per_thr) == 0 && k != 0 ? count+1 : count;
    // Linha inicial e final
    tmp = (count % row_per_thr) * height;
    rules->minor_i = tmp < size ? tmp : size-1;
    tmp = (count % row_per_thr + 1) * height - 1;
    rules->major_i = tmp < size ? tmp : size-1;

    // Coluna inicial e final
    tmp = (k % col_per_thr) * width;
    rules->minor_j = tmp < size ? tmp : size-1;
    tmp = (k % col_per_thr + 1) * width - 1;
    rules->major_j = tmp < size ? tmp : size-1;

    rules->total_size = size;
    rules->max_threads = max_threads;

    #ifdef DEBUG
    printf("Thread #%d: i:[%d, %d] | j:[%d, %d]\n", k,
           rules->minor_i, rules->major_i, rules->minor_j, rules->major_j);
    #endif

    // Cria threads para processar a submatriz
    pthread_create(&threads[k], NULL, play, (void *) rules);
  }

  for (int k = 0; k < max_threads; k++)
    pthread_join(threads[k], NULL);

  #ifdef RESULT
  printf("Final:\n");
  print (prev,size);
  #endif

  free_board(prev,size); // Desaloca memória
  free_board(next,size); // Desaloca memória

  pthread_mutex_destroy(&critical_section);
  sem_destroy(&game_calculation);
}

//! Calcula divisão das threads
/*  Calcula a divisão da matriz de forma mais adequada
 *  procurando o a menor diferença entre as rules-matrizes.
 *  row+col-2 == quantidade de mutex que será necessário.
 *  Mas será que precisa de mutex? Cada thread escreve em
 *  um lugar diferente.
 */
void division_of_work(int max_threads, int size,
                      int *row_per_thr, int *col_per_thr,
                      int *height, int *width)
{
  *row_per_thr = max_threads;
  *col_per_thr = 1;
  for (int i = 1; i <= sqrt(max_threads); i++) {
    for (int j = sqrt(max_threads); j <= max_threads; j++) {
      if (i*j == max_threads && abs(i-j) < abs(*row_per_thr-*col_per_thr)) {
          *row_per_thr = i;
          *col_per_thr = j;
      }
    }
  }

  // Altura
  float precision = size / *row_per_thr;
  *height = (int) precision;
  if (*height * *row_per_thr != size)
    *height = precision - *height >= 0.5 ? *height : *height+1;
  // Largura
  precision = size / *col_per_thr;
  *width = (int) precision;
  if (*width * *col_per_thr != size)
    *width = precision - *width >= 0.5 ? *width : *width+1;
}

/* return the number of on cells adjacent to the i,j cell */
int adjacent_to(cell_t ** board, int size, int i, int j) {
  int	living_cells=0;
  // Limits
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

void* play(void *arg) {
  int	a;
  Rules *rules = (Rules*) arg;

  while (GENERATION < STEPS) {

    for (int i=rules->minor_i; i<=rules->major_i; i++) {
      for (int j=rules->minor_j; j<=rules->major_j; j++) {
        a = adjacent_to(prev, rules->total_size, i, j);
        if (a == 2) next[i][j] = prev[i][j]; // Still the same
        if (a == 3) next[i][j] = 1;           // It's Alive!!!
        if (a < 2) next[i][j] = 0;            // Die
        if (a > 3) next[i][j] = 0;            // Die
      }
    }

    pthread_mutex_lock(&critical_section);
    LAST++;
    if (LAST == rules->max_threads-1) {

      printf("STEPS %d\n", GENERATION);

      #ifdef DEBUG
      printf("%d\n----------\n", GENERATION + 1);
      print(next, rules->total_size);
      #endif

      tmp = next;
      next = prev;
      prev = tmp;

      LAST = 0;
      GENERATION++;

      for (int j = 0; j < rules->max_threads; j++)
        sem_post(&game_calculation);
    }
    pthread_mutex_unlock(&critical_section);

    sem_wait(&game_calculation);
  }


  free(rules);
  pthread_exit(NULL);
}

cell_t ** allocate_board(int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  for (int i = 0; i < size; i++)
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board(cell_t ** board, int size) {
  for (int i = 0; i < size; i++)
    free(board[i]);
  free(board);
}

/* print the life board */
void print(cell_t ** board, int size) {
  for (int j = 0; j < size; j++) {
    for (int i = 0; i < size; i++)
      printf("%c", board[i][j] ? 'x' : ' ');
    printf("\n");
  }
}

/* read a file into the life board */
void read_file(FILE * f, cell_t ** board, int size) {
  char	*s = (char *) malloc(size+10);
  fgets(s, size+10,f); // Ignore first line

  for (int j = 0; j < size; j++) {
    fgets(s, size+10,f); // Gets new line
    for (int i = 0; i < size; i++)
      board[i][j] = s[i] == 'x'; // Completing the line board
  }
  free(s);
}
