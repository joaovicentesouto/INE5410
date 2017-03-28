#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

void imprimir_afiliacao(pid_t filho, pid_t pai) {

}

int main(int argc, char * argv[]) {

  pid_t pid = getpid();
  pid_t pid2 = getpid();

  // Pai cria dois filhos
  for(int i = 0; i < 2; i++) {
    if (pid != 0)
      pid = fork();
    if(pid == 0)
      break;
  }

  // Filhos do pai cria 2 netos
  if (pid == 0) {
    for(int i = 0; i < 2; i++) {
      if (pid2 != 0)
        pid2 = fork();
      if (pid2 == 0)
        break;
    }
  }

  // Todos menos o pai imprime
  if (pid == 0)
    printf("Processo %d filho de %d\n", getpid(), getppid());

  // Processo Pai (qualquer) espera seus filhos
  while (wait(NULL) != -1) {
  }

  return 0;
}
