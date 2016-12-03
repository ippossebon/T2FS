#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[31];
    FILE2 file;
    int aux;

    printf("Informe o nome do arquivo a ser criado: ");
    fflush(stdin);
    scanf("%s", file_name);

    file = create2(&file_name[0]);

    aux = close2(file);
    if(aux == 0){
        printf("[test_open_close] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("Erro ao fechar o arquivo %s.\n", file_name);
    }

    return 0;
}
