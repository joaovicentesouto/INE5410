#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int compute_pi(int, int);

int main(int argc, char **argv){
  int i;
  int size, rank;
  int pontos;
  int pontos_no_circulo;
  int divisao;

  if(argc != 2){
    if (rank == 0) {
      printf("Uso:\n");
      printf("\t%s <numero de pontos a serem sorteados>\n", argv[0]);
    }
    return 1;
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  pontos = atoi(argv[1]);
  divisao = (int) pontos/size;

  if (rank == size-1 && rank != 0) {
    int inicio, fim;
    inicio = rank*divisao;
    fim = pontos-1;
    printf("Quant %d\n", divisao);
    printf("Rank %d: inic: %d e fim: %d\n", rank, inicio, fim);

    pontos_no_circulo = compute_pi(inicio, fim);
    MPI_Send(&pontos_no_circulo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  } else if (size != 1){
    int inicio, fim;
    inicio = rank*divisao;
    fim = (rank+1)*divisao-1;
    printf("Rank %d: inic: %d e fim: %d\n", rank, inicio, fim);

    pontos_no_circulo = compute_pi(inicio, fim);
    if (rank != 0)
      MPI_Send(&pontos_no_circulo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  } else {
    pontos_no_circulo = compute_pi(0, pontos);
  }


  // calcula a aproximacao de Pi baseado nos pontos sorteados
  if (rank == 0) {
    int recebe;
    for (int i = 1; i < size; ++i) {
      MPI_Recv(&recebe, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      pontos_no_circulo += recebe;
    }
    printf("Pi = %.040f\n", ((double)pontos_no_circulo/(double)pontos)*4);
  }

  MPI_Finalize();
  return 0;
}

int compute_pi(int seed, int pontos){
  int i;
  int pontos_no_circulo;
  double x, y;

  pontos_no_circulo = 0;
  srand(seed);
  
  for(i=seed; i<=pontos; i++){
  	// sorteia um ponto: coordenadas x e y dentro do quadrado
  	// consideramos que R = 1, então x e y pertencem ao intervalo [0; 1]
    x = (double)rand()/(double)(RAND_MAX);
    y = (double)rand()/(double)(RAND_MAX);      
    
    // verifica se o ponto sorteado encontra-se dentro do circulo
    // um ponto (x, y) esta dentro do circulo se: x^2 + y^2 < R^2
    // nesse caso, consideramos R = 1
    if( (x*x + y*y) < 1 ){
      pontos_no_circulo++;
    }      
  }
  
  return pontos_no_circulo;
}
