#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[32];
    char buffer[64];
    FILE2 file;
    int aux;

    printf("Informe o nome do arquivo a ser lido: ");
    fflush(stdin);
    scanf("%s", file_name);

    file = open2(&file_name[0]);

    aux = read2(file, &buffer[0], 63);
    buffer[63] = '\0';

    if(aux >= 0){
        printf("[test_read] %d bytes lidos do arquivo %s.\n", aux, file_name);
        printf("[test_read] Buffer lido: %s.\n", buffer);
    }
    else{
        printf("[test_read] Erro ao ler o arquivo %s.\n", file_name);
    }

    return 0;
}
