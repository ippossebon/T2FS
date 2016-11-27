#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#define	INVALID_PTR	-1
#define MAX_OPENED_FILES 20
#define SECTOR_SIZE 256
#define INODE_SIZE 16
#define ERRO -1
#define SUCESSO 0

int readSuperBlock(struct t2fs_superbloco *superblock);
int readInode(struct t2fs_inode *actual_inode, int inode_number);
int findInBlock(int block, char *filename);

#endif
