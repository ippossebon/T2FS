#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[32];
    char buffer[64];
    char buffer2[64] = "Este e um buffer de 64 caracteres para teste! 11223344556677889";
    FILE2 file;
    int aux, i;

    printf("Informe o nome do arquivo a ser criado e escrito: ");
    fflush(stdin);
    scanf("%s", file_name);

    file = create2(&file_name[0]);
    if(file == -1){
        printf("[test_write2] Erro ao criar o arquivo = %s.\n", file_name);
    }
    else{
        printf("[test_write2] Aberto o arquivo = %s, com o handle = %d.\n", file_name, file);
    }

    for(i = 0; i < 192; i++){
        aux = write2(file, &buffer2[0], 64);

        if(aux >= 0)
            printf("[test_write2] write for %d.\n", i);
        else
            printf("[test_write2] Erro no write for %d.\n", i);
    }

    aux = close2(file);
    if(aux == 0){
        printf("[test_write2] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("[test_write2] Erro ao fechar o arquivo %s.\n", file_name);
    }

    file = open2(&file_name[0]);
    if(file == -1){
        printf("[test_write2] Erro ao abrir o arquivo = %s.\n", file_name);
    }
    else{
        printf("[test_write2] Aberto o arquivo = %s, com o handle = %d.\n", file_name, file);
    }

    for(i = 0; i < 192; i++){
        aux = read2(file, &buffer[0], 64);

        if(aux != -1){
            printf("[test_write2] read for %d.\n", i);
            printf("[test_write] Buffer lido: %s.\n", buffer);
        }
        else
            printf("[test_write2] Erro no read for %d.\n", i);
    }

    aux = close2(file);
    if(aux == 0){
        printf("[test_write] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("[test_write] Erro ao fechar o arquivo %s.\n", file_name);
    }

    aux = delete2(&file_name[0]);
    if (aux == 0){
        printf("Arquivo apagado com sucesso\n");
    }
    else{
        printf("Erro ao apagar arquivo\n");
    }

    return 0;
}
