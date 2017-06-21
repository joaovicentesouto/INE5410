#include <stdio.h>
#include <stdlib.h>

#define NRA 4                 /* number of rows in matrix A */
#define NCA 4                 /* number of columns in matrix A */
#define NCB 4                 /* number of columns in matrix B */

int main (int argc, char *argv[])
{
  int i, j, k;

  double **a = (double **) malloc(sizeof(double *) * NRA);
  double **b = (double **) malloc(sizeof(double *) * NCA);
  double **c = (double **) malloc(sizeof(double *) * NRA);

  #pragma omp parallel
  {
    /* matrix A to be multiplied */
    #pragma omp for nowait
    for (i = 0; i < NRA; i++)
      a[i] = (double *) malloc(sizeof(double) * NCA);

    /* matrix B to be multiplied */
    #pragma omp for nowait
    for (i = 0; i < NCA; i++)
      b[i] = (double *) malloc(sizeof(double) * NCB);

    /* result matrix C */
    #pragma omp for
    for (i = 0; i < NRA; i++)
      c[i] = (double *) malloc(sizeof(double) * NCB);

    #pragma omp single
    printf("Initializing matrices...\n");

    /*** Initialize matrices ***/
    #pragma omp for private(i, j)
    for (i=0; i<NRA; i++)
      for (j=0; j<NCA; j++)
        a[i][j]= i+j;

    #pragma omp single
    {
      printf("******************************************************\n");
      printf("A:\n");

      for (i=0; i<NRA; i++) {
        for (j=0; j<NCB; j++)
          printf("%10.2f  ", a[i][j]);
        printf("\n");
      }

      printf("******************************************************\n");
    }

    #pragma omp for private(i, j)
    for (i=0; i<NCA; i++)
      for (j=0; j<NCB; j++)
        b[i][j]= i*j;

    #pragma omp single
    {
      printf("******************************************************\n");
      printf("B:\n");

      for (i=0; i<NRA; i++) {
        for (j=0; j<NCB; j++)
          printf("%10.2f  ", b[i][j]);
        printf("\n");
      }

      printf("******************************************************\n");
      printf ("Done.\n");
    }

    #pragma omp for private(i, j)
    for (i=0; i<NRA; i++)
      for (j=0; j<NCB; j++)
        c[i][j]= 0;

    /*** Do the matrix-matrix multiplication ***/
      #pragma omp for nowait private(i, j, k)
    for (i=0; i<NRA; i++)
      for(j=0; j<NCB; j++)
        for (k=0; k<NCA; k++)
          c[i][j] += a[i][k] * b[k][j];

    /*** Print results ***/
    #pragma omp single
    {
      printf("******************************************************\n");
      printf("Result Matrix:\n");

      for (i=0; i<NRA; i++) {
        for (j=0; j<NCB; j++)
          printf("%10.2f  ", c[i][j]);
        printf("\n");
      }

      printf("******************************************************\n");
      printf ("Done.\n");
    }

    #pragma omp for nowait
    for (i = 0; i < NRA; i ++)
      free(a[i]);
    

    #pragma omp for nowait
    for (i = 0; i < NCA; i ++)
      free(b[i]);

    #pragma omp for nowait
    for (i = 0; i < NRA; i ++)
      free(c[i]);
  }

  free(a);
  free(b);
  free(c);

  return 0;
}
