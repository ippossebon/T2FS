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

int inodes_area_sectors;
int blocks_start_sector;
int inodes_start_sector;
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

    inodes_start_sector = (int)superblock->superblockSize + (int)superblock->freeBlocksBitmapSize + (int)superblock->freeInodeBitmapSize;
    inodes_area_sectors = (int)superblock->inodeAreaSize;
    blocks_start_sector = inodes_start_sector + inodes_area_sectors;
    sectors_by_block = (int)superblock->blockSize;
    blocks_total = ((int)superblock->diskSize - blocks_start_sector) / sectors_by_block;

    return SUCESSO;
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

/* Função para ler um inode especificado. Argumentos:
    - um ponteiro para a estrutura inode onde será colocado o inode lido
    - o número do inode a ser lidos
Retorna 0 em caso de sucesso e -1 em caso de erro. */
int readInode(struct t2fs_inode *actual_inode, int inode_number){
    int sector = 0, inode_position = 0, total_inodes, i;
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char pointer_buffer[4];

    total_inodes = inodes_area_sectors * INODE_SIZE;

    /* Teste se o número informado é de um inode válido */
    if((inode_number < 0)||(inode_number >= total_inodes)){
        printf("inode_number %d < 0 ou maior que o total_inodes %d\n", inode_number, total_inodes);
        return ERRO;
    }
    if(getBitmap2(BITMAP_INODE, inode_number) != OCUPADO){
        printf("bitmap do i-node está como livre\n");
        setBitmap2 (BITMAP_INODE, inode_number, OCUPADO);
        //return ERRO;
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


/* Recebe o número e os dados de um i-node e grava no disco. */
int writeInode(int inode_number, struct t2fs_inode inode){
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char pointer_buffer[4];
    int inodes_by_sector, sector, position, i;

    inodes_by_sector = SECTOR_SIZE / INODE_SIZE;
    printf("inodes_by_sector = %d\n", inodes_by_sector);
    if((inode_number < 0)||(inode_number >= inodes_area_sectors * inodes_by_sector)){
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

/*- Escreve registro no disco no diretório pai, retornando SUCESSO ou ERRO.
    Inicia escrita em blocks_start_sector + blocksFileSize*/
int writeRecord(struct t2fs_record* record){

    if (record->inodeNumber <= 0){
        return ERRO;
    }
    else{

        /* Para cada setor de cada bloco, lê os seus 4 registros e verifica se são válidos.
        Se encontrar algum registro inválido: escreve o novo registro neste inválido e
        grava, no disco, todo este setor. Senão, passa para o próximo setor.
        Se nenhum setor possuir nenhum registro inválido, aloca um novo bloco livre.*/

        unsigned char buffer_sector[SECTOR_SIZE];
        unsigned char type_buffer;
        unsigned char name_buffer[32];
        unsigned char size_in_blocks_buffer[4];
        unsigned char size_in_bytes_buffer[4];
        unsigned char inode_number_buffer[4];
        int b, s;

        for (b = 0; b < blocks_total; b++){
            for (s = 0; s < 16; s++){
                if (read_sector(blocks_start_sector + s + (b * 16), &buffer_sector[0]) != 0){
                    printf("[writeRecord] Erro ao ler setor.\n");
                    return ERRO;
                }
                /* .
                Um setor possui 4 registros. Lê cada um dos registros, procurando por um registro
                inválido. Cada registro possui 64 bytes.*/

                int i = 0;
                for (i = 0; i < 4 ; i++){
                    /* Lê cada um dos registros do setor. */
                    type_buffer = (unsigned char)buffer_sector[i * 64];

                    if (type_buffer == TYPEVAL_INVALIDO){
                        // O registro é inválido, então podemos utiliza-lo.
                        memcpy(&type_buffer, &record->TypeVal, sizeof(unsigned char));
                        memcpy(name_buffer, (char*)&record->name, sizeof(record->name));
                        memcpy(size_in_blocks_buffer, (char*)&record->blocksFileSize, 4);
                        memcpy(size_in_bytes_buffer, (char*)&record->bytesFileSize, 4);
                        memcpy(inode_number_buffer, (char*)&record->inodeNumber, 4);

                        // Copia as informações de cada buffer para o buffer_sector
                        memcpy(&buffer_sector[i * 64], &type_buffer, sizeof(type_buffer));
                        memcpy(&buffer_sector[(i * 64) + 1], &name_buffer, sizeof(name_buffer));
                        memcpy(&buffer_sector[(i * 64) + 32], &size_in_blocks_buffer, sizeof(size_in_blocks_buffer));
                        memcpy(&buffer_sector[(i * 64) + 36], &size_in_bytes_buffer, sizeof(size_in_bytes_buffer));
                        memcpy(&buffer_sector[(i * 64) + 40], &inode_number_buffer, sizeof(inode_number_buffer));

                        // Escreve no disco todo o setor.
                        if (write_sector(blocks_start_sector + s + (b * 16), &buffer_sector[0]) != 0){
                            printf("[writeRecord] Erro ao escrever setor de volta no disco\n");
                            return ERRO;
                        }

                        int aux = setBitmap2(BITMAP_DADOS, b, OCUPADO);

                        if (aux == ERRO){
                            return ERRO;
                        }

                        printf("Registro escrito no bloco %d\n", b);
                        printf("type: %d\n", (int)type_buffer);
                        printf("name: %s\n", name_buffer);
                        printf("size_in_blocks_buffer: %u\n", size_in_blocks_buffer[0]);
                        printf("size_in_bytes_buffer: %u\n", size_in_bytes_buffer[0]);
                        printf("inode_number: %d\n", inode_number_buffer[0]);
                        return SUCESSO;
                    }
                }
                /* Se chegou até aqui, é porque não há registros inválidos neste bloco.
                Portanto, deve alocar um novo bloco. */
            }
        }
    }
    return ERRO;
}


/* Esta função recebe um nome de um arquivo/diretório com o seu caminho absoluto.
    Retorna o número de inode do diretório pai do arquivo/diretório
    ou -1 em caso do caminho ou arquivo/diretório não existir */
int findParentDir(char *name){
    /* Este é o número do inode retornado, começando sempre na raiz */
    int inode_number = 0;

    /* Controle se estamos lendo arquivo ou diretório */
    int dir = 0;

    /* Confere se o caminho absoluto começa na raiz */
    if(name[0] != '/'){
        return ERRO;
    }

    char *token = strtok(name, "/");
    while(token) {
        if(dir == 0){
            printf("Procurando o arquivo/diretório = %s no inode_number = %d\n", token, inode_number);
            inode_number = findInDir(inode_number, token, &dir);
            if(inode_number == ERRO)
                return ERRO;

            token = strtok(NULL, "/");
        }
        else{
            printf("Caminho informado não é válido.\n");
            return ERRO;
        }
    }
    return inode_number;
}

/* Função para localizar um arquivo ou subdiretório em um diretório pai. Deve-se informar:
    - i-node do diretório pai
    - nome do arquvo ou subdiretório, sem o caminho absoluto
    Retorna o número do inode do arquivo ou -1 em caso de erro */
int findInDir(int inode_number, char *name, int *dir){
    struct t2fs_inode inode;
    int aux, file_inode;

    /* Lê o i-node do diretório-pai */
    aux = readInode(&inode, inode_number);
    if(aux != 0){
        printf("Inode de diretório inválido\n");
        return ERRO;
    }

    printf("inode->dataPtr[0] = %d\n", inode.dataPtr[0]);
    printf("inode->dataPtr[1] = %d\n", inode.dataPtr[1]);
    printf("inode->singleIndPtr = %d\n", inode.singleIndPtr);
    printf("inode->doubleIndPtr = %d\n", inode.doubleIndPtr);

    /* Tenta localizar o arquivos nos blocos apontados por ponteiros diretos */
    if(inode.dataPtr[0] !=	INVALID_PTR){
        file_inode = findInBlock(inode.dataPtr[0], name, dir);
        if(file_inode != ERRO){
            return file_inode;
        }
    }
    if(inode.dataPtr[1] !=	INVALID_PTR){
        file_inode = findInBlock(inode.dataPtr[1], name, dir);
        if(file_inode != ERRO){
            return file_inode;
        }
    }
    /* Tenta localizar o arquivos nos blocos apontados por indireção simples e dupla */
    // if(inode.singleIndPtr != INVALID_PTR){
    //     file_inode = findInBlock(inode.singleIndPtr, name);
    //     if(file_inode != ERRO){
    //         return file_inode;
    //     }
    // }
    // if(inode.doubleIndPtr != INVALID_PTR){
    //     file_inode = findInBlock(inode.doubleIndPtr, name);
    //     if(file_inode != ERRO){
    //         return file_inode;
    //     }
    // }
    return ERRO;
}

/* Funçao auxiliar que recebe um endereço para um bloco de dados de um diretório e um nome do arquivo.
    Irá procurar um arquivo com o mesmo nome. Encontrando, devolve o número do inode do arquivo, senão retorna -1 */
int findInBlock(int block, char *name, int *dir){
    int sector, i, j, k, inode_number;
    unsigned char buffer_sector[SECTOR_SIZE];
    char buffer_name[32];
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
            if(((int)buffer_sector[j*DIR_SIZE] == TYPEVAL_REGULAR)||((int)buffer_sector[j*DIR_SIZE] == TYPEVAL_DIRETORIO)){
                if((int)buffer_sector[j*DIR_SIZE] == TYPEVAL_REGULAR){
                    *dir = 1;
                }
                else{
                    *dir = 0;
                }
                for(k = 1 + j*DIR_SIZE; k < 32 + j*DIR_SIZE; k++){
                    buffer_name[k - 1 - j*DIR_SIZE] = buffer_sector[k];
                }
                for(k = 40 + j*DIR_SIZE; k < 44 + j*DIR_SIZE; k++){
                    buffer_inode_number[k - 40 - j*DIR_SIZE] = buffer_sector[k];
                }
                inode_number = *(int *)buffer_inode_number;

                printf("Teste: nome do arquivo = %s\n", buffer_name);
                printf("Teste: inode_number do arquivo = %d\n", inode_number);

                /* Compara os nomes dos arquivos. */
                if(strcmp(name, buffer_name) == 0){
                    return inode_number;
                }
            }
        }
    }
    return ERRO;
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

/* Tenta alocar um novo bloco para um arquivo.
Recebe o tipo de arquivo para o qual o bloco será alocado. No caso de um arquivo
do tipo diretório, o bloco encontrado deve ser formatado.
Em caso de sucesso, retorna o número do bloco;
caso não exista nenhum bloco disponível, retorna 0; em caso de erro, -1. */
int allocNewBlock(BYTE type){
    int block_number = searchBitmap2(BITMAP_DADOS, LIVRE);

    /* Não há blocos livres */
    if (block_number == 0){
        return 0;
    }

    int aux = 0;

    if (type == TYPEVAL_DIRETORIO){
        aux = formatDirBlock(block_number);
    }
    aux += setBitmap2(BITMAP_DADOS, block_number, OCUPADO);

    if (aux != SUCESSO){
        return ERRO;
    }

    return block_number;
}
