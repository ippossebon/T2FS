#include <stdlib.h>
#include <stdio.h>
#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/t2fs.h"
#include "../include/utilities.h"

/* Retorna 0 se conseguiu ler; -1, caso contrário. */
int readSuperBlock(struct t2fs_superbloco *superblock){
    unsigned char buffer_sector[256];
    unsigned char word_buffer[2];
    unsigned char dword_buffer[4];
    unsigned int s = 0;
    int i;

    if (read_sector(0, &buffer_sector[0]) != 0){
        printf("Erro ao ler setor\n");
        return ERRO;
    }

    for(i = 0; i < 4; i++){
        superblock->id[i] = buffer_sector[i];
    }

    for(i = 4; i < 6; i++){
        word_buffer[i - 4] = buffer_sector[i];
    }
    superblock->version = *(WORD *)word_buffer;

    for(i = 6; i < 8; i++){
        word_buffer[i - 6] = buffer_sector[i];
    }
    superblock->superblockSize = *(WORD *)word_buffer;

    for(i = 8; i < 10; i++){
        word_buffer[i - 8] = buffer_sector[i];
    }
    superblock->freeBlocksBitmapSize = *(WORD *)word_buffer;

    for(i = 10; i < 12; i++){
        word_buffer[i - 10] = buffer_sector[i];
    }
    superblock->freeInodeBitmapSize = *(WORD *)word_buffer;

    for(i = 12; i < 14; i++){
        word_buffer[i - 12] = buffer_sector[i];
    }
    superblock->inodeAreaSize = *(WORD *)word_buffer;

    for(i = 14; i < 16; i++){
        word_buffer[i - 14] = buffer_sector[i];
    }
    superblock->blockSize = *(WORD *)word_buffer;

    for(i = 16; i < 20; i++){
        dword_buffer[i - 16] = buffer_sector[i];
    }
    superblock->diskSize = *(DWORD *)dword_buffer;

    printf("id = %c%c%c%c\n", superblock->id[0], superblock->id[1], superblock->id[2], superblock->id[3]);
    printf("version = %d\n", superblock->version);
    printf("superblockSize = %d\n", superblock->superblockSize);
    printf("blocks_bitmap_size = %d\n", superblock->freeBlocksBitmapSize);
    printf("inodes_bitmap_size = %d\n", superblock->freeInodeBitmapSize);
    printf("inodes_area_size = %d\n", superblock->inodeAreaSize);
    printf("sectors_per_block = %d\n", superblock->blockSize);
    printf("total_sectors_count = %d\n", superblock->diskSize);

    return 0;
}
