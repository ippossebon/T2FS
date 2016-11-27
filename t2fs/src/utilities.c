/* 	Arquivo de Código da Biblioteca t2fs.c
Implementado por Camila Haas Primieri e Isadora Pedrini Possebon
Sistemas Operacionais I - N
Universidade Federal do Rio Grande do Sul - UFRGS */

#include <stdlib.h>
#include <stdio.h>
#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/t2fs.h"
#include "../include/utilities.h"

#define MAX_OPENED_FILES 20
#define SECTOR_SIZE 256
#define ERRO -1
#define SUCESSO 0


/* Retorna 0 se conseguiu ler; -1, caso contrário. */
int readSuperBlock(struct t2fs_superbloco *superblock){
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char word_buffer[2];
    unsigned char dword_buffer[4];
    int i;

    if (read_sector(0, &buffer_sector[0]) != 0){
        printf("Erro ao ler setor\n");
        return ERRO;
    }

    for(i = 0; i < 4; i++){
        superblock->id[i] = buffer_sector[i];
    }

    for(i = 4; i < 6; i++){
        word_buffer[i - 4] = buffer_sector[i];
    }
    superblock->version = *(WORD *)word_buffer;

    for(i = 6; i < 8; i++){
        word_buffer[i - 6] = buffer_sector[i];
    }
    superblock->superblockSize = *(WORD *)word_buffer;

    for(i = 8; i < 10; i++){
        word_buffer[i - 8] = buffer_sector[i];
    }
    superblock->freeBlocksBitmapSize = *(WORD *)word_buffer;

    for(i = 10; i < 12; i++){
        word_buffer[i - 10] = buffer_sector[i];
    }
    superblock->freeInodeBitmapSize = *(WORD *)word_buffer;

    for(i = 12; i < 14; i++){
        word_buffer[i - 12] = buffer_sector[i];
    }
    superblock->inodeAreaSize = *(WORD *)word_buffer;

    for(i = 14; i < 16; i++){
        word_buffer[i - 14] = buffer_sector[i];
    }
    superblock->blockSize = *(WORD *)word_buffer;

    for(i = 16; i < 20; i++){
        dword_buffer[i - 16] = buffer_sector[i];
    }
    superblock->diskSize = *(DWORD *)dword_buffer;

    // printf("id = %c%c%c%c\n", superblock->id[0], superblock->id[1], superblock->id[2], superblock->id[3]);
    // printf("version = %d\n", superblock->version);
    // printf("superblockSize = %d\n", superblock->superblockSize);
    // printf("blocks_bitmap_size = %d\n", superblock->freeBlocksBitmapSize);
    // printf("inodes_bitmap_size = %d\n", superblock->freeInodeBitmapSize);
    // printf("inodes_area_size = %d\n", superblock->inodeAreaSize);
    // printf("sectors_per_block = %d\n", superblock->blockSize);
    // printf("total_sectors_count = %d\n", superblock->diskSize);

    return 0;
}


/* Cria um registro com os parâmetros especificados. Retorna um ponteiro para este registro,
em caso de sucesso. Caso contrário, retorna NULL. */
struct t2fs_record* createRecord(BYTE type, char* name, DWORD file_size_in_blocks, DWORD file_size_in_bytes, int inode_number){

    struct t2fs_record* record;
    record = malloc(sizeof(struct t2fs_record));

    if (strlen(name) > 32){
        printf("[createRecord] tamanho do arquivo não pode ser maior do que 32 bytes. Tamanho enviado: %d\n", strlen(name));
        return NULL;
    }

    record->TypeVal = type;
    strcpy(record->name, name);
    record->blocksFileSize = file_size_in_blocks;
    record->bytesFileSize = file_size_in_bytes;
    record->inodeNumber = inode_number;

    return record;
}

/* Função utilizada tanto para arquivos regulares quanto diretórios.
Retorna 0 se arquivo não existe; 1, caso contrário.*/
int existsFile(char* filename){

    return 0;
}

/*
Na criação de
um arquivo, todos os diretórios intermediários da raiz até ao diretório corrente já devem existir. Se não existirem, a
primitiva de criação deverá retornar com erro. Por exemplo, ao criar o arqx com o caminho /a/b/c/d/arqx todos os
diretórios do caminho já devem existir (a, b, c e d).
*/
int isValidPath(char* path){


    return 0;
}

/* Retorna 1 se o nome do arquivo é válido. 0, caso contrário.
Nomes de arquivos só podem conter letras, números e o caractere '.'. */
int isFileNameValid(char* filename){

    for(int i = 0; i < strlen(filename); i++){
        if (!(filename[i] == 46
        || 46 < filename[i]  <= 57
        || 65 <= filename[i] <= 90
        || 97 <==filename[i] <= 122)){
            return 0;
        }
    }
    return 1;
}

/* Aloca um novo i-node. Em caso de sucesso, retorna o índice do i-node alocado; -1, em caso de erro.*/
int createINode(){
    /* Procura por um i-node livre. Parâmetro 0 indica que vai procurar por i-nodes.
    Valor 1 indica que procura por um i-node LIVRE. */
    int	inode_index;
    inode_index = searchBitmap2 (0, 1);

    if (inode_index <= 0){
        printf("[createINode] Não foi encontrado nenhum i-node disponível.\n");
        return -1;
    }

    // Aloca-se o i-node em questão
    if(setBitmap2(0, inode_index, 0) != 0){
        printf("[createINode] Erro ao setar i-node %d\n", inode_index);
        return -1;
    }

    // Inicialia entradas do i-node
    // Procura pelo i-node em questão em inodeArea (Leitura do disco)
    // Precisamos definir qual é a área em que estão localizados os bitmaps.
    unsigned char* buffer;


    if (read_sector() != 0){
        printf("[createINode] Erro ao ler setor\n");
        return -1;
    }


    return inode_index;
}
