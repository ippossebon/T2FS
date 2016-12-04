#include <stdlib.h>
#include <stdio.h>
#include "t2fs.h"

int main()
{
    char dir_name[31] = "/sub/dirIsadoraTeste/";
    rmdir2(&dir_name[0]);

    return 0;
}
