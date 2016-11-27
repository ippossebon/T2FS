#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#define MAX_OPENED_FILES 20
#define SECTOR_SIZE 256
#define INODE_SIZE 16
#define ERRO -1
#define SUCESSO 0

int readSuperBlock(struct t2fs_superbloco *superblock, int *inode_start_position, int *inode_sectors, int *block_to_sectors);
int readInode(struct t2fs_inode *actual_inode, int inode_number, int inode_start_position, int inode_sectors);

#endif
