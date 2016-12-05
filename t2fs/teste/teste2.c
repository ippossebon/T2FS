#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

#define ERRO -1
#define SUCESSO 0

int main()
{
    int aux;
    char dir_name[32] = "/sub/subdir1/";
    DIR2 dir;

    aux = mkdir2(&dir_name[0]);

    if (aux == SUCESSO){
        printf("Diretório %s criado com sucesso.\n", dir_name);
    }

    dir = opendir2(&dir_name[0]);
    if(dir == -1){
        printf("Erro ao abrir o diretório = %s.\n", dir_name);
    }
    else{
        printf("Aberto o diretório = %s, com o handle = %d.\n", dir_name, dir);
    }

    char filename[32] = "/sub/subdir1/arquivoRegular1";
    aux = create2(&filename[0]);

    if (aux == SUCESSO){
        printf("Arquivo %s criado com sucesso\n", filename);
    }

    DIRENT2 dentry;
    aux = readdir2(dir, &dentry);

    if(aux == ERRO){
        printf("[test_readdir2] Erro ao ler entrada do diretório.\n");

    }

    printf("[test_readdir2] Entrada lida do diretóio %s :\n", dir_name);
    printf("filename = %s\n", dentry.name);
    printf("file type = %d\n", dentry.fileType);
    printf("file size = %d\n", dentry.fileSize);

    return 0;
}
