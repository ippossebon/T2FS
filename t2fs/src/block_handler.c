#include <stdlib.h>
#include "t2fs.h"

t2fs_superbloco superbloco;
t2fs_record current_working_directory;


int readSuperBlock(){
    if (read_sector(0, (char*) &superbloco) != 0){
        return -1;
    }



    return 0;
}
