#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[32];
    int aux;

    printf("Informe o nome do arquivo a ser apagado: ");
    fflush(stdin);
    scanf("%s", file_name);

    aux = delete2 (&file_name[0]);
    if(aux == 0){
        printf("[test_delete] Arquivo %s apagado corretamente\n", file_name);
    }
    else{
        printf("[test_delete] Erro ao apagar o arquivo %s.\n", file_name);
    }

    return 0;
}
