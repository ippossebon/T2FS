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

WORD sectors_per_block;
WORD inodes_start_sector;
WORD blocks_start_sector;
WORD inodes_area_size;

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

int writeRecord(struct t2fs_record* record, int inode){
    /* Inicia escrita em blocks_start_sector + blocksFileSize*/
    if (inode == 0){
        /* Diretório raiz: Aloca um novo bloco, utilizando o i-node 0.
        Escreve o bloco com o conteúdo "0". */

        char* data[2];
        /* Como converter os dados de record para data?? */

        /* Escreve o registro no primeiro bloco de dados do disco, sabendo que um
        registro ocupa 4 setores. */
        return writeBlock(blocks_start_sector, &data, 4);
    }
    else{
        // Escreve registro no disco no diretório pai

        /* NOT IMPLEMENTED */
        return ERRO;
    }
}

/* Escreve i-node no disco. */
int writeINode(t2fs_inode* inode){

    /* Como converter os campos do i-node para char que vai pro disco ?*/

    return ERRO;
}

/* Escreve um bloco na memória. Retorna SUCESSO ou ERRO.
Consideramos que vamos escrever pelo menos um setor inteiro.*/
int writeBlock(int block, char* data, int numb_sectors_to_write)
{
    if (block < 0){
        return ERRO;


    /* Exemplo: escrita no bloco 2. start_sector = 2 * 16 = 32
    Começa a escrever no setor 32. */
    int start_sector = block * sectors_per_block;

    int i;
    for (i = 0; i < numb_sectors_to_write; i++)
    {
        if(write_sector(start_sector + i, &data[i]) != 0){
            return ERRO;
         }
    }
    return SUCESSO;
}

/* Procura por um bloco livre no disco, aloca-o e escrve de volta no disco.
 Retorna o número do bloco alocado, ou, -1 em caso de erro. */
int allocNewBlock(){
    unsigned char block[sectors_per_block * SECTOR_SIZE];
    int block_number;

    /* 0 indica que procuramos por um bloco livre. */
    block_number = searchBitmap2(BITMAP_DADOS, 0);

    if (block_number < 0){
        printf("[allocNewBlock] Não há blocos de dados disponíveis.\n");
        return ERRO;
    }

    /* Marca bloco como ocupado. */
    int aux;
    aux = setBitmap2(BITMAP_DADOS, block_number, 1);
    if (aux != 0){
        printf("[allocNewBlock] Erro ao marcar bloco como ocupado.\n");
        return ERRO;
    }

    /* Escreve bloco no disco. */
    aux = writeBlock(block_number, data);
    if (aux == ERRO){
        return ERRO;
    }

    return block_number;
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
    int i;

    for(i = 0; i < strlen(filename); i++){
        if (!(filename[i] == 46
        || 46 < filename[i]  || filename[i] <= 57
        || 65 <= filename[i] || filename[i] <= 90
        || 97 <=filename[i] || filename[i] <= 122)){
            return 0;
        }
    }
    return 1;
}

/* Retorna o índice do i-node livre que foi encontrado. Se não encontrou, retorna -1.*/
int findFreeINode(){
    /* 1: i-node ocupado
        0: i-node livre */
    int	inode_number;
    inode_number = searchBitmap2(BITMAP_INODE, 0);

        if (inode_number < 0){
            printf("[searchFreeINode] Não foi encontrado nenhum i-node disponível. inode_number: %d \n", inode_number);
            return ERRO;
        }
        else{
            printf("[searchFreeINode] i-node livre encontrado: %d\n", inode_number);
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
    if((inode_number < 0)||(inode_number >= total_inodes)){
        printf("fora dos limites. inode = %d, total_inodes= %d\n", inode_number, total_inodes);
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

    printf("ok\n");
    /* Seleciona os bytes do inode desejado e coloca as informações no ponteiro do inode */
    for(i = inode_position; i < inode_position + 4; i++){
        pointer_buffer[i - inode_position] = buffer_sector[i];
    }
    actual_inode->dataPtr[0] = *(int *)pointer_buffer;
    printf("ok1\n");

    for(i = inode_position + 4; i < inode_position + 8; i++){
        pointer_buffer[i - inode_position - 4] = buffer_sector[i];
    }
    actual_inode->dataPtr[1] = *(int *)pointer_buffer;
    printf("ok2\n");

    for(i = inode_position + 8; i < inode_position + 12; i++){
        pointer_buffer[i - inode_position - 8] = buffer_sector[i];
    }
    actual_inode->singleIndPtr = *(int *)pointer_buffer;
    printf("ok3\n");

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

/* Cria um registro com os parâmetros especificados. Retorna um ponteiro para este registro,
em caso de sucesso. Caso contrário, retorna NULL. */
struct t2fs_record* createRecord(BYTE type, char* name, DWORD file_size_in_blocks, DWORD file_size_in_bytes, int inode_number){

    struct t2fs_record* record;
    record = malloc(sizeof(struct t2fs_record));

    if (strlen(name) > 32){
        printf("[createRecord] nome arquivo não pode ser maior do que 32 bytes. Tamanho enviado: %d\n", strlen(name));
        return NULL;
    }

    record->TypeVal = type;
    strcpy(record->name, name);
    record->blocksFileSize = file_size_in_blocks;
    record->bytesFileSize = file_size_in_bytes;
    record->inodeNumber = inode_number;

    return writeRecord(record, inode_number);
}


/* Aloca um novo i-node. Em caso de sucesso, retorna o índice do i-node alocado; -1, em caso de erro.*/
int createINode(int inode_start_position, int inode_sectors){
    /* Procura por um i-node livre. Parâmetro 0 indica que vai procurar por i-nodes.
    Valor 0 indica que procura por um i-node LIVRE. */
    int	inode_number;
    inode_number = searchBitmap2(BITMAP_INODE, 0);

    if (inode_number < 0){
        printf("[createINode] Não foi encontrado nenhum i-node disponível. inode_number: %d \n", inode_number);
        return ERRO;
    }

    // Aloca-se o i-node em questão
    if(setBitmap2(0, inode_number, 1) != 0){
        printf("[createINode] Erro ao setar i-node %d\n", inode_number);
        return ERRO;
    }

    struct t2fs_inode* inode = malloc(sizeof(struct t2fs_inode));

    /* ESCREVE I-NODE NO DISCO.
        Como separar os campos e escrevÊ-los no disco?
    */

    return inode_number;
}
