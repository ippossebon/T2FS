#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[32];
    FILE2 file;
    int aux;

    printf("Informe o nome do arquivo a ser criado: ");
    fflush(stdin);
    scanf("%s", file_name);

    file = create2(&file_name[0]);
    if(file == -1){
        printf("[test_create2] Erro ao criar o arquivo = %s.\n", file_name);
    }
    else{
        printf("[test_create2] Aberto o arquivo = %s, com o handle = %d.\n", file_name, file);
    }

    aux = close2(file);
    if(aux == 0){
        printf("[test_create2] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("[test_create2] Erro ao fechar o arquivo %s.\n", file_name);
    }

    return 0;
}
