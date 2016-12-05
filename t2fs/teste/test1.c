#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

#define ERRO -1
#define SUCESSO 0

int main()
{
    int aux;
    char dir_name[32] = "/sub/subdir1/";
    aux = mkdir2(&dir_name[0]);

    if (aux == SUCESSO){
        printf("Diretório %s criado com sucesso.\n", dir_name);
    }

    char filename[32] = "/sub/subdir1/arquivoRegular1";
    aux = create2(&filename[0]);

    if (aux == SUCESSO){
        printf("Arquivo %s criado com sucesso\n", filename);
    }

    return 0;
}
