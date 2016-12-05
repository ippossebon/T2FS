#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[32];
    char buffer[64];
    char buffer2[64] = "Teestando, teeeestando!";
    FILE2 file;
    int aux;

    printf("Informe o nome do arquivo a ser criado e escrito: ");
    fflush(stdin);
    scanf("%s", file_name);

    file = create2(&file_name[0]);
    if(file == -1){
        printf("[test_write] Erro ao criar o arquivo = %s.\n", file_name);
    }
    else{
        printf("[test_write] Aberto o arquivo = %s, com o handle = %d.\n", file_name, file);
    }

    aux = read2(file, &buffer[0], 63);

    if(aux != -1){
        buffer[aux] = '\0';
        printf("[test_write] %d bytes lidos do arquivo %s.\n", aux, file_name);
        printf("[test_write] Buffer lido: %s\n", buffer);
    }
    else{
        printf("[test_write] Erro ao ler o arquivo %s.\n", file_name);
    }

    printf("teste1\n");
    aux = close2(file);
    if(aux == 0){
        printf("[test_write] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("[test_write] Erro ao fechar o arquivo %s.\n", file_name);
    }

    printf("teste1\n");

    file = open2(&file_name[0]);
    if(file == -1){
        printf("[test_write] Erro ao abrir o arquivo = %s.\n", file_name);
    }
    else{
        printf("[test_write] Aberto o arquivo = %s, com o handle = %d.\n", file_name, file);
    }


    aux = write2(file, &buffer2[0], 23);

    if(aux >= 0){
        printf("[test_write] %d bytes escritos no arquivo %s.\n", aux, file_name);
        printf("[test_write] Buffer escrito: %s\n", buffer2);
    }
    else{
        printf("[test_write] Erro ao ler o arquivo %s.\n", file_name);
    }

    aux = close2(file);
    if(aux == 0){
        printf("[test_write] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("[test_write] Erro ao fechar o arquivo %s.\n", file_name);
    }

    file = open2(&file_name[0]);
    if(file == -1){
        printf("[test_write] Erro ao abrir o arquivo = %s.\n", file_name);
    }
    else{
        printf("[test_write] Aberto o arquivo = %s, com o handle = %d.\n", file_name, file);
    }


    aux = read2(file, &buffer[0], 63);

    if(aux != -1){
        buffer[aux] = '\0';
        printf("[test_write] %d bytes lidos do arquivo %s.\n", aux, file_name);
        printf("[test_write] Buffer lido: %s.\n", buffer);
    }
    else{
        printf("[test_write] Erro ao ler o arquivo %s.\n", file_name);
    }

    aux = close2(file);
    if(aux == 0){
        printf("[test_write] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("[test_write] Erro ao fechar o arquivo %s.\n", file_name);
    }

    return 0;
}
