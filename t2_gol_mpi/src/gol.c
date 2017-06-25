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
void play_optimized(cell_t * board, cell_t * newboard, int size, int lines, int beg, int end);
/** Conjunto de adjacent_to personalizados para minimizar os testes **/
int adjacent_to(cell_t * board, int lines, int size, int i, int j);
int left_line_safe_adjacent_to(cell_t * board, int size, int i, int j);
int right_line_safe_adjacent_to(cell_t * board, int size, int i, int j);
int top_column_safe_adjacent_to(cell_t * board, int size, int i, int j);
int bottom_column_safe_adjacent_to(cell_t * board, int size, int i, int j);
int totally_safe_adjacent_to(cell_t * board, int size, int i, int j);

int main (int argc, char *argv[]) {
  int processes, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (processes == 1) {
    printf("Eh necessario mais que um processo para o modelo mestre-escravo funcionar.\n");
    MPI_Finalize();
    return 1;
  }

  if (rank == 0) {

    /*===============================================================*/
    /* 1: Leitura do tamanho e geracoes e broadcast para os escravos */
    int size, steps, limit_proc, i;
    FILE *f;
    f = stdin;
    fscanf(f, "%d %d", &size, &steps);

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&steps, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /*===========================================================*/
    /* Se o numero de processos for maior que a quant. de linhas */
    /* entao o ultimo processo vai ser o de rank size e os maio- */
    /* res que ele vao fizer sem ter o que fazer.                */
    limit_proc = processes-1 <= size? processes-1 : size;

    /*===============================================================*/
    /* Define quant. de linhas, balanceando para que fique o mais    */
    /* distribuido possivel. Criterio: parte quebrada da divisão     */
    /* <= 0.5 -> parte inteira | se nao -> parte inteira+1           */
<<<<<<< HEAD
=======
    /* limit_proc eh o numero de maximo de processos (um por linhas) */
    int i, limit_proc;
    limit_proc = processes-1 <= size? processes-1 : size;

>>>>>>> 0b9588612eba71db4d9cfa03f00a157e13cc12a0
    float precision = ((float)size)/((float)(limit_proc));
    int lines = (int) precision;
    lines += precision - lines <= 0.5 ? 0 : 1;

    MPI_Bcast(&lines, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /*===============================================================*/
    /* Ultimo processo pode ter mais ou menos linhas.                */
    int last_lines;
    if (processes > 2) {
      last_lines = lines + (size - lines*limit_proc);
      MPI_Send(&last_lines, 1, MPI_INT, limit_proc, 0, MPI_COMM_WORLD);
    }

    /*===============================================================*/
    /* Alocando a matriz e lendo do arquivo                          */
    cell_t * prev = (cell_t *) malloc(sizeof(cell_t) * (size+1) * size);
    read_file(f, prev, size);
    fclose(f);

    #ifdef RESULT
    printf("Initial:\n");
    print(prev, size);
    #endif

    /*===============================================================*/
    /* 2: Envio das linhas para os escravos (+linhas para observacao)*/
    MPI_Send(prev, (lines+1)*size, MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD);
    for (i = 1; i < limit_proc-1; i++)
      MPI_Send((prev + (i*lines-1)*size), (lines+2)*size, MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD);
    if (processes > 2)
      MPI_Send((prev + (i*lines-1)*size), (last_lines+1)*size, MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD);

    #ifdef DEBUG
    for (int k = 0; k < steps-1; ++k) {
      MPI_Recv(prev, (lines*size), MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for (i = 1; i < limit_proc-1; i++)
        MPI_Recv((prev + i*lines*size), (lines*size), MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if (processes > 2)
        MPI_Recv((prev + i*lines*size), (last_lines*size), MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      printf("%d ---------\n", k+1);
      print (prev, size);
    }
    #endif

    /*===============================================================*/
    /* 3: Espera pelo cálculo, imprime resultado e desaloca memória  */
    MPI_Recv(prev, (lines*size), MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (i = 1; i < limit_proc-1; i++)
      MPI_Recv((prev + i*lines*size), (lines*size), MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (processes > 2)
      MPI_Recv((prev + i*lines*size), (last_lines*size), MPI_UNSIGNED_CHAR, (i+1), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    #ifdef RESULT
    printf("Final:\n");
    print (prev, size);
    #endif

    free(prev);

  } else {

    /*===============================================================*/
    /* 1: Recebe tamanho, geracoes e linhas por broadcast            */
    int size, steps, lines;
    cell_t *prev, *next, *tmp;

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&steps, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&lines, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /*===============================================================*/
    /* 2: Primeiro e ultimo processos sao casos especiais            */
    if (rank == 1) {
<<<<<<< HEAD
      /*=============================================================*/
      /* processes > 2 => Se ele for o unico escravo, sem comunicacao*/

=======
>>>>>>> 0b9588612eba71db4d9cfa03f00a157e13cc12a0
      MPI_Request next_line;
      MPI_Status st;

      prev = (cell_t *) malloc(sizeof(cell_t) * (lines+1) * size);
      next = (cell_t *) malloc(sizeof(cell_t) * (lines+1) * size);
      MPI_Recv(prev, (lines+1)*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, &st);

      for (int i = 0; i < steps; ++i) {
        /*===========================================================*/
        /* Calcula as linhas que nao dependem de outros processos    */
        play_optimized(prev, next, size, lines+1, 0, lines-2);

        /*===========================================================*/
        /* Espera receber a linha que depende de outros processo     */
        if (i != 0 && processes > 2)
          MPI_Wait(&next_line, &st);
        play_optimized(prev, next, size, lines+1, lines-1, lines-1);

        tmp = next;
        next = prev;
        prev = tmp;

        /* ======================================================= */
        /* Prepara pra receber sem bloquear e comeca envio         */
        if (processes > 2 && i < steps-1) {
          MPI_Irecv((prev + lines*size), size, MPI_UNSIGNED_CHAR, 2, 0, MPI_COMM_WORLD, &next_line);
          MPI_Send((prev + (lines-1)*size), size, MPI_UNSIGNED_CHAR, 2, 0, MPI_COMM_WORLD);
        }

        #ifdef DEBUG
        MPI_Send(prev, lines*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
        #endif
      }

      /*===========================================================*/
      /* Envia resultado final, apenas as linhas que importam.     */
      MPI_Send(prev, lines*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);

    } else if ((rank == processes-1 && processes-1 <= size) || (rank == size && processes-1 > size)) {
      MPI_Request previous_line;
      MPI_Status st;

      MPI_Recv(&lines, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &st);
      prev = (cell_t *) malloc(sizeof(cell_t) * (lines+1) * size);
      next = (cell_t *) malloc(sizeof(cell_t) * (lines+1) * size);
      MPI_Recv(prev, (lines+1)*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, &st);

      for (int i = 0; i < steps; ++i) {
        /*===========================================================*/
        /* Calcula as linhas que nao dependem de outros processos    */
        play_optimized(prev, next, size, lines+1, 2, lines);

        /*===========================================================*/
        /* Espera receber a linha que depende de outros processo     */
        if (i != 0)
          MPI_Wait(&previous_line, &st);
        play_optimized(prev, next, size, lines+1, 1, 1);

        tmp = next;
        next = prev;
        prev = tmp;

        /* ======================================================= */
        /* Prepara pra receber sem bloquear e comeca envio         */
        if (i < steps-1) {
          MPI_Irecv(prev, size, MPI_UNSIGNED_CHAR, rank-1, 0, MPI_COMM_WORLD, &previous_line);
          MPI_Send((prev + size), size, MPI_UNSIGNED_CHAR, rank-1, 0, MPI_COMM_WORLD);
        }

        #ifdef DEBUG
        MPI_Send((prev + size), lines*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
        #endif
      }

      /*===========================================================*/
      /* Envia resultado final, apenas as linhas que importam.     */
      MPI_Send((prev + size), lines*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);

    } else if (rank < size) {
      MPI_Request next_line, previous_line;
      MPI_Status st;

      prev = (cell_t *) malloc(sizeof(cell_t) * (lines+2) * size);
      next = (cell_t *) malloc(sizeof(cell_t) * (lines+2) * size);
      MPI_Recv(prev, (lines+2)*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, NULL);

      for (int i = 0; i < steps; ++i) {
        /*===========================================================*/
        /* Calcula as linhas que nao dependem de outros processos    */
        play_optimized(prev, next, size, lines+2, 2, lines-1);

        /*===========================================================*/
        /* Espera receber a linha que depende de outros processo     */
        if (i != 0) {
          MPI_Wait(&previous_line, &st);
          MPI_Wait(&next_line, &st);
        }
        play_optimized(prev, next, size, lines+2, 1, 1);
        play_optimized(prev, next, size, lines+2, lines, lines);

        tmp = next;
        next = prev;
        prev = tmp;

        /* ======================================================= */
        /* Prepara pra receber sem bloquear e comeca envio         */
        if (i < steps-1) {
          MPI_Irecv(prev, size, MPI_UNSIGNED_CHAR, rank-1, 0, MPI_COMM_WORLD, &previous_line);
          MPI_Irecv((prev + (lines+1)*size), size, MPI_UNSIGNED_CHAR, rank+1, 0, MPI_COMM_WORLD, &next_line);
          MPI_Send((prev + lines*size), size, MPI_UNSIGNED_CHAR, rank+1, 0, MPI_COMM_WORLD);
          MPI_Send((prev + size), size, MPI_UNSIGNED_CHAR, rank-1, 0, MPI_COMM_WORLD);
        }

        #ifdef DEBUG
        MPI_Send((prev + size), lines*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
        #endif
      }

      /*===========================================================*/
      /* Envia resultado final, apenas as linhas que importam      */
      MPI_Send((prev + size), lines*size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);

    }

    /*============================================================*/
    /* Desaloca memoria e vai embora                              */
    if (rank < size) {
      free(prev);
      free(next);
    }
  }

  MPI_Finalize();
  return 0;
}

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

  /* ================================================================ */
  /* Se tiver que calcular a ultima linha, "end" para que nao execute */
  /* dentro do for das linhas seguras, se nao end+1 porque e seguro   */
  end = lines == end_inicial+1? end : end+1;

  /* ============================================================ */
  /* Separamos o calculo em 3 partes:                             */
  /* Inicio == 0 -> Coord. (0, j), cuidar linha superior          */
  /* Intermediarios -> Coord. (i, j), cuidar apenas j [0, size-1] */
  /* Fim == size-1 -> Coord. (size-1, j), cuidar linha inferior   */
  /* Proposito: Muito menos comparacoes, ganho em tempo de exec.  */

  /* ============================================================ */
  /* Caso tenha que calcular a primeira (rank == 1 apenas)        */
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

  /* ================================================================ */
  /* Linhas intermediarias, qlqr processo: i < end+1 (process-1: end) */
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
void print(cell_t * board, int size) {
  for (int j = 0; j < size; ++j) {  // For each row
    for (int i = 0; i < size; ++i)  // Print each column position...
      printf ("%c", board[i*size + j] ? 'x' : ' ');
    printf ("\n");                  // End line
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
  free(s);
}
