
#include <stdlib.h>
#include "stdio.h"
#include "math.h"
#define  MAX_THREADS 7
#define  SIZE 11

int main(int argc, char * argv[]) {

    int row, col;

    row = MAX_THREADS;
    col = 1;

    for (int i = 1; i <= MAX_THREADS/2; i++) {
      for (int j = sqrt(MAX_THREADS); j <= MAX_THREADS; j++)
        if (i*j == MAX_THREADS && abs(i-j) < abs(row-col)) {
            row = i;
            col = j;
        }
    }

    int size_row, size_col, count=0;

    size_row = (int) SIZE/row;
    size_row = size_row*row == SIZE ? size_row : size_row+1;
    size_col = (int) SIZE/col;
    size_col = size_col*col == SIZE ? size_col : size_col+1;
    printf("row = %d\n", row);
    printf("col = %d\n", col);
    printf("size_row = %d\n", size_row);
    printf("size_col = %d\n\n", size_col);

    for (int k = 0; k < MAX_THREADS; ++k) {
      printf("\nk = %d\n", k);

      int tmp;
      count = k%col == 0 && k!=0? count+1 : count;

      tmp = (count%row)*size_row;
      printf("i.%d   = %d\n", k, (tmp < SIZE ? tmp : SIZE-1));
      tmp = (count%row+1)*size_row-1;
      printf("i.%d+1 = %d\n", k, (tmp < SIZE ? tmp : SIZE-1));

      tmp = (k%col)*size_col;
      printf("j.%d   = %d\n", k, (tmp < SIZE ? tmp : SIZE-1));
      tmp = (k%col+1)*size_col-1;
      printf("j.%d+1 = %d\n", k, (tmp < SIZE ? tmp : SIZE-1));
    }

    return 0;
}
