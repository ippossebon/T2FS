#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include "../include/t2fs.h"

/*
#include "t2fs.h"
#include "apidisk.h"
#include "bitmap2.h"

struct t2fs_superbloco superblock;
struct t2fs_record current_working_directory;

 Informações contidas no superbloco. Quantidade em número de setores
int blocks_bitmap_size;
int inodes_bitmap_size;
int inodes_area_size;
int sectors_per_block;
int total_sectors_count; */

int readSuperBlock(struct t2fs_superbloco *superblock);
struct t2fs_record* createRecord(BYTE type, char* name, DWORD file_size_in_blocks, DWORD file_size_in_bytes, int inode_number);

#endif
