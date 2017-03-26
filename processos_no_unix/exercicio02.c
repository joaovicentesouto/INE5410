#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main(int argc, char * argv[]) {

    pid_t pid = getpid();
    for(int i = 0; i < 4; i++) {
        if (pid != 0) {
            pid = fork();
            if (pid != 0)
                printf("Processo pai %d criou %d\n", getpid(), pid);
        }
    }

    return 0;
}
