#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#define TYPEVAL_INVALIDO    0x00
#define TYPEVAL_REGULAR     0x01
#define TYPEVAL_DIRETORIO   0x02

#define	INVALID_PTR	-1
#define TYPEVAL_INVALIDO    0x00
#define TYPEVAL_REGULAR     0x01
#define TYPEVAL_DIRETORIO   0x02

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
int writeRecord(struct t2fs_record* record_to_write, struct t2fs_record* parent_record, struct record_location* location);
int createRecord(BYTE type, char* name, int inode_number);
int isFileNameValid(char* filename);
int formatDirBlock(int block);
int findRecord(char* name, struct record_location* location);
int findInBlock(int block, char* name, int* dir, struct record_location* location);
int findInDir(int inode_number, char* name, int* dir, struct record_location* location);
int findInList(int block, char* name, int* dir, struct record_location* location);
int findInListDouble(int block, char* name, int* dir, struct record_location* location);
int formatDirBlock(int block);
int allocNewBlock(BYTE type);
int findInvalidRecordInINode(struct t2fs_inode* inode, struct record_location* location, struct t2fs_record* parent_record);
int findInvalidRecordInBlock(int block, struct record_location* location);
int findInvalidRecordInList(int block, struct record_location* location);
int findInvalidRecordInListDouble(int block, struct record_location* location);
int readRecord(struct record_location* location, struct t2fs_record* actual_record);
int createNewRegistersBlock(struct t2fs_inode* inode, struct record_location* location, struct t2fs_record* parent_record);
int formatPointerBlock(int block);
int formatDirINode(int inode_number);
int eraseRecord(struct record_location* location);
int freeBlocks(int inode_number);
int freeListBlock(int block);
int freeDoubleListBlock(int block);
int testEmpty(int inode_number);
int testEmptyBlock(int block);
int testEmptyList(int block);


#endif
