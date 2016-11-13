#include <stdlib.h>
#include <stdio.h>
#include "apidisk.h"
#include "bitmap2.h"
#include "t2fs.h"
#include "utilities.h"

/* Retorna 0 se conseguiu ler; -1, caso contrário. */
int readSuperBlock(t2fs_superbloco *superblock){
    unsigned char buffer_sector[256];
    int i;

    if (read_sector(0, &buffer_sector[0]) != 0){
        printf("Erro ao ler setor\n");
        return -1;
    }

    for(i = 0; i < 4; i++){
        superblock->id[i] = buffer_sector[i];
    }

    superblock->version;
    superblock->superblockSize;
    superblock->freeBlocksBitmapSize;
    superblock->freeInodeBitmapSize;
    superblock->inodeAreaSize;
    superblock->blockSize;
    superblock->diskSize;

    printf("id = %s\n", superblock.id);
    printf("version = %d\n", superblock.version);
    printf("superblockSize = %d\n", superblock.superblockSize);
    printf("blocks_bitmap_size = %d\n", superblock.freeBlocksBitmapSize);
    printf("inodes_bitmap_size = %d\n", superblock.freeInodeBitmapSize);
    printf("inodes_area_size = %d\n", superblock.inodeAreaSize);
    printf("sectors_per_block = %d\n", superblock.blockSize);
    printf("total_sectors_count = %d\n", superblock.diskSize);

    return 0;
}
