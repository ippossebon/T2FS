#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

#define ERRO -1
#define SUCESSO 0

int main()
{
    char pathname[31];
    DIR2 dir;
    int aux;

    printf("Informe o nome do diretório a ser aberto: ");
    fflush(stdin);
    scanf("%s", pathname);

    dir = opendir2(&pathname[0]);
    if(dir == -1){
        printf("[test_open_close_dir] Erro ao abrir o diretório = %s.\n", pathname);
    }
    else{
        printf("[test_open_close_dir] Aberto o diretório = %s, com o handle = %d.\n", pathname, dir);
    }

    aux = closedir2(dir);
    if(aux == 0){
        printf("[test_open_close_dir] Diretório %s fechado corretamente\n", pathname);
    }
    else{
        printf("Erro ao fechar o diretório %s.\n", pathname);
    }

    return 0;
}
