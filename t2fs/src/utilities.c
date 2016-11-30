/* 	Arquivo de Código da Biblioteca t2fs.c
Implementado por Camila Haas Primieri e Isadora Pedrini Possebon
Sistemas Operacionais I - N
Universidade Federal do Rio Grande do Sul - UFRGS */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/t2fs.h"
#include "../include/utilities.h"

#define MAX_OPENED_FILES 20
#define SECTOR_SIZE 256
#define ERRO -1
#define SUCESSO 0

extern WORD sectors_per_block;
extern WORD inodes_start_sector;
extern WORD blocks_start_sector;
extern WORD inodes_area_size;

/* Retorna 0 se conseguiu ler; -1, caso contrário. */
int readSuperBlock(struct t2fs_superbloco *superblock, int *inode_start_position, int *inode_sectors, int *block_to_sectors){
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

    printf("id = %c%c%c%c\n", superblock->id[0], superblock->id[1], superblock->id[2], superblock->id[3]);
    printf("version = %d\n", superblock->version);
    printf("superblockSize = %d\n", superblock->superblockSize);
    printf("blocks_bitmap_size = %d\n", superblock->freeBlocksBitmapSize);
    printf("inodes_bitmap_size = %d\n", superblock->freeInodeBitmapSize);
    printf("inodes_area_size = %d\n", superblock->inodeAreaSize);
    printf("sectors_per_block = %d\n", superblock->blockSize);
    printf("total_sectors_count = %d\n", superblock->diskSize);

    sectors_per_block = superblock->blockSize;
    inodes_area_size = superblock->inodeAreaSize;

    inodes_start_sector = (superblock->superblockSize + superblock->freeBlocksBitmapSize + superblock->freeInodeBitmapSize) - 1;
    blocks_start_sector = (inodes_start_sector + inodes_area_size) - 1;

    return 0;
}

/* Retorna 1 se o nome do arquivo é válido. 0, caso contrário.
Nomes de arquivos só podem conter letras, números e o caractere '.'. */
int isFileNameValid(char* filename){
    int i;

    for(i = 0; i < strlen(filename); i++){
        if (filename[i] == 46){
        }
        else if(filename[i] >= 48 && filename[i]<= 57){
        }
        else if(filename[i] >= 65 && filename[i] <= 90){
        }
        else if(filename[i] >= 97 && filename[i] <= 122){
        }
        else{
            return ERRO;
        }
    }
    return SUCESSO;
}

/* Retorna o índice do i-node livre que foi encontrado. Se não encontrou, retorna -1.*/
int findFreeINode(){

    int	inode_number;
    inode_number = searchBitmap2(BITMAP_INODE, LIVRE);

        if (inode_number < 0){
            printf("[searchFreeINode] Não foi encontrado nenhum i-node disponível.\n");
            return ERRO;
        }
        else{
            //printf("[searchFreeINode] i-node livre encontrado: %d\n", inode_number);
            return inode_number;
        }
}

/* Função para ler um inode especificado. Argumentos:
    - um ponteiro para a estrutura inode onde será colocado o inode lido
    - o número do inode a ser lido
    - a posição inicial dos inodes em disco
    - e quantidade de setores ocupados por inodes
Retorna 0 em caso de sucesso e -1 em caso de erro. */
int readInode(struct t2fs_inode *actual_inode, int inode_number, int inode_start_position, int inode_sectors){
    int sector = 0, inode_position = 0, total_inodes, i;
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char pointer_buffer[4];

    total_inodes = inode_sectors * INODE_SIZE;

    /* Teste se o número informado é de um inode válido */
    if((inode_number <= 0)||(inode_number >= total_inodes)){
        return ERRO;
    }
    /* Localiza o setor correto para a leitura */
    sector = inode_start_position;
    sector = sector + inode_number/INODE_SIZE;

    if (read_sector(sector, &buffer_sector[0]) != 0){
        printf("Erro ao ler setor %d\n", sector);
        return ERRO;
    }

    /* Seleciona a posição dos bytes do inode certo dentro do setor lido */
    inode_position = inode_number % INODE_SIZE;
    inode_position = inode_position * 16;

    /* Seleciona os bytes do inode desejado e coloca as informações no ponteiro do inode */
    for(i = inode_position; i < inode_position + 4; i++){
        pointer_buffer[i - inode_position] = buffer_sector[i];
    }
    actual_inode->dataPtr[0] = *(int *)pointer_buffer;

    for(i = inode_position + 4; i < inode_position + 8; i++){
        pointer_buffer[i - inode_position - 4] = buffer_sector[i];
    }
    actual_inode->dataPtr[1] = *(int *)pointer_buffer;

    for(i = inode_position + 8; i < inode_position + 12; i++){
        pointer_buffer[i - inode_position - 8] = buffer_sector[i];
    }
    actual_inode->singleIndPtr = *(int *)pointer_buffer;

    for(i = inode_position + 12; i < inode_position + 16; i++){
        pointer_buffer[i - inode_position - 12] = buffer_sector[i];
    }
    actual_inode->doubleIndPtr = *(int *)pointer_buffer;

    printf("Dados do inode %d\n", inode_number);
    printf("dataPtr[0] = %d\n", actual_inode->dataPtr[0]);
    printf("dataPtr[1] = %d\n", actual_inode->dataPtr[1]);
    printf("singleIndPtr = %d\n", actual_inode->singleIndPtr);
    printf("doubleIndPtr = %d\n", actual_inode->doubleIndPtr);

    return SUCESSO;
}

