#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[31] = "/arq/a";
    create2(&file_name[0]);

    return 0;
}
