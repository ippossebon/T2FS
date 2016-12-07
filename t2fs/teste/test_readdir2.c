#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char pathname[32];
    DIR2 dir;
    int aux;

    printf("Informe o nome do diretório a ser lido: ");
    fflush(stdin);
    scanf("%s", pathname);

    dir = opendir2(&pathname[0]);
    if(dir == -1){
        printf("[test_readdir2] Erro ao abrir o diretório = %s.\n", pathname);
    }
    else{
        printf("[test_readdir2] Aberto o diretório = %s, com o handle = %d.\n", pathname, dir);
    }

    DIRENT2 dentry;
    aux = readdir2(dir, &dentry);

    if(aux == ERRO){
        printf("[test_readdir2] Erro ao ler entrada do diretório.\n");
    }

    printf("[test_readdir2] Entrada lida do diretório %s :\n", pathname);
    printf("filename = %s\n", dentry.name);
    printf("file type = %d\n", dentry.fileType);
    printf("file size = %d\n", dentry.fileSize);

    aux = readdir2(dir, &dentry);

    if(aux == ERRO){
        printf("[test_readdir2] Erro ao ler entrada do diretório.\n");
    }

    printf("[test_readdir2] Entrada lida do diretório %s :\n", pathname);
    printf("filename = %s\n", dentry.name);
    printf("file type = %d\n", dentry.fileType);
    printf("file size = %d\n", dentry.fileSize);

    aux = readdir2(dir, &dentry);

    if(aux == ERRO){
        printf("[test_readdir2] Erro ao ler entrada do diretório.\n");
    }

    printf("[test_readdir2] Entrada lida do diretório %s :\n", pathname);
    printf("filename = %s\n", dentry.name);
    printf("file type = %d\n", dentry.fileType);
    printf("file size = %d\n", dentry.fileSize);

    aux = readdir2(dir, &dentry);

    if(aux == ERRO){
        printf("[test_readdir2] Erro ao ler entrada do diretório.\n");
    }

    printf("[test_readdir2] Entrada lida do diretório %s :\n", pathname);
    printf("filename = %s\n", dentry.name);
    printf("file type = %d\n", dentry.fileType);
    printf("file size = %d\n", dentry.fileSize);
    
    return 0;
}
