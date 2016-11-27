#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#define MAX_OPENED_FILES 20
#define SECTOR_SIZE 256
#define ERRO -1
#define SUCESSO 0

int readSuperBlock(struct t2fs_superbloco *superblock);

#endif
