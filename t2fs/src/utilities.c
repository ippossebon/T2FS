#include <stdlib.h>
#include <stdio.h> // retirar
#include "apidisk.h"
#include "utilities.h"

/* Retorna 0 se conseguiu ler; -1, caso contrário. */
int readSuperBlock(){
    if (read_sector(0, (unsigned char*) &superblock) != 0){
        printf("Erro ao ler setor\n");
        return -1;
    }

    blocks_bitmap_size = superblock.freeBlocksBitmapSize;
    inodes_bitmap_size = superblock.freeInodeBitmapSize;
    inodes_area_size = superblock.inodeAreaSize;
    sectors_per_block = superblock.blockSize;
    total_sectors_count = superblock.diskSize;

    printf("blocks_bitmap_size = %d\n", blocks_bitmap_size);
    printf("inodes_bitmap_size = %d\n", inodes_bitmap_size);
    printf("inodes_area_size = %d\n", inodes_area_size);
    printf("sectors_per_block = %d\n", sectors_per_block);
    printf("total_sectors_count = %d\n", total_sectors_count);

    return 0;
}
