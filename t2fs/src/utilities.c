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

int inodes_start_sector;
int blocks_start_sector;
int inode_sectors;
int sectors_by_block;
int blocks_total;

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

    inodes_start_sector = (int)superblock->superblockSize + (int)superblock->freeBlocksBitmapSize + (int)superblock->freeInodeBitmapSize;
    inode_sectors = (int)superblock->inodeAreaSize;
    blocks_start_sector = inodes_start_sector + inode_sectors;
    sectors_by_block = (int)superblock->blockSize;
    blocks_total = ((int)superblock->diskSize - blocks_start_sector) / sectors_by_block;

    return SUCESSO;
}

/* Função para ler um inode especificado. Argumentos:
    - um ponteiro para a estrutura inode onde será colocado o inode lido
    - o número do inode a ser lidos
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
    sector = inodes_start_sector;
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

    // printf("actual_inode->dataPtr[0] = %d\n", actual_inode->dataPtr[0]);
    // printf("actual_inode->dataPtr[1] = %d\n", actual_inode->dataPtr[1]);
    // printf("actual_inode->singleIndPtr = %d\n", actual_inode->singleIndPtr);
    // printf("actual_inode->doubleIndPtr = %d\n", actual_inode->doubleIndPtr);

    return SUCESSO;
}

/* Recebe o número e os dados de um i-node e grava no disco. */
int writeInode(int inode_number, struct t2fs_inode inode){
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char pointer_buffer[4];
    int inodes_by_sector, sector, position, i;

    inodes_by_sector = SECTOR_SIZE / INODE_SIZE;
    printf("inodes_by_sector = %d\n", inodes_by_sector);
    if((inode_number < 0)||(inode_number >= inode_sectors * inodes_by_sector)){
        printf("Número de i-node inválido\n");
        return ERRO;
    }

    sector = inode_number / inodes_by_sector + blocks_start_sector;
    printf("sector = %d\n", sector);
    if (read_sector(sector, &buffer_sector[0]) != 0){
        printf("Erro ao ler setor %d\n", sector);
        return ERRO;
    }

    position = (inode_number % inodes_by_sector) * INODE_SIZE;
    printf("position = %d\n", position);
    memcpy(pointer_buffer, (char*)&inode.dataPtr[0], sizeof(int));
    for(i = position; i < position + 4; i++){
            buffer_sector[i] = pointer_buffer[i - position];
    }

    position = position + 4;
    memcpy(pointer_buffer, (char*)&inode.dataPtr[1], sizeof(int));
    for(i = position; i < position + 4; i++){
            buffer_sector[i] = pointer_buffer[i - position];
    }

    position = position + 4;
    memcpy(pointer_buffer, (char*)&inode.singleIndPtr, sizeof(int));
    for(i = position; i < position + 4; i++){
            buffer_sector[i] = pointer_buffer[i - position];
    }

    position = position + 4;
    memcpy(pointer_buffer, (char*)&inode.doubleIndPtr, sizeof(int));
    for(i = position; i < position + 4; i++){
            buffer_sector[i] = pointer_buffer[i - position];
    }

    if (write_sector(sector, &buffer_sector[0]) != 0){
        printf("Erro ao gravar setor %d\n", sector);
        return ERRO;
    }
    return SUCESSO;
}

/* Recebe um número de um bloco que irá ser formatado para conter registros de um diretório
    Todas as entradas TypeVal passam a ter informações inválidas - 0x00*/
int formatDirBlock(int block){
    int sector, i, j, position = 0;
    unsigned char buffer_sector[SECTOR_SIZE];

    if((block < 0)||(block >= blocks_total)){
        printf("Bloco informado inválido.\n");
        return ERRO;
    }

    /* Marca os bytes do TypeVal como TYPEVAL_INVALIDO no buffer */
    for(j = 0; j < SECTOR_SIZE / DIR_SIZE; j++){
        buffer_sector[position] = TYPEVAL_INVALIDO;
        position += DIR_SIZE;
    }

    // for(j = 0; j < SECTOR_SIZE; j++){
    //     printf("buffer_sector[%d] = %d\n", j, (int)buffer_sector[j]);
    // }

    /* Grava nos setores do bloco o buffer com TypeVal Inválido */
    sector = blocks_start_sector + block * sectors_by_block;
    for(i = 0; i < sectors_by_block; i++){
        if (write_sector(sector, &buffer_sector[0]) != 0){
            printf("Erro ao gravar setor %d\n", sector);
            return ERRO;
        }
        //printf("Gravado setor #%d\n", sector);
        sector++;
    }
    return SUCESSO;
}

/* Função para localizar um arquivo ou subdiretório em um diretório pai. Deve-se informar:
    - identificador do diretório pai
    - nome do arquvo ou subdiretório, sem o caminho absoluto
    Retorna o número do inode do arquivo ou -1 em caso de erro */
int findInDir(int dir_handler, char filename[31]){
    struct t2fs_inode inode;
    int aux, file_handler;

    /* Lê o i-node do diretório-pai */
    aux = readInode(&inode, dir_handler);
    if(aux != 0){
        printf("Handler de diretório inválido\n");
        return ERRO;
    }

    /* Tenta localizar o arquivos nos blocos apontados por ponteiros diretos */
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
    /* Tenta localizar o arquivos nos blocos apontados por indireção simples e dupla */
    // if(inode.singleIndPtr != INVALID_PTR){
    //     file_handler = findInBlock(inode.singleIndPtr, filename);
    //     if(file_handler != ERRO){
    //         return file_handler;
    //     }
    // }
    // if(inode.doubleIndPtr != INVALID_PTR){
    //     file_handler = findInBlock(inode.doubleIndPtr, filename);
    //     if(file_handler != ERRO){
    //         return file_handler;
    //     }
    // }
    return ERRO;
}

/* Funçao auxiliar que recebe um endereço para um bloco de dados de um diretório e um nome do arquivo.
    Irá procurar um arquivo com o mesmo nome. Encontrando, devolve o número do inode do arquivo, senão retorna -1 */
int findInBlock(int block, char filename[31]){
    int sector, i, j, k, handler;
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char buffer_name[31];
    unsigned char buffer_inode_number[4];

    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("Erro ao ler setor %d\n", sector);
            return ERRO;
        }

        /* Cada setor possui quatro entrada para registros de arquivos e subdiretórios */
        for(j=0; j < 4; j++){

            /* Primeiro testa se o registro é válido */
            if(((int)buffer_sector[j*DIR_SIZE] == 1)||((int)buffer_sector[j*DIR_SIZE] == 2)){
                for(k = 1 + j*DIR_SIZE; k < 32 + j*DIR_SIZE; k++){
                    buffer_name[k - 1 - j*DIR_SIZE] = buffer_sector[k];
                }
                for(k = 40 + j*DIR_SIZE; k < 44 + j*DIR_SIZE; k++){
                    buffer_inode_number[k - 40 - j*DIR_SIZE] = buffer_sector[k];
                }
                handler = *(int *)buffer_inode_number;

                printf("Teste: nome do arquivo = %s\n", buffer_name);
                printf("Teste: handler do arquivo = %d\n", handler);

                /* Compara os nomes dos arquivos. */
                for(k=0; k<31; k++){
                    if(filename[k] != buffer_name[k]){
                        break;
                    }
                    else if(filename[k] == '\0'){
                        return handler;
                    }
                }
            }
        }
    }
    return ERRO;
}
