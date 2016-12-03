#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char file_name[31] = "/sub/i";
    create2(&file_name[0]);

    return 0;
}
