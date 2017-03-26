- Exercícios práticos Processos no Linux

- INE5410 - Programação Concorrente Prof. Márcio Castro

- 1. Dicas úteis
Para realizar os exercícios a seguir, você necessitará de:
    • Um editor de texto para escrever o seu código: escolha o editor de sua preferência (vim, emacs, nano, pico, gedit, ...)
    • Um compilador: usaremos o GCC (GNU C Compiler). • Um terminal: para compilar e executar o seu programa.

A sintaxe para compilar um programa em C é a seguinte:
$ gcc -o <nome_arquivo_binario> <nome_arquivo_contendo_o_código>

Por exemplo: para criar um programa chamado meu_programa a partir de um
código em C chamado meu_programa.c faça:
$ gcc -o meu_programa meu_programa.c

Se tudo ocorrer bem, ao final da compilação será gerado um arquivo binário chamado meu_programa. Para executá-lo, digite:
$ ./meu_programa

Você deverá incluir as seguintes bibliotecas nos seus códigos:
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

- 2. Exercícios

Exercício 1
    Escreva um programa em C que cria um processo utilizando a chamada de sistema fork().
    Ambos os processos pai e filho deverão impri- mir na tela a frase “Novo processo criado!”.
    Você deverá utilizar apenas um printf().

Exercício 2
    Escreva um programa em C no qual o processo pai cria 4 pro- cessos filhos.
    Para cada filho criado, o processo pai deverá imprimir na tela “Processo pai XX criou YY”,
onde XX é o PID do pai e YY o PID do filho. Além disso, os processos filhos deverão imprimir
na tela “Processo filho XX”, onde XX é o PID do filho. Dica: utilize a função getpid() para
retornar o PID do processo corrente.

Exercício 3
    Escreva um programa em C no qual o processo pai cria 2 processos filhos e cada um dos
processos filhos cria mais 2 processos filhos. Os processos filhos deverão imprimir na
tela “Processo XX filho de YY”, onde XX é o PID do processo e YY é o PID do pai do processo.
    Dica: utilize a função getpid() para retornar o PID do processo corrente e getppid() para
retornar o PID do pai do processo corrente.
