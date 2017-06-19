
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>



int main(int argc, char *argv[]) {

	if (argc < 2)
		return 1;

	int processes, rank;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		int x = atoi(argv[1]);
		int *vetor = (int *) malloc(x*x*sizeof(int));

		MPI_Bcast(&x, 1, MPI_INT, 0, MPI_COMM_WORLD);

		for (int i = 0; i < x; ++i) {
			for (int j = 0; j < x; ++j) {
				vetor[i*x + j] = i*j;
			}
		}

		MPI_Bcast(vetor, x*x, MPI_INT, 0, MPI_COMM_WORLD);
		//MPI_Send(vetor + (x/2)*x, (x/2)*x, MPI_INT, 2, 0, MPI_COMM_WORLD);
		//MPI_Recv(&x, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, NULL);

		free(vetor);
	} else {
		int x_slave;
		MPI_Bcast(&x_slave, 1, MPI_INT, 0, MPI_COMM_WORLD);
		//MPI_Recv(&x_slave, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
		
		int *vetor_s = (int *) malloc(x_slave*x_slave*sizeof(int));
		MPI_Bcast(vetor_s, x_slave*x_slave, MPI_INT, 0, MPI_COMM_WORLD);
		//MPI_Recv(&vetor_s, x_slave*x_slave, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);

		printf("Rank: %d [", rank);
		for (int j = 0; j < x_slave; ++j) {
			printf(" %d ", vetor_s[(rank-1)*x_slave + j]);
		}
		printf("]\n");
		//MPI_Send(&x_slave, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		free(vetor_s);
	}

	printf("%d : chegou na barreira\n", rank);
	//MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}