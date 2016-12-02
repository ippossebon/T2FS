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

<<<<<<< HEAD
#define	INVALID_PTR	-1

struct file_descriptor{
    struct t2fs_record* record;      /* Registro do arquivo */
    int current_pointer;            /* Contador de posição do arquivo */
    int sector_record;              /* Número do setor em que o arquivo foi escrito */
    int record_index_in_sector;     /* Índice do registro dentro do setor - dado que um setor possui 4 registros*/
};

=======
>>>>>>> d2f3c49d39b0a5693bb275a14613eef33a215e9b
int readSuperBlock(struct t2fs_superbloco *superblock);
int readInode(struct t2fs_inode *actual_inode, int inode_number);
int findFreeINode();
int writeInode(int inode_number, struct t2fs_inode inode);
int writeRecord(struct t2fs_record* record);
int createRecord(BYTE type, char* name, int inode_number);
int isFileNameValid(char* filename);
int formatDirBlock(int block);
int findRecord(char* name, struct record_location* location);
int findInBlock(int block, char* name, int* dir, struct record_location* location);
int findInDir(int inode_number, char* name, int* dir, struct record_location* location);
int formatDirBlock(int block);

#endif
