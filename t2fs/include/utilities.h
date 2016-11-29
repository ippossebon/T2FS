#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#define MAX_OPENED_FILES 20
#define SECTOR_SIZE 256
#define INODE_SIZE 16

#define ERRO -1
#define SUCESSO 0

#define TYPEVAL_FILE 0x01
#define TYPEVAL_DIR 0x02
#define TYPEVAL_INVALID 0x00

int readSuperBlock(struct t2fs_superbloco *superblock, int *inode_start_position, int *inode_sectors, int *block_to_sectors);
struct t2fs_record* createRecord(BYTE type, char* name, DWORD file_size_in_blocks, DWORD file_size_in_bytes, int inode_number);
int readInode(struct t2fs_inode *actual_inode, int inode_number, int inode_start_position, int inode_sectors);
int findFreeINode();
int createINode(int inode_start_position, int inode_sectors);

#endif
