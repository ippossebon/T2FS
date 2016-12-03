#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char pathname[31] = "/sub/dirIsadoraTeste/";
    int aux = rmdir2(&pathname[0]);

    if (aux == ERRO){
        printf("Erro ao remover diretório\n");
    }

    return 0;
}
