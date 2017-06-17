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
typedef unsigned char cell_t;

/* return the number of on cells adjacent to the i,j cell */
int adjacent_to (cell_t * board, int size, int i, int j) {
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

void play (cell_t * board, cell_t * newboard, int size) {
  int	a, position;
  /* for each cell, apply the rules of Life */
  for (int i = 0; i < size; ++i)
  for (int j = 0; j < size; ++j) {
    position = i*size + j;
    a = adjacent_to (board, size, i, j);
    if (a == 2) newboard[position] = board[position];
    if (a == 3) newboard[position] = 1;
    if (a < 2) newboard[position] = 0;
    if (a > 3) newboard[position] = 0;
  }
}

/* print the life board */
void print (cell_t * board, int size) {
  for (int j = 0; j < size; ++j) {  // For each row
    for (int i = 0; i < size; ++i)  // Print each column position...
      printf ("%c", board[i*size + j] ? 'x' : ' ');
    printf ("\n");  // End line
  }
}

/* read a file into the life board */
void read_file (FILE * f, cell_t * board, int size) {
  char	*s = (char *) malloc(size+10);
  fgets (s, size+10,f);                 // Ignore first line

  for (int j = 0; j < size; ++j) {      // For to read board file
    fgets (s, size+10,f);               // Get string
    for (int i = 0; i < size; ++i)      // For to set cells
      board[i*size + j] = s[i] == 'x';  // Set cells
  }
}

int main () {

  int size, steps;
  FILE *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);

  cell_t * prev = (cell_t *) malloc(sizeof(cell_t) * size * size);
  read_file (f, prev,size);
  fclose(f);

  cell_t * next = (cell_t *) malloc(sizeof(cell_t) * size * size);
  cell_t * tmp;

  #ifdef DEBUG
  printf("Initial:\n");
  print(prev,size);
  #endif

  for (int i = 0; i < steps; ++i) {
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

  free(prev);
  free(next);
}
