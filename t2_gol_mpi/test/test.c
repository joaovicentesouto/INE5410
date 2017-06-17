
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>



int main(int argc, char const *argv[]) {

	if (argc < 2)
		return 1;

	int processes, rank;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		int x = atoi(argv[1]);
		int vetor[x][x];

		MPI_Send(&x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

		for (int i = 0; i < x; ++i) {
			for (int j = 0; j < x; ++j) {
				vetor[i][j] = i*j;
			}
		}

		MPI_Send(&vetor[0], (x-1)*x, MPI_INT, 1, 0, MPI_COMM_WORLD);
		//MPI_Recv(&x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, NULL);

	} else {
		int x_slave;
		MPI_Recv(&x_slave, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
		
		int vetor_s[x_slave-1][x_slave];
		MPI_Recv(&vetor_s, (x_slave-1)*x_slave, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);

		for (int i = 0; i < x_slave-1; ++i) {
			printf("[");
			for (int j = 0; j < x_slave; ++j) {
				printf(" %d ", vetor_s[i][j]);
			}
			printf("]\n");
		}
		//MPI_Send(&x_slave, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

	}

	printf("%d : chegou na barreira\n", rank);
	//MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}