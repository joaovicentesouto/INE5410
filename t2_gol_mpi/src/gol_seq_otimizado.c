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
int adjacent_to(cell_t * board, int lines, int size, int i, int j) {
  int count = 0;
  if (i > 0) {
      count += board[(i-1)*size + j];
      if (j > 1)
          count += board[(i-1)*size + j-1];
      if (j+1 < size)
          count += board[(i-1)*size + j+1];
  }

  if (j > 1) {
      count += board[i*size + j-1];
      if (i+1 < lines)
          count += board[(i+1)*size + j-1];
  }

  if (i+1 < lines) {
      count += board[(i+1)*size + j];
      if (j+1 < size)
          count += board[(i+1)*size + j+1];
  }

  if (j+1 < size)
      count += board[i*size + j+1];

  return count;
}

/* Pode acessar qualquer linha, menos a coluna da esquerda */
int left_line_safe_adjacent_to(cell_t * board, int size, int i, int j) {
  int count = 0;
  count += board[(i-1)*size + j];
  count += board[(i-1)*size + j+1];

  count += board[i*size + j+1];
  
  count += board[(i+1)*size + j];
  count += board[(i+1)*size + j+1];
  return count;
}

/* Pode acessar qualquer linha, menos a coluna da direita */
int right_line_safe_adjacent_to(cell_t * board, int size, int i, int j) {
  int count = 0;
  count += board[(i-1)*size + j-1];
  count += board[(i-1)*size + j];

  count += board[i*size + j-1];
  
  count += board[(i+1)*size + j-1];
  count += board[(i+1)*size + j];
  return count;
}

/* Pode acessar qualquer coluna, menos a linha de cima */
int top_column_safe_adjacent_to(cell_t * board, int size, int i, int j) {
  int count = 0;
  count += board[i*size + j-1];
  count += board[i*size + j+1];

  count += board[(i+1)*size + j-1];
  count += board[(i+1)*size + j];
  count += board[(i+1)*size + j+1];
  return count;
}

/* Pode acessar qualquer coluna, menos a linha de baixo */
int bottom_column_safe_adjacent_to(cell_t * board, int size, int i, int j) {
  int count = 0;
  count += board[(i-1)*size + j-1];
  count += board[(i-1)*size + j];
  count += board[(i-1)*size + j+1];

  count += board[i*size + j-1];
  count += board[i*size + j+1];
  return count;
}

/* Eh seguro acessar qualquer posicao ao redor da celula */
int totally_safe_adjacent_to(cell_t * board, int size, int i, int j) {
  int count = 0;
  count += board[(i-1)*size + j-1];
  count += board[(i-1)*size + j];
  count += board[(i-1)*size + j+1];

  count += board[i*size + j-1];
  count += board[i*size + j+1];
  
  count += board[(i+1)*size + j-1];
  count += board[(i+1)*size + j];
  count += board[(i+1)*size + j+1];
  return count;
}

void play_optimized(cell_t * board, cell_t * newboard, int size, int lines, int beg, int end) {
  int a, position, end_inicial = end;

  end = lines == end_inicial+1? end : end+1;

  if (beg == 0) {
    a = adjacent_to(board, lines, size, 0, 0);
    if (a == 2)
      newboard[0] = board[0];
    else if (a == 3)
      newboard[0] = 1;
    else
      newboard[0] = 0;

    for (int j = 1; j < size-1; ++j) {
      a = top_column_safe_adjacent_to(board, size, 0, j);
      if (a == 2)
        newboard[j] = board[j];
      else if (a == 3)
        newboard[j] = 1;
      else
        newboard[j] = 0;
    }

    position = size-1;
    a = adjacent_to(board, lines, size, 0, position);
    if (a == 2)
      newboard[position] = board[position];
    else if (a == 3)
      newboard[position] = 1;
    else
      newboard[position] = 0;

    ++beg;
  }

  /* ============================================================ */
  /* Linhas intermediarias 1 ate size-2, linhas seguras           */
  for (int i = beg; i < end; ++i) {
    position = i*size;
    a = left_line_safe_adjacent_to(board, size, i, 0);
    if (a == 2)
      newboard[position] = board[position];
    else if (a == 3)
      newboard[position] = 1;
    else
      newboard[position] = 0;

    for (int j = 1; j < size-1; ++j) {
      position = i*size + j;
      a = totally_safe_adjacent_to(board, size, i, j);
      if (a == 2)
        newboard[position] = board[position];
      else if (a == 3)
        newboard[position] = 1;
      else
        newboard[position] = 0;
    }

    position = i*size + size-1;
    a = right_line_safe_adjacent_to(board, size, i, size-1);
    if (a == 2)
      newboard[position] = board[position];
    else if (a == 3)
      newboard[position] = 1;
    else
      newboard[position] = 0;
  }


  /* =================================================================== */
  /* Caso tenha que calcular a ultima linha (rank == processes-1 apenas) */
  if (lines == end_inicial+1) {
    position = end_inicial*size;
    a = adjacent_to(board, lines, size, end_inicial, 0);
    if (a == 2)
      newboard[position] = board[position];
    else if (a == 3)
      newboard[position] = 1;
    else
      newboard[position] = 0;

    for (int j = 1; j < size-1; ++j) {
      position = end_inicial*size + j;
      a = bottom_column_safe_adjacent_to(board, size, end_inicial, j);
      if (a == 2)
        newboard[position] = board[position];
      else if (a == 3)
        newboard[position] = 1;
      else
        newboard[position] = 0;
    }

    position = end_inicial*size + size-1;
    a = adjacent_to(board, lines, size, end_inicial, size-1);
    if (a == 2)
      newboard[position] = board[position];
    else if (a == 3)
      newboard[position] = 1;
    else
      newboard[position] = 0;
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
    play_optimized(prev, next, size, size, 0, size-1);

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
