
#include <stdlib.h>
#include "stdio.h"
#include "math.h"
#define  MAX_THREADS 10

int main(int argc, char * argv[]) {

    int row, n_row, col, n_col, count=0, n=20;

    row = MAX_THREADS;
    col = 1;

    for (int i=1; i<=MAX_THREADS/2; i++)
      for (int j=(int)sqrt(MAX_THREADS); j<=MAX_THREADS; j++)
        if (i*j == MAX_THREADS && abs(i-j) < abs(row-col)) {
            row = i;
            col = j;
        }

    n_row = (int) n/row;
    n_row = n_row*row == n? n_row : n_row+1;
    n_col = (int) n/col;
    n_col = n_col*col == n? n_col : n_col+1;
    printf("row = %d\n", row);
    printf("col = %d\n", col);
    printf("n_row = %d\n", n_row);
    printf("n_col = %d\n\n", n_col);

    for (int k = 0; k < MAX_THREADS; ++k) {
      printf("\nk = %d\n", k);

      count = k%col == 0 && k!=0? count+1 : count;
      printf("i.%d   = %d\n", k, (count%row)*n_row);
      int i_final = (count%row+1)*n_row-1;
      printf("i.%d+1 = %d\n", k, (i_final < n? i_final : n-1));

      printf("j.%d   = %d\n", k, (k%col)*n_col);
      int j_final = (k%col+1)*n_col-1;
      printf("j.%d+1 = %d\n", k, (j_final < n? j_final : n-1));
    }

    return 0;
}
