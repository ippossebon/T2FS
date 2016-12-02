#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#define TYPEVAL_INVALIDO    0x00
#define TYPEVAL_REGULAR     0x01
#define TYPEVAL_DIRETORIO   0x02

#define	INVALID_PTR	-1

#define MAX_OPENED_FILES 20
#define SECTOR_SIZE 256
#define INODE_SIZE 16
#define DIR_SIZE 64

#define ERRO -1
#define SUCESSO 0
#define LIVRE 0
#define OCUPADO 1

int readSuperBlock(struct t2fs_superbloco *superblock);
int readInode(struct t2fs_inode *actual_inode, int inode_number);
int findFreeINode();
int writeInode(int inode_number, struct t2fs_inode inode);
int writeRecord(struct t2fs_record* record);
int createRecord(BYTE type, char* name, int inode_number);
int isFileNameValid(char* filename);
int formatDirBlock(int block);
int findParentDir(char* name);
int findInBlock(int block, char* name, int* dir);
int findInDir(int inode_number, char* name, int* dir);
int formatDirBlock(int block);

#endif
