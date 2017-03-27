#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

void imprimir_afiliacao(pid_t filho, pid_t pai) {
  printf("Processo %d filho de %d\n", filho, pai);
}

int main(int argc, char * argv[]) {

  pid_t pid = getpid();
  pid_t pid2 = getpid();

  // Pai cria dois filhos
  for(int i = 0; i < 2; i++) {
    if (pid != 0)
      pid = fork();
  }

  // Filhos do pai cria 2 netos
  if (pid == 0) {
    for(int i = 0; i < 2; i++) {
      if (pid2 != 0)
        pid2 = fork();
    }
  }

  // Todos menos o pai imprime
  if (pid == 0) {
    pid_t meu_nome = getpid();
    pid_t nome_pai = getppid();
    imprimir_afiliacao(meu_nome, nome_pai);
  }
  return 0;
}
