#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#define MAX_OPENED_FILES 20
#define SECTOR_SIZE 256
#define ERRO -1
#define SUCESSO 0

int readSuperBlock(struct t2fs_superbloco *superblock);
struct t2fs_record* createRecord(BYTE type, char* name, DWORD file_size_in_blocks, DWORD file_size_in_bytes, int inode_number);

#endif
