#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[31];
    FILE2 file;
    int aux;

    printf("Informe o nome do arquivo a ser aberto: ");
    fflush(stdin);
    scanf("%s", file_name);

    file = open2(&file_name[0]);
    if(file == -1){
        printf("[test_open_close] Erro ao abrir o arquivo = %s.\n", file_name);
    }
    else{
        printf("[test_open_close] Aberto o arquivo = %s, com o handle = %d.\n", file_name, file);
    }

    aux = close2(file);
    if(aux == 0){
        printf("[test_open_close] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("[test_open_close] Erro ao fechar o arquivo %s.\n", file_name);
    }

    return 0;
}
