//
//  main.c
//  Atividade não presencial (14/mar)
//  Programação Concorrente
//
//  Criado por João Vicente Souto
//  Matrícula: 16105151

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int *multiplica_vetores(int tamanho_vetor, int *A, int *B) {

    int *C = malloc(tamanho_vetor * sizeof(int));
    for (int i=0; i < tamanho_vetor; i++)
        C[i] = A[i] * B[i];

    return C;
}

int *cria_vetor_randomico(int tamanho_vetor) {

    int *vetor = malloc(tamanho_vetor * sizeof(int));
    for (int i=0; i < tamanho_vetor; i++)
      vetor[i] = rand() % 10;

    return vetor;
}

void printf_vetor(char nome_vetor, int tamanho_vetor, int *vetor) {

    printf("%c = [", nome_vetor);

    for(int i=0; i < tamanho_vetor-1; i++)
        printf("%d, ", vetor[i]);

    printf("%d]\n", vetor[tamanho_vetor-1]);
}

int main(int argc, const char * argv[]) {

    const int tamanho_vetor = argc > 1? atoi(argv[1]) : 5;
    srand(time(NULL));
    int *A = cria_vetor_randomico(tamanho_vetor);
    int *B = cria_vetor_randomico(tamanho_vetor);
    int *C = multiplica_vetores(tamanho_vetor, A, B);

    printf_vetor('A', tamanho_vetor, A);
    printf_vetor('B', tamanho_vetor, B);
    printf_vetor('C', tamanho_vetor, C);

    free(A);
    free(B);
    free(C);

    return 0;
}
