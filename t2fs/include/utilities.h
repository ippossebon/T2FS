#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#define MAX_OPENED_FILES 20
#define SECTOR_SIZE 256
#define INODE_SIZE 16
#define DIR_SIZE 64

#define ERRO -1
#define SUCESSO 0

#define TYPEVAL_FILE 0x01
#define TYPEVAL_DIR 0x02
#define TYPEVAL_INVALID 0x00

#define LIVRE 0
#define OCUPADO 1

#define	INVALID_PTR	-1

int readSuperBlock(struct t2fs_superbloco *superblock);
int readInode(struct t2fs_inode *actual_inode, int inode_number);
int findFreeINode();
int writeInode(int inode_number, struct t2fs_inode inode);
int writeRecord(struct t2fs_record* record);
int createRecord(BYTE type, char* name, int inode_number);
int isFileNameValid(char* filename);
int formatDirBlock(int block);
int findInBlock(int block, char filename[31]);
int findInDir(int dir_handler, char filename[31]);

#endif