/* Cria um registro com os parâmetros especificados.
Retorna 0 se executou corretamente; caso contrário, -1. */
int createRecord(BYTE type, char* name, int inode_number){

    /* Um registro de arquivo regular é inicializado com 1 bloco de dados e 0 bytes.*/
    struct t2fs_record* record;
    record = malloc(64);

    if (strlen(name) > 32){
        printf("[createRecord] nome arquivo não pode ser maior do que 32 bytes.\n");
        return ERRO;
    }

    record->TypeVal = type;
    strcpy(record->name, name);
    record->blocksFileSize = 1;
    record->bytesFileSize = 0;
    record->inodeNumber = inode_number;

    int aux = writeRecord(record);

    if(aux == SUCESSO){
        return SUCESSO;
    }
    return ERRO;
}

/*- Escreve registro no disco no diretório pai, retornando SUCESSO ou ERRO.
    Inicia escrita em blocks_start_sector + blocksFileSize*/
int writeRecord(struct t2fs_record* record){

    if (record->inodeNumber <= 0){
        return ERRO;
    }
    else{
        int block_index = searchBitmap2(BITMAP_DADOS, LIVRE);

        if(block_index <= 0){
            printf("Indice fora dos limites: %d\n", block_index);
            return ERRO;
        }

        /* Para cada setor de cada bloco, lê os seus 4 registros e verifica se são válidos.
        Se encontrar algum registro inválido: escreve o novo registro neste inválido e
        grava, no disco, todo este setor. Senão, passa para o próximo setor.
        Se nenhum setor possuir nenhum registro inválido, aloca um novo bloco livre.*/

        unsigned char buffer_sector[SECTOR_SIZE];
        char type_buffer[2];
        unsigned char name_buffer[32];
        unsigned char size_in_blocks_buffer[4];
        unsigned char size_in_bytes_buffer[4];
        unsigned char inode_number_buffer[4];
        int s;


        for (s = 0; s < 16; s++){
            if (read_sector(blocks_start_sector + (s * 256), &buffer_sector[0]) != 0){
                printf("[writeRecord] Erro ao ler setor.\n");
                return ERRO;
            }

            /* .
            Um setor possui 4 registros. Lê cada um dos registros, procurando por um registro
            inválido. Cada registro possui 64 bytes.*/
            int i = 0;
            for (i = 0; i < 4 ; i++){
                /* Lê cada um dos registros do setor. */
                type_buffer[0] = (char) buffer_sector[i * 64];
                printf("type_buffer: %c %c\n", type_buffer[0], type_buffer[1]);
                if (type_buffer == TYPEVAL_INVALIDO){
                    printf("Achou registro invalido\n");
                    // O registro é inválido, então podemos utiliza-lo.
                    memcpy(name_buffer, (char*)&record->name, sizeof(record->name));
                    memcpy(size_in_blocks_buffer, (char*)&record->blocksFileSize, sizeof(int));
                    memcpy(size_in_bytes_buffer, (char*)&record->bytesFileSize, sizeof(int));
                    memcpy(inode_number_buffer, (char*)&record->inodeNumber, sizeof(int));

                    // Copia as informações de cada buffer para o buffer_sector
                    memcpy(&buffer_sector[i * 64], &type_buffer, sizeof(type_buffer));
                    memcpy(&buffer_sector[(i * 64) + 1], &name_buffer, sizeof(name_buffer));
                    memcpy(&buffer_sector[(i * 64) + 32], &size_in_blocks_buffer, sizeof(size_in_blocks_buffer));
                    memcpy(&buffer_sector[(i * 64) + 36], &size_in_bytes_buffer, sizeof(size_in_bytes_buffer));
                    memcpy(&buffer_sector[(i * 64) + 40], &inode_number_buffer, sizeof(inode_number_buffer));

                    // Escreve no disco todo o setor.
                    if (write_sector(blocks_start_sector + (s * 256), &buffer_sector[0]) != 0){
                        printf("[writeRecord] Erro ao escrever setor de volta no disco\n");
                        return ERRO;
                    }
                    int aux = setBitmap2(BITMAP_DADOS, block_index, OCUPADO);

                    if (aux == ERRO){
                        return ERRO;
                    }
                    return SUCESSO;
                }
            }
            /* Se chegou até aqui, é porque não há registros inválidos neste bloco.
            Portanto, deve alocar um novo bloco. */
        }
    }
    return ERRO;
}
