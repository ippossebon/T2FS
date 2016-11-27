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

int inode_start_position;
int inode_sectors;
int sectors_by_block;

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

    inode_start_position = (int)superblock->superblockSize + (int)superblock->freeBlocksBitmapSize + (int)superblock->freeInodeBitmapSize;
    inode_sectors = (int)superblock->inodeAreaSize;
    sectors_by_block = (int)superblock->blockSize;

    return SUCESSO;
}

/* Função para ler um inode especificado. Argumentos:
    - um ponteiro para a estrutura inode onde será colocado o inode lido
    - o número do inode a ser lido
    - a posição inicial dos inodes em disco
    - e quantidade de setores ocupados por inodes
Retorna 0 em caso de sucesso e -1 em caso de erro. */
int readInode(struct t2fs_inode *actual_inode, int inode_number){
    int sector = 0, inode_position = 0, total_inodes, i;
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char pointer_buffer[4];

    total_inodes = inode_sectors * INODE_SIZE;

    /* Teste se o número informado é de um inode válido */
    if((inode_number < 0)||(inode_number >= total_inodes)){
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

    return SUCESSO;
}

/* Função para localizar um arquivo ou subdiretório em um diretório pai. Deve-se informar:
    - identificador do diretório pai
    - nome do arquvo ou subdiretório, sem
*/
int findInDir(int dir_handler, char *filename){
    struct t2fs_inode inode;
    int aux, file_handler;

    aux = readInode(&inode, dir_handler);
    if(aux != 0){
        printf("Handler de diretório informado inválido\n");
        return ERRO;
    }

    if(inode.dataPtr[0] !=	INVALID_PTR){
        file_handler = findInBlock(inode.dataPtr[0], filename);
        if(file_handler != ERRO){
            return file_handler;
        }
    }
    if(inode.dataPtr[1] !=	INVALID_PTR){
        file_handler = findInBlock(inode.dataPtr[1], filename);
        if(file_handler != ERRO){
            return file_handler;
        }
    }
    return ERRO;
}

int findInBlock(int block, char *filename){
    int sector;

    sector = inode_start_position + inode_sectors + block * sectors_by_block;
    sector++;

    return ERRO;
}
