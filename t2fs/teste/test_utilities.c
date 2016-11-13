#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    int ret;
	ret = readSuperBlock();

    if (ret == -1){
        printf("ERRO em readSuperBlock\n");
    }

    return 0;
}
