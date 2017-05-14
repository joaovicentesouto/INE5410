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
#include <math.h>
typedef unsigned char cell_t;

// Variáveis goblais
cell_t **prev, **next, **tmp;
// Usar typedef na Submatrix?
typedef struct {
  int minor_i, minor_j, major_i, major_j, total_size;
} Submatrix;

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

  int max_threads = argc > 1? atoi(argv[1]) : 1;
  printf("max_t:%d\n", max_threads);

  // Cria tabuleiro e carrega células
  int size, steps;
  FILE    *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);
  prev = allocate_board(size);
  read_file(f, prev, size);
  fclose(f);
  next = allocate_board(size);

  #ifdef RESULT
  printf("Initial:\n");
  print(prev,size);
  #endif

  #ifdef DEBUG
  printf("Initial:\n");
  print(prev,size);
  #endif

  // Parâmetros iniciais para cada threads
  int row_per_thr, col_per_thr, height, width;
  division_of_work(max_threads, size, &row_per_thr, &col_per_thr, &height, &width);

  #ifdef DEBUG
  printf("Divisão das linhas: %d\n", row_per_thr);
  printf("Divisão das colunas: %d\n", col_per_thr);
  printf("Tamanho relativo de cada submatriz: %d x %d\n\n ", height, width);
  #endif

  // Cálculo das gerações
  for (int i=0; i<steps; i++) {

    //printf("Geração:%d\n", i);
    pthread_t threads[max_threads];

    // For para criar todas as threads.
    // Cada threads tem sua submatriz.
    int count, k = 0; // Auxiliar
    for (k = 0; k < max_threads; k++) {

      int tmp;
      Submatrix *sub = malloc(sizeof(Submatrix));

      count = k%col_per_thr==0 && k!=0 ? count+1 : count;
      // Linha inicial e final
      tmp = (count%row_per_thr) * height;
      sub->minor_i = tmp < size ? tmp : size - 1;
      tmp = (count%row_per_thr + 1) * height - 1;
      sub->major_i = tmp < size ? tmp : size - 1;

      // Coluna inicial e final
      tmp = (k%col_per_thr) * width;
      sub->minor_j = tmp < size ? tmp : size-1;
      tmp = (k%col_per_thr + 1) * width - 1;
      sub->major_j = tmp < size ? tmp : size-1;

      sub->total_size = size;

      #ifdef DEBUG
      printf("Thread #%d: i:[%d, %d] | j:[%d, %d]\n", k,
             sub->minor_i, sub->major_i, sub->minor_j, sub->major_j);
      #endif

      // Cria threads para processar a submatriz
      pthread_create(&threads[k], NULL, play, (void *) sub);
    }

    for (k = 0; k < max_threads; k++) {
      pthread_join(threads[k], NULL);
    }

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

//! Calcula divisão das threads
/*  Calcula a divisão da matriz de forma mais adequada
 *  procurando o a menor diferença entre as sub-matrizes.
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

  int	count=0;
  // Limits
  int sk = (i>0) ? i-1 : i;
  int ek = (i+1 < size) ? i+1 : i;
  int sl = (j>0) ? j-1 : j;
  int el = (j+1 < size) ? j+1 : j;

  for (int k=sk; k<=ek; k++)
    for (int l=sl; l<=el; l++)
      count+=board[k][l];
  count-=board[i][j]; // Your own decreased cell position

  return count;
}

//! Play de uma thread
void* play(void *arg) {
  Submatrix *sub = (Submatrix*) arg;
  int	a;
  /* for each cell, apply the rules of Life */
  for (int i=sub->minor_i; i<=sub->major_i; i++) {
    for (int j=sub->minor_j; j<=sub->major_j; j++) {
      a = adjacent_to(prev, sub->total_size, i, j);
      if (a == 2) next[i][j] = prev[i][j]; // Still the same
      if (a == 3) next[i][j] = 1;           // It's Alive!!!
      if (a < 2) next[i][j] = 0;            // Dies
      if (a > 3) next[i][j] = 0;            // Dies
    }
  }
  free(sub);
  pthread_exit(NULL);
}

cell_t ** allocate_board(int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  int	i;
  for (i=0; i<size; i++)
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board(cell_t ** board, int size) {
  for (int i=0; i<size; i++)
    free(board[i]);
  free(board);
}

/* print the life board */
void print(cell_t ** board, int size) {
  /* for each row */
  for (int j=0; j<size; j++) {
    /* print each column position... */
    for (int i=0; i<size; i++)
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
  free(s);
}
