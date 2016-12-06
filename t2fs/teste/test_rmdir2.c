#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char dirname[31] = "/sub/dir";
    char file1[31] = "/sub/dir/file1";
    char file2[31] = "/sub/dir/file2";


    mkdir2(&dirname[0]);
    create2(&file1[0]);
    create2(&file2[0]);

    int aux = rmdir2(&dirname[0]);

    if (aux == ERRO){
        printf("Erro ao apagar diretorio %s\n", dirname);
    }

    aux = delete2(&file1[0]);
    aux = delete2(&file2[0]);

    aux = rmdir2(&dirname[0]);
    printf("Diretorio apagado corretamente\n");

    return 0;
}
