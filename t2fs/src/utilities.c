#include <stdlib.h>
#include "apidisk.h"
#include "utilities.h"

/* Retorna 0 se conseguiu ler; -1, caso contrário. */
int readSuperBlock(){
    if (read_sector(0, (unsigned char*) &superblock) != 0){
        return -1;
    }

    blocks_bitmap_size = superblock.freeBlocksBitmapSize;
    inodes_bitmap_size = superblock.freeInodeBitmapSize;
    inodes_area_size = superblock.inodeAreaSize;
    sectors_per_block = superblock.blockSize;
    total_sectors_count = superblock.diskSize;
    sectors_per_block = superblock.blockSize/SECTOR_SIZE;

    return 0;
}
