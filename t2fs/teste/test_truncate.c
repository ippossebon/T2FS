#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[32];
    char buffer[64];
    char buffer2[64] = "Teestando, testando 123456789!";
    FILE2 file;
    int aux;

    printf("Informe o nome do arquivo a ser criado e escrito: ");
    fflush(stdin);
    scanf("%s", file_name);

    file = create2(&file_name[0]);
    if(file == -1){
        printf("[test_truncate] Erro ao criar o arquivo = %s.\n", file_name);
    }
    else{
        printf("[test_truncate] Aberto o arquivo = %s, com o handle = %d.\n", file_name, file);
    }

    aux = write2(file, &buffer2[0], 23);

    if(aux >= 0){
        printf("[test_write] %d bytes escritos no arquivo %s.\n", aux, file_name);
        printf("[test_write] Buffer escrito: %s\n", buffer2);
    }
    else{
        printf("[test_write] Erro ao ler o arquivo %s.\n", file_name);
    }

    aux = read2(file, &buffer[0], 63);

    if(aux != -1){
        buffer[aux] = '\0';
        printf("[test_truncate] %d bytes lidos do arquivo %s.\n", aux, file_name);
        printf("[test_truncate] Buffer lido: %s\n", buffer);
    }
    else{
        printf("[test_truncate] Erro ao ler o arquivo %s.\n", file_name);
    }

    aux = close2(file);
    if(aux == 0){
        printf("[test_truncate] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("[test_truncate] Erro ao fechar o arquivo %s.\n", file_name);
    }

    file = open2(&file_name[0]);
    if(file == -1){
        printf("[test_truncate] Erro ao abrir o arquivo = %s.\n", file_name);
    }
    else{
        printf("[test_truncate] Aberto o arquivo = %s, com o handle = %d.\n", file_name, file);
    }

    /* Reposiciona current_pointer */
    aux = seek2(file, 4);
    if (aux == 0){
        printf("[test_truncate] Ponteiro do arquivo %s reposicionado corretamente\n", file_name);
    }
    else{
        printf("[test_truncate] Erro ao reposionar o ponteiro do arquivo %s.\n", file_name);
    }

    aux = read2(file, &buffer[0], 64-16);
    if(aux != -1){
        buffer[aux] = '\0';
        printf("[test_truncate] %d bytes lidos do arquivo %s.\n", aux, file_name);
        printf("[test_truncate] Buffer lido APÓS SEEK: %s\n", buffer);
    }
    else{
        printf("[test_truncate] Erro ao ler o arquivo %s APÓS SEEK.\n", file_name);
    }

    aux = truncate2(file);
    if (aux == 0){
        printf("[test_truncate] Arquivo %s truncado corretamente\n", file_name);
    }
    else{
        printf("[test_truncate] Erro ao truncar arquivo %s.\n", file_name);
    }

    aux = read2(file, &buffer[0], 15);
    if(aux != -1){
        buffer[aux] = '\0';
        printf("[test_truncate] %d bytes lidos do arquivo %s.\n", aux, file_name);
        printf("[test_truncate] Buffer lido APÓS TRUNCATE: %s\n", buffer);
    }
    else{
        printf("[test_truncate] Erro ao ler o arquivo %s APÓS TRUNCATE.\n", file_name);
    }

    aux = close2(file);
    if(aux == 0){
        printf("[test_truncate] Arquivo %s fechado corretamente\n", file_name);
    }
    else{
        printf("[test_truncate] Erro ao fechar o arquivo %s.\n", file_name);
    }

    return 0;
}
