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
        printf("[readSuperBlock] Erro ao ler setor\n");
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
        if (filename[i] >= 46 && filename[i]<= 57){
        }
        else if(filename[i] >= 65 && filename[i] <= 90){
        }
        else if(filename[i] >= 97 && filename[i] <= 122){
        }
        else{
            printf("[isFileNameValid] Erro: filename[i] = %c\n", filename[i]);
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
        printf("[readInode] inode_number %d < 0 ou maior que o total_inodes %d\n", inode_number, total_inodes);
        return ERRO;
    }

    /* Localiza o setor correto para a leitura */
    sector = inodes_start_sector;
    sector = sector + inode_number/INODE_SIZE;

    if (read_sector(sector, &buffer_sector[0]) != 0){
        printf("[readInode] Erro ao ler setor %d\n", sector);
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
        printf("[findFreeINode] Não foi encontrado nenhum i-node disponível.\n");
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
    //printf("[writeInode] inodes_by_sector = %d\n", inodes_by_sector);
    if((inode_number < 0)||(inode_number >= inodes_area_sectors * inodes_by_sector)){
        printf("[writeInode] Número de i-node inválido\n");
        return ERRO;
    }

    sector = inode_number / inodes_by_sector + inodes_start_sector;
    //printf("[writeInode] sector = %d\n", sector);
    if (read_sector(sector, &buffer_sector[0]) != 0){
        printf("[writeInode] Erro ao ler setor %d\n", sector);
        return ERRO;
    }

    position = (inode_number % inodes_by_sector) * INODE_SIZE;
    //printf("[writeInode] position = %d\n", position);
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
        printf("[writeInode] Erro ao gravar setor %d\n", sector);
        return ERRO;
    }
    return SUCESSO;
}

/* Recebe o registro que deve ser escrito e o registro do diretório pai (onde deve ser escrito).
Recupera o i-node do diretório pai e escreve o registro no registro livre que encontrar.
Retorna SUCESSO ou ERRO. */
int writeRecord(struct t2fs_record* record_to_write, struct t2fs_record* parent_record, struct record_location* location){

    if (record_to_write->inodeNumber <= 0){
        printf("[writeRecord] Número de i-node informado inválido.\n");
        return ERRO;
    }

    /* Recupera o i-node do diretório pai. */
    struct t2fs_inode parent_inode;
    int aux = readInode(&parent_inode, parent_record->inodeNumber);
    if (aux == ERRO){
        printf("[writeRecord] Erro ao ler i-node do pai\n");
        return ERRO;
    }
    // printf("[writeRecord] parent_inode->dataPtr[0] = %d\n", parent_inode.dataPtr[0]);
    // printf("[writeRecord] parent_inode->dataPtr[1] = %d\n", parent_inode.dataPtr[1]);
    // printf("[writeRecord] parent_inode->singleIndPtr = %d\n", parent_inode.singleIndPtr);
    // printf("[writeRecord] parent_inode->doubleIndPtr = %d\n", parent_inode.doubleIndPtr);

    /* No i-node do diretório pai, procura por um registro inicializado como
    inválido, que é onde este novo registro será escrito. Se nenhum registro
    inválido for encontrado, os campos de location terão o valor -1. */
    aux = findInvalidRecordInINode(&parent_inode, location, parent_record);
    if (aux == ERRO){
        printf("[writeRecord] Erro ao procurar por registro inválido no i-node do diretório pai\n");
        return ERRO;
    }

    /* Não há registros inválidos neste i-node. */
    if (location->sector == -1 && location->position == -1){
        return SUCESSO;
    }

    /* Encontrou registro inválido: escreve novo registro neste registro inválido.
    A posição do registro inválido está em location. */
    unsigned char buffer_sector[SECTOR_SIZE];

    if (read_sector(location->sector, &buffer_sector[0]) != 0){
        printf("[writeRecord] Erro ao ler setor do registro inválido no diretório pai.\n");
        return ERRO;
    }

    /* buffer_sector contém todo o setor que possui um registro inválido.
    Acessa, dentro do setor, o registro inválido. */

    unsigned char type_buffer;
    unsigned char name_buffer[32];
    unsigned char size_in_blocks_buffer[4];
    unsigned char size_in_bytes_buffer[4];
    unsigned char inode_number_buffer[4];

    /* Se o registro acessado for realmente inválido. Escrevemos o novo registro.*/
    int position_in_sector = location->position;

    if ((unsigned char)buffer_sector[position_in_sector * 64] == TYPEVAL_INVALIDO){
        memcpy(&type_buffer, &record_to_write->TypeVal, sizeof(unsigned char));
        memcpy(name_buffer, (char*)&record_to_write->name, sizeof(record_to_write->name));
        memcpy(size_in_blocks_buffer, (char*)&record_to_write->blocksFileSize, 4);
        memcpy(size_in_bytes_buffer, (char*)&record_to_write->bytesFileSize, 4);
        memcpy(inode_number_buffer, (char*)&record_to_write->inodeNumber, 4);

        /* Copia as informações de cada buffer intermediário para o buffer_sector */
        memcpy(&buffer_sector[position_in_sector * 64], &type_buffer, sizeof(type_buffer));
        memcpy(&buffer_sector[(position_in_sector * 64) + 1], &name_buffer, sizeof(name_buffer));
        memcpy(&buffer_sector[(position_in_sector * 64) + 33], &size_in_blocks_buffer, sizeof(size_in_blocks_buffer));
        memcpy(&buffer_sector[(position_in_sector * 64) + 37], &size_in_bytes_buffer, sizeof(size_in_bytes_buffer));
        memcpy(&buffer_sector[(position_in_sector * 64) + 41], &inode_number_buffer, sizeof(inode_number_buffer));

        /* Escreve no disco todo o setor */
        if (write_sector(location->sector, &buffer_sector[0]) != 0){
            printf("[writeRecord] Erro ao escrever setor de volta no disco (com o novo registro)\n");
            return ERRO;
        }

        /* Para testes */
        // printf("[writeRecord] Registro escrito no setor %d, posição %d\n", location->sector, location->position);
        // printf("[writeRecord] Tipo do registro: %d\n", (int)type_buffer);
        // printf("[writeRecord] Nome: %s\n", name_buffer);
        // printf("[writeRecord] size_in_blocks_buffer: %u\n", size_in_blocks_buffer[0]);
        // printf("[writeRecord] size_in_bytes_buffer: %u\n", size_in_bytes_buffer[0]);
        // printf("[writeRecord] inode_number: %d\n", inode_number_buffer[0]);

        return SUCESSO;
    }
    return ERRO;
}

/* Esta função recebe um nome de um arquivo/diretório com o seu caminho absoluto.
    Retorna o número de inode do diretório pai do arquivo/diretório
    ou -1 em caso do caminho ou arquivo/diretório não existir */
int findRecord(char *name, struct record_location* location){
    /* Este é o número do inode, começando sempre na raiz */
    int inode_number = 0;

    /* Controle se estamos lendo arquivo ou diretório */
    int dir = 0;

    /* Confere se o caminho absoluto começa na raiz */
    if(name[0] != '/'){
        return ERRO;
    }

    /* Testa se o arquivo está na raiz ou um subdiretório */
    int i, root = 1;
    for(i = 1; i < strlen(name); i++){
        if(name[i] == '/'){
            root = 0;
        }
    }

    char *token = strtok(name, "/");
    while(token) {
        /* Confere se os subdiretórios são de fato diretórios */
        if(dir == 0){
            //printf("Procurando o arquivo/diretório = %s no inode_number = %d\n", token, inode_number);
            inode_number = findInDir(inode_number, token, &dir, location);

            token = strtok(NULL, "/");

            /* O caminho absoluto inforamdo é inválido */
            if((token != 0)&&(inode_number == ERRO)){
                return ERRO;
            }
            /* O caminho absoluto é válido, mas o arquivo informado não existe */
            else if((token == 0)&&(inode_number == ERRO)){
                /* Se o arquivo estiver no root, vamos setar a localização do registro pai como inválido */
                if(root == 1){
                    location->sector = INVALID_PTR;
                    location->position = INVALID_PTR;
                }
                return SUCESSO;
            }
        }
        else{
            printf("[findRecord] Caminho informado não é válido.\n");
            return ERRO;
        }
    }
    /* O caminho absoluto e o arquivo informado são válidos e existentes */
    return 1;
}

/* Função para localizar um arquivo ou subdiretório em um diretório pai. Deve-se informar:
    - i-node do diretório pai
    - nome do arquvo ou subdiretório, sem o caminho absoluto
    Retorna o número do inode do arquivo ou -1 em caso de erro */
int findInDir(int inode_number, char *name, int *dir, struct record_location* location){
    struct t2fs_inode inode;
    int aux, file_inode;

    /* Lê o i-node do diretório-pai */
    aux = readInode(&inode, inode_number);
    if(aux != 0){
        printf("[findInDir] Inode de diretório inválido\n");
        return ERRO;
    }

    // printf("inode->dataPtr[0] = %d\n", inode.dataPtr[0]);
    // printf("inode->dataPtr[1] = %d\n", inode.dataPtr[1]);
    // printf("inode->singleIndPtr = %d\n", inode.singleIndPtr);
    // printf("inode->doubleIndPtr = %d\n", inode.doubleIndPtr);

    /* Tenta localizar o arquivos nos blocos apontados por ponteiros diretos */
    if(inode.dataPtr[0] !=	INVALID_PTR){
        file_inode = findInBlock(inode.dataPtr[0], name, dir, location);
        if(file_inode != ERRO){
            return file_inode;
        }
    }
    if(inode.dataPtr[1] !=	INVALID_PTR){
        file_inode = findInBlock(inode.dataPtr[1], name, dir, location);
        if(file_inode != ERRO){
            return file_inode;
        }
    }
    /* Tenta localizar o arquivos nos blocos apontados por indireção simples e dupla */
    if(inode.singleIndPtr != INVALID_PTR){
        file_inode = findInList(inode.singleIndPtr, name, dir, location);
        if(file_inode != ERRO){
            return file_inode;
        }
    }
    if(inode.doubleIndPtr != INVALID_PTR){
        file_inode = findInListDouble(inode.doubleIndPtr, name, dir, location);
        if(file_inode != ERRO){
            return file_inode;
        }
    }
    return ERRO;
}

/* Funçao auxiliar que recebe um endereço para um bloco de dados de um diretório e um nome do arquivo.
    Irá procurar um arquivo com o mesmo nome. Encontrando, devolve o número do inode do arquivo, senão retorna -1 */
int findInBlock(int block, char *name, int *dir, struct record_location* location){
    int sector, i, j, k, inode_number;
    unsigned char buffer_sector[SECTOR_SIZE];
    char buffer_name[32];
    unsigned char buffer_inode_number[4];

    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[findInBlock] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui quatro entrada para registros de arquivos e subdiretórios */
        for(j=0; j < 4; j++){

            /* Primeiro testa se o registro é válido */
            if(((int)buffer_sector[j*DIR_SIZE] == TYPEVAL_REGULAR)||((int)buffer_sector[j*DIR_SIZE] == TYPEVAL_DIRETORIO)){
                if((int)buffer_sector[j*DIR_SIZE] == TYPEVAL_REGULAR)
                    *dir = 1;

                else
                    *dir = 0;

                for(k = 1 + j*DIR_SIZE; k < 33 + j*DIR_SIZE; k++){
                    buffer_name[k - 1 - j*DIR_SIZE] = buffer_sector[k];
                }
                for(k = 41 + j*DIR_SIZE; k < 45 + j*DIR_SIZE; k++){
                    buffer_inode_number[k - 41 - j*DIR_SIZE] = buffer_sector[k];
                }
                inode_number = *(int *)buffer_inode_number;

                /* Compara os nomes dos arquivos. */
                // printf("[findInBlock] Comparando os nomes: %s e %s.\n", name, buffer_name);
                if(strcmp(name, buffer_name) == 0){
                    location->sector = sector + i;
                    location->position = j;
                    return inode_number;
                }
            }
        }
    }
    return ERRO;
}

int findInList(int block, char* name, int* dir, struct record_location* location){
    int sector, i, j, k, file_inode, pointer;
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char buffer_pointer[4];

    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[findInList] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;
            if(pointer == TYPEVAL_INVALIDO){
                return ERRO;
            }
            else{
                file_inode = findInBlock(pointer, name, dir, location);
                if(file_inode != ERRO){
                    return file_inode;
                }
            }
        }
    }
    return ERRO;
}

int findInListDouble(int block, char* name, int* dir, struct record_location* location){
    int sector, i, j, k, file_inode, pointer;
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char buffer_pointer[4];

    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[findInListDouble] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;
            if(pointer == TYPEVAL_INVALIDO){
                return ERRO;
            }
            else{
                file_inode = findInList(pointer, name, dir, location);
                if(file_inode != ERRO){
                    return file_inode;
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
        printf("[formatDirBlock] Bloco informado inválido.\n");
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
            printf("[formatDirBlock] Erro ao gravar setor %d\n", sector);
            return ERRO;
        }
        //printf("Gravado setor #%d\n", sector);
        sector++;
    }
    return SUCESSO;
}

/* Função que tenta alocar um novo bloco para um arquivo.
Recebe o tipo de arquivo para o qual o bloco será alocado. No caso de um arquivo
do tipo diretório, o bloco encontrado deve ser formatado.
Em caso de sucesso, retorna o número do bloco;
caso não exista nenhum bloco disponível, retorna 0; em caso de erro, -1. */
int allocNewBlock(BYTE type){
    int block_number = searchBitmap2(BITMAP_DADOS, LIVRE);

    /* Não há blocos livres */
    if (block_number == 0)
        return 0;

    int aux = 0;

    if (type == TYPEVAL_DIRETORIO)
        aux = formatDirBlock(block_number);
    aux += setBitmap2(BITMAP_DADOS, block_number, OCUPADO);

    if (aux != SUCESSO)
        return ERRO;

    return block_number;
}

/* Recebe um i-node e uma estrutura location, onde será armazenado o retorno.
Procura pelo primeiro registro não válido deste i-node e retorna sua localização
em location.*/
int findInvalidRecordInINode(struct t2fs_inode* inode, struct record_location* location, struct t2fs_record* parent_record){
    int aux;

    /* Procura por registro inválido nos ponteiros diretos. */
    if (inode->dataPtr[0] != INVALID_PTR){
        //printf("[findInvalidRecordInINode] Procurando em inode->dataPtr[0].\n");
        aux = findInvalidRecordInBlock(inode->dataPtr[0], location);
        if(aux == SUCESSO)
            return SUCESSO;
    }
    if (inode->dataPtr[1] != INVALID_PTR) {
        //printf("[findInvalidRecordInINode] Procurando em inode->dataPtr[1].\n");
        aux = findInvalidRecordInBlock(inode->dataPtr[1], location);
        if(aux == SUCESSO)
            return SUCESSO;
    }

    /* Procura por registro inválido nos blocos do bloco de índices.*/
    if (inode->singleIndPtr != INVALID_PTR){
        //printf("[findInvalidRecordInINode] Procurando em inode->singleIndPtr.\n");
        aux = findInvalidRecordInList(inode->singleIndPtr, location);
        if(aux == SUCESSO)
            return SUCESSO;
    }

    /* Procura por registro inválido nos blocos dos blocos de índices. */
    if (inode->doubleIndPtr != INVALID_PTR){
        //printf("[findInvalidRecordInINode] Procurando em inode->doubleIndPtr.\n");
        aux = findInvalidRecordInListDouble(inode->doubleIndPtr, location);
        if(aux == SUCESSO)
            return SUCESSO;
    }

    /* Se não foi localizado nenhum registro inválido, será necessário criar um novo bloco de registros */
    //printf("[findInvalidRecordInINode] Não existe registros vazios, é necessário criar um novo bloco de registros\n");
    aux = createNewRegistersBlock(inode, location, parent_record);
    if(aux == SUCESSO){
        //printf("[findInvalidRecordInINode] Novo bloco de registros criado com sucesso.\n");
        return SUCESSO;
    }
    return ERRO;
}

/* Funçao auxiliar que recebe um endereço para um bloco de dados e procura por um
registro inválido neste bloco. Encontrando, devolve o número do setor e posição
dentro do setor; senão encontrar, retorna os valores -1 em location. Em caso de
erro, retorna -1. */
int findInvalidRecordInBlock(int block, struct record_location* location){
    int sector, i, j;
    unsigned char buffer_sector[SECTOR_SIZE];

    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[findInvalidRecordInBlock] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui quatro entradas para registros de arquivos e subdiretórios */
        for(j=0; j < 4; j++){

            /* Testa se o registro é inválido */
            if((int)buffer_sector[j*DIR_SIZE] == TYPEVAL_INVALIDO){
                location->sector = sector + i;
                location->position = j;

                //printf("[findInvalidRecordInBlock] Registro inválido localizado no setor = %d e na posição = %d.\n", sector + i, j);
                return SUCESSO;
            }
        }
    }
    /* Não encotrou nenhum registro inválido no bloco */
    location->sector = ERRO;
    location->position = ERRO;

    return ERRO;
}

/* Recebe o endereço do bloco de índices e procura por um registro inválido em todos
os blocos de dados apontados por ele. Se encontrar, sua localização
(setor + posição dentro do setor) estará em location. Se não encontrar, os campos
de location terão o valor -1. Em caso de sucesso, retorna 0; em caso de erro, -1. */
int findInvalidRecordInList(int block, struct record_location* location){
    int sector, i, j, k, pointer, aux;
    unsigned char buffer_sector[SECTOR_SIZE];
    char buffer_pointer[4];


    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[findInvalidRecordInList] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;
            if(pointer == TYPEVAL_INVALIDO){

                /* Aqui devemos acrescentar mais um bloco para ponteiros */
                int new_block = allocNewBlock(TYPEVAL_DIRETORIO);

                strcpy(buffer_pointer, (char*)new_block);
                for(k = 0; k < 4; k++){
                    buffer_sector[k + j*4] = buffer_pointer[k];
                }
                if (write_sector(sector + i, &buffer_sector[0]) != 0){
                    printf("[findInvalidRecordInList] Erro ao gravar setor %d\n", sector);
                    return ERRO;
                }
                aux = findInvalidRecordInBlock(new_block, location);
                if (aux == SUCESSO){
                    return SUCESSO;
                }
            }
            else{
                /* Procura, no bloco de dados apontado, por um registro inválido.*/
                int aux;
                aux = findInvalidRecordInList(pointer, location);
                if (aux == SUCESSO){
                    return SUCESSO;
                }
            }
        }
    }
    return ERRO;
}

/* Recebe o endereço do bloco dos blocos de índices e procura por um registro
inválido em todos os blocos de dados apontados por eles. Se encontrar, sua localização
(setor + posição dentro do setor) estará em location. Se não encontrar, os campos
de location terão o valor -1. Em caso de sucesso, retorna 0; em caso de erro, -1. */
int findInvalidRecordInListDouble(int block, struct record_location* location){
    int sector, i, j, k, pointer, aux;
    unsigned char buffer_sector[SECTOR_SIZE];
    char buffer_pointer[4];

    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[findInvalidRecordInListDouble] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;
            if(pointer == TYPEVAL_INVALIDO){
                /* Aqui devemos acrescentar mais um bloco para ponteiros */
                int new_block = allocNewBlock(TYPEVAL_DIRETORIO);
                aux = formatPointerBlock(new_block);
                if(aux == ERRO)
                    return ERRO;

                strcpy(buffer_pointer, (char*)new_block);
                for(k = 0; k < 4; k++){
                    buffer_sector[k + j*4] = buffer_pointer[k];
                }
                if (write_sector(sector + i, &buffer_sector[0]) != 0){
                    printf("[findInvalidRecordInList] Erro ao gravar setor %d\n", sector);
                    return ERRO;
                }
                aux = findInvalidRecordInList(new_block, location);
                if (aux == SUCESSO){
                    return SUCESSO;
                }
            }
            else{
                /* Procura, no bloco de dados apontado, por um registro inválido.*/
                int aux;
                aux = findInvalidRecordInList(pointer, location);
                if (aux == SUCESSO){
                    return SUCESSO;
                }
            }
        }
    }
    return ERRO;
}

/* Função que cria um novo bloco de registros para escrever um registro.
Recebe um i-node e um ponteiro para a localização do setor.
Retorna -1 em caso de erro e 0 em caso de sucesso */
int createNewRegistersBlock(struct t2fs_inode* inode, struct record_location* location, struct t2fs_record* parent_record){
    int new_block, aux;

    /* Procura por registro inválido nos ponteiros diretos. */
    if (inode->dataPtr[0] == INVALID_PTR){
        new_block = allocNewBlock(TYPEVAL_DIRETORIO);
        inode->dataPtr[0] = new_block;
        aux = writeInode(parent_record->inodeNumber, *inode);
        if(aux == ERRO){
            printf("[createNewRegistersBlock] Erro ao escrever no i-node.\n");
        }
        aux = findInvalidRecordInBlock(new_block, location);
        if(aux == ERRO){
            printf("[createNewRegistersBlock] Erro ao achar registro inválido no novo bloco de diretório.\n");
        }
        return SUCESSO;
    }
    if (inode->dataPtr[1] == INVALID_PTR){
        new_block = allocNewBlock(TYPEVAL_DIRETORIO);
        inode->dataPtr[1] = new_block;
        aux = writeInode(parent_record->inodeNumber, *inode);
        if(aux == ERRO){
            printf("[createNewRegistersBlock] Erro ao escrever no i-node.\n");
        }
        aux = findInvalidRecordInBlock(new_block, location);
        if(aux == ERRO){
            printf("[createNewRegistersBlock] Erro ao achar registro inválido no novo bloco de diretório.\n");
        }
        return SUCESSO;
    }

    /* Procura por registro inválido nos blocos do bloco de índices.*/
    if (inode->singleIndPtr == INVALID_PTR){
        new_block = allocNewBlock(TYPEVAL_DIRETORIO);
        aux = formatPointerBlock(new_block);
        if(aux == ERRO){
            printf("[createNewRegistersBlock] Erro formantando o bloco.\n");
        }
        inode->singleIndPtr = new_block;
        aux = writeInode(parent_record->inodeNumber, *inode);
        if(aux == ERRO){
            printf("[createNewRegistersBlock] Erro ao escrever no i-node.\n");
        }

        aux = findInvalidRecordInList(inode->singleIndPtr, location);
        if(aux == SUCESSO)
            return SUCESSO;

        printf("[createNewRegistersBlock] Erro ao achar registro inválido no novo bloco de diretório.\n");
        return ERRO;
    }

    if (inode->singleIndPtr != INVALID_PTR){
        aux = findInvalidRecordInList(inode->singleIndPtr, location);
        if(aux == SUCESSO)
            return SUCESSO;
    }

    /* Procura por registro inválido nos blocos dos blocos de índices. */
    if (inode->doubleIndPtr == INVALID_PTR){
        new_block = allocNewBlock(TYPEVAL_DIRETORIO);
        aux = formatPointerBlock(new_block);
        if(aux == ERRO){
            printf("[createNewRegistersBlock] Erro formantando o bloco.\n");
        }
        inode->doubleIndPtr = new_block;
        aux = writeInode(parent_record->inodeNumber, *inode);
        if(aux == ERRO){
            printf("[createNewRegistersBlock] Erro ao escrever no i-node.\n");
        }

        aux = findInvalidRecordInListDouble(inode->doubleIndPtr, location);
        if(aux == SUCESSO)
            return SUCESSO;

        printf("[createNewRegistersBlock] Erro ao achar registro inválido no novo bloco de diretório.\n");
        return ERRO;
    }

    if (inode->doubleIndPtr != INVALID_PTR){
        aux = findInvalidRecordInListDouble(inode->doubleIndPtr, location);
        if(aux == SUCESSO)
            return SUCESSO;

        printf("[createNewRegistersBlock] Erro ao achar registro inválido no novo bloco de diretório.\n");
        return ERRO;
    }

    return ERRO;
}

/* Cria um novo bloco para um arquivo aberto. Retorna o número do bloco no disco */
int createNewBlock(struct t2fs_inode* inode, struct t2fs_record* parent_record){
    int new_block, aux;

    if (inode->dataPtr[0] == INVALID_PTR){
        new_block = allocNewBlock(TYPEVAL_REGULAR);
        inode->dataPtr[0] = new_block;

        aux = writeInode(parent_record->inodeNumber, *inode);
        if(aux == ERRO){
            printf("[createNewBlock] Erro ao escrever no i-node.\n");
        }
        return new_block;
    }
    if (inode->dataPtr[1] == INVALID_PTR){
        new_block = allocNewBlock(TYPEVAL_REGULAR);
        inode->dataPtr[1] = new_block;

        aux = writeInode(parent_record->inodeNumber, *inode);
        if(aux == ERRO){
            printf("[createNewBlock] Erro ao escrever no i-node.\n");
        }
        return new_block;
    }
    if (inode->singleIndPtr == INVALID_PTR){
        new_block = allocNewBlock(TYPEVAL_REGULAR);
        printf("[createNewBlock] Novo bloco criado %d\n", new_block);
        aux = formatPointerBlock(new_block);
        if(aux == ERRO){
            printf("[createNewBlock] Erro formantando o bloco.\n");
        }

        inode->singleIndPtr = new_block;
        aux = writeInode(parent_record->inodeNumber, *inode);
        if(aux == ERRO){
            printf("[createNewBlock] Erro ao escrever no i-node.\n");
        }

        aux = createNewBlockInList(new_block);
        if(aux == ERRO){
            printf("[createNewBlock] Erro no createNewBlockInList.\n");
            return ERRO;
        }
        return new_block;
    }

    /* Cria novo bloco em indireção simples */
    if (inode->singleIndPtr != INVALID_PTR){
        int list = inode->singleIndPtr;
        new_block = createNewBlockInList(list);
        if(new_block > 0){
            return new_block;
        }
    }

    if (inode->doubleIndPtr == INVALID_PTR){
        int new_block2, new_block3;

        new_block = allocNewBlock(TYPEVAL_REGULAR);
        aux = formatPointerBlock(new_block);
        if(aux == ERRO){
            printf("[createNewBlock] Erro formantando o bloco.\n");
        }

        inode->doubleIndPtr = new_block;
        aux = writeInode(parent_record->inodeNumber, *inode);
        if(aux == ERRO){
            printf("[createNewBlock] Erro ao escrever no i-node.\n");
        }

        new_block2 = createNewBlockInList(new_block);
        if(new_block2 == ERRO){
            return ERRO;
        }

        aux = formatPointerBlock(new_block2);
        if(aux == ERRO){
            printf("[createNewBlock] Erro formantando o bloco.\n");
        }
        new_block3 = createNewBlockInList(new_block);
        if(new_block3 == ERRO){
            return ERRO;
        }

        return new_block3;
    }

    if (inode->doubleIndPtr != INVALID_PTR){
        int list = inode->doubleIndPtr;
        new_block = createNewBlockInListDouble(list);
        if(new_block > 0){
            return new_block;
        }
    }

    return ERRO;
}

/* Recebe um arquivo que é uma lista de vazia de ponteiros e cria um bloco */
int createNewBlockInList(int block){
    int sector, i, j, k, pointer, new_block;
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char buffer_pointer[4];


    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[createNewBlockInList] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;
            if(pointer == INVALID_PTR){
                new_block = allocNewBlock(TYPEVAL_REGULAR);
                memcpy(&buffer_pointer[0], (char*)&new_block, 4);

                for(k = 0; k < 4; k++){
                    buffer_sector[k + j*4] = buffer_pointer[k];
                }

                if (write_sector(sector + i, &buffer_sector[0]) != 0){
                    printf("[createNewBlockInList] Erro ao gravar setor %d\n", sector);
                    return ERRO;
                }

                return new_block;
            }
        }
    }
    return ERRO;
}

int createNewBlockInListDouble(int block){
    int sector, i, j, k, pointer, new_block, aux, new_block2;
    unsigned char buffer_sector[SECTOR_SIZE];
    char buffer_pointer[4];


    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[createNewBlockInListDouble] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;

            if(pointer != INVALID_PTR){
                new_block = createNewBlockInList(pointer);
                if(new_block != ERRO){
                    return new_block;
                }
            }

            else if(pointer == INVALID_PTR){
                new_block = allocNewBlock(TYPEVAL_REGULAR);
                memcpy(buffer_pointer, (char*)new_block, 4);

                for(k = 0; k < 4; k++){
                    buffer_sector[k + j*4] = buffer_pointer[k];
                }

                if (write_sector(sector + i, &buffer_sector[0]) != 0){
                    printf("[createNewBlockInListDouble] Erro ao gravar setor %d\n", sector);
                    return ERRO;
                }

                aux = formatPointerBlock(new_block);
                if(aux == ERRO){
                    printf("[createNewBlockInListDouble] Erro formantando o bloco.\n");
                }
                new_block2 = createNewBlockInList(new_block);
                if(new_block2 == ERRO){
                    return ERRO;
                }
                return new_block2;
            }
        }
    }
    return ERRO;
}

int formatPointerBlock(int block){
    int sector, i, j, k;
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char buffer_pointer[4];
    int invalid = -1;

    memcpy(&buffer_pointer[0], (char*)&invalid, 4);
    if((block < 0)||(block >= blocks_total)){
        printf("[formatPointerBlock] Bloco informado inválido.\n");
        return ERRO;
    }
    for(j=0; j < SECTOR_SIZE / 4; j++){
        for(k = 0; k < 4; k++){
            buffer_sector[k + j*4] = buffer_pointer[k];
        }
    }
    sector = blocks_start_sector + block * sectors_by_block;
    for(i = 0; i < sectors_by_block; i++){
        if (write_sector(sector, &buffer_sector[0]) != 0){
            printf("[formatPointerBlock] Erro ao gravar setor %d\n", sector);
            return ERRO;
        }
        sector++;
    }
    return SUCESSO;
}

/* Recebe a posição de um registro no disco e retorna o registro na estrutura
actual_record. Retorna 0 se executou corretamente; caso contrário, -1. */
int readRecord(struct record_location* location, struct t2fs_record* actual_record){

    unsigned char buffer_sector[SECTOR_SIZE];

    if (read_sector(location->sector, &buffer_sector[0]) != 0){
        printf("[readRecord] Erro ao ler setor indicado\n");
        return ERRO;
    }

    int position_in_sector = location->position;

    /* Recupera o registro em questão */
    memcpy(&actual_record->TypeVal, &buffer_sector[position_in_sector * 64], sizeof(actual_record->TypeVal));
    memcpy(&actual_record->name, &buffer_sector[(position_in_sector * 64) + 1], sizeof(actual_record->name));
    memcpy(&actual_record->blocksFileSize, &buffer_sector[(position_in_sector * 64) + 33], sizeof(actual_record->blocksFileSize));
    memcpy(&actual_record->bytesFileSize, &buffer_sector[(position_in_sector * 64) + 37], sizeof(actual_record->bytesFileSize));
    memcpy(&actual_record->inodeNumber, &buffer_sector[(position_in_sector * 64) + 41], sizeof(actual_record->inodeNumber));

    /* Teste */
    // printf("[readRecord] Registro recuperado do setor %d, posição %d\n", location->sector, location->position);
    // printf("[readRecord] Type: %d\n", actual_record->TypeVal);
    // printf("[readRecord] Name: %s\n", actual_record->name);
    // printf("[readRecord] blocksFileSize: %d\n", actual_record->blocksFileSize);
    // printf("[readRecord] bytesFileSize: %d\n", actual_record->bytesFileSize);
    // printf("[readRecord] inodeNumber: %d\n", actual_record->inodeNumber);

    return SUCESSO;
}

/* Função que recebe um i-node relativo a um diretório, aloca novos blocos para
este i-node e formata os blocos como diretórios.
Retorna 0 em caso de sucesso; -1, caso contrário. */
int formatDirINode(int inode_number){

    struct t2fs_inode inode;
    int aux = readInode(&inode, inode_number);
    if (aux == ERRO){
        return ERRO;
    }

    int new_block = allocNewBlock(TYPEVAL_DIRETORIO);
    aux = formatDirBlock(new_block);

    if (aux == ERRO){
        return ERRO;
    }
    inode.dataPtr[0] = new_block;

    aux = writeInode(inode_number, inode);
    if(aux == ERRO){
        printf("[formatDirINode] Erro ao escrever no i-node.\n");
    }

    //printf("[formatDirINode] Bloco de diretório para o i-node criado com sucesso\n");
    return SUCESSO;
}

/* Dada uma localização, esta função torna inválido o registro ali localizado */
int eraseRecord(struct record_location* location){
    unsigned char buffer_sector[SECTOR_SIZE];

    if (read_sector(location->sector, &buffer_sector[0]) != 0){
        printf("[readRecord] Erro ao ler setor indicado\n");
        return ERRO;
    }
    buffer_sector[location->position * DIR_SIZE] = INVALID_PTR;

    if (write_sector(location->sector, &buffer_sector[0]) != 0){
        printf("[writeInode] Erro ao gravar setor %d\n", location->sector);
        return ERRO;
    }
    return SUCESSO;
}

/* Recebe o número do inode e libera todos os blocos do i-node */
int freeBlocks(int inode_number){
    struct t2fs_inode inode;
    int aux;

    readInode(&inode, inode_number);

    /* Tenta localizar os blocos ocupados pelo arquivo */
    if(inode.dataPtr[0] !=	INVALID_PTR){
        setBitmap2 (BITMAP_DADOS, inode.dataPtr[0], LIVRE);
    }
    if(inode.dataPtr[1] !=	INVALID_PTR){
        setBitmap2 (BITMAP_DADOS, inode.dataPtr[1], LIVRE);
    }
    /* Tenta localizar o arquivos nos blocos apontados por indireção simples e dupla */
    if(inode.singleIndPtr != INVALID_PTR){
        setBitmap2 (BITMAP_DADOS, inode.singleIndPtr, LIVRE);

        int block = inode.singleIndPtr;
        aux = freeListBlock(block);
        if(aux == ERRO){
            return ERRO;
        }
    }
    if(inode.doubleIndPtr != INVALID_PTR){
        setBitmap2 (BITMAP_DADOS, inode.doubleIndPtr, LIVRE);

        int block = inode.doubleIndPtr;
        aux = freeDoubleListBlock(block);
        if(aux == ERRO){
            return ERRO;
        }
    }
    return SUCESSO;
}

/* Recebe um bloco que é uma lista de ponteiros para outros blocos */
int freeListBlock(int block){
    int sector, i, j, k, pointer;
    unsigned char buffer_sector[SECTOR_SIZE];
    char buffer_pointer[4];


    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[freeListBlock] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;
            if(pointer == TYPEVAL_INVALIDO){
                return SUCESSO;
            }
            else{
                setBitmap2 (BITMAP_DADOS, pointer, LIVRE);
            }
        }
    }
    return SUCESSO;
}

/* Recebe um bloco que é uma lista de ponteiros para uma lista de blocos */
int freeDoubleListBlock(int block){
    int sector, i, j, k, pointer, aux;
    unsigned char buffer_sector[SECTOR_SIZE];
    char buffer_pointer[4];


    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[freeListBlock] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;
            if(pointer == TYPEVAL_INVALIDO){
                return SUCESSO;
            }
            else{
                aux = freeListBlock(pointer);
                if(aux == ERRO){
                    return ERRO;
                }
            }
        }
    }
    return SUCESSO;
}

/* Recebe o número do i-node de um diretório e verifica se ele é vazio.
Retorna 0 se for vazio; -1, caso contrário.*/
int testEmpty(int inode_number){
    struct t2fs_inode inode;
    int aux;

    readInode(&inode, inode_number);

    /* Tenta localizar os blocos ocupados pelo arquivo */
    if(inode.dataPtr[0] !=	INVALID_PTR){
        /* Verifica se o bloco apontado é vazio (tipo inválido)*/

        int sector = blocks_start_sector + inode.dataPtr[0] * sectors_by_block;
        unsigned char buffer[SECTOR_SIZE];

        int b;
        for(b = 0; b < sectors_by_block; b++){
            if (read_sector(sector + b, &buffer[0]) != 0){
                printf("[testEmpty] Erro ao ler setor %d\n", sector + b);
                return ERRO;
            }

            if ((unsigned char)buffer[64] == TYPEVAL_INVALIDO){
                return SUCESSO;
            }
            else{
                return ERRO;
            }
        }
    }
    if(inode.dataPtr[1] !=	INVALID_PTR){
        /* Verifica se o bloco apontado é vazio (tipo inválido)*/
        int sector = blocks_start_sector + inode.dataPtr[1] * sectors_by_block;
        unsigned char buffer[SECTOR_SIZE];

        int b;
        for(b = 0; b < sectors_by_block; b++){
            if (read_sector(sector + b, &buffer[0]) != 0){
                printf("[testEmpty] Erro ao ler setor %d\n", sector + b);
                return ERRO;
            }
            if ((unsigned char)buffer[64] == TYPEVAL_INVALIDO){
                return SUCESSO;
            }
            else{
                return ERRO;
            }
        }
    }
    /* Tenta localizar o arquivos nos blocos apontados por indireção simples e dupla */
    if(inode.singleIndPtr != INVALID_PTR){

        int block = inode.singleIndPtr;
        aux = testEmptyBlock(block);
        if(aux == ERRO){
            return ERRO;
        }
    }
    if(inode.doubleIndPtr != INVALID_PTR){

        int block = inode.doubleIndPtr;
        aux = testEmptyList(block);
        if(aux == ERRO){
            return ERRO;
        }
    }
    return SUCESSO;
}

/* Recebe um bloco que é uma lista de ponteiros para outros blocos */
int testEmptyBlock(int block){
    int sector, i, j, k, pointer;
    unsigned char buffer_sector[SECTOR_SIZE];
    char buffer_pointer[4];


    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[freeListBlock] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;

            if(pointer != TYPEVAL_INVALIDO){
                int sector = blocks_start_sector + pointer * sectors_by_block;
                unsigned char buffer[SECTOR_SIZE];

                int b;
                for(b = 0; b < sectors_by_block; b++){
                    if (read_sector(sector + b, &buffer[0]) != 0){
                        printf("[testEmpty] Erro ao ler setor %d\n", sector + b);
                        return ERRO;
                    }

                    if ((unsigned char)buffer[64] != TYPEVAL_INVALIDO){
                        return ERRO;
                    }
                }
            }
        }
    }
    return SUCESSO;
}

/* Recebe um bloco que é uma lista de ponteiros para uma lista de blocos */
int testEmptyList(int block){
    int sector, i, j, k, pointer, aux;
    unsigned char buffer_sector[SECTOR_SIZE];
    char buffer_pointer[4];


    sector = blocks_start_sector + block * sectors_by_block;

    /* Irá varrer os setores do bloco, lendo um por vez */
    for(i=0; i < sectors_by_block; i++){
        if (read_sector(sector + i, &buffer_sector[0]) != 0){
            printf("[freeListBlock] Erro ao ler setor %d\n", sector + i);
            return ERRO;
        }

        /* Cada setor possui 64 ponteiros blocos de arquivos e subdiretórios */
        for(j=0; j < SECTOR_SIZE / 4; j++){
            for(k = 0; k < 4; k++){
                buffer_pointer[k] = buffer_sector[k + j*4];
            }
            pointer = *(int *)buffer_pointer;
            if(pointer == TYPEVAL_INVALIDO){
                return SUCESSO;
            }
            else{
                aux = testEmptyBlock(pointer);
                if(aux == ERRO){
                    return ERRO;
                }
            }
        }
    }
    return SUCESSO;
}

/* Recebe o número do bloco no disco e lê a n-ésima entrada deste bloco.
Retorna 0 se conseguiu ler, -1, caso contrário.
*/
int readNthEntry(int block, int entry, struct t2fs_record* record){

    int sector = blocks_start_sector + block *sectors_by_block;
    struct record_location location;
    location.sector = sector;
    location.position = entry % 4;

    int aux = readRecord(&location, record);

    if (aux == ERRO){
        printf("[readNthEntry] Erro ao ler registro %d no setor %d\n", location.position, location.sector);
        return ERRO;
    }
    return SUCESSO;
}
/* Recebe uma posição do bloco do arquivo e o i-node
Retorna o número do bloco se achou o bloco ou -1 em caso de erro */
int findBlock(int block_number, struct t2fs_inode* inode){
    if(block_number == 0){
        return inode->dataPtr[0];
    }
    if(block_number == 1){
        return inode->dataPtr[1];
    }
    if((block_number > 1)&&(block_number < 1026)){
        int sector, start_sector, sector_in_block, position, i;
        start_sector = blocks_start_sector + inode->singleIndPtr * sectors_by_block;
        /* Um bloco tem 1024 ponteiros de 4 bytes. Um setor, 64 ponteiros */
        sector_in_block = (block_number - 2) / 64;
        sector = start_sector + sector_in_block;
        position = (block_number - 2) % 64;

        unsigned char buffer_sector[SECTOR_SIZE];
        unsigned char buffer_block[4];

        printf("[findBlock] inode->singleIndPtr: %d\n", inode->singleIndPtr);
        if (read_sector(sector, &buffer_sector[0]) != 0){
            printf("[findBlock] Erro ao ler setor do registro inválido no diretório pai.\n");
            return ERRO;
        }

        for(i = 0; i < 4; i++){
            buffer_block[i] = buffer_sector[position*4 + i];
        }

        int new_block;
        memcpy(&new_block, &buffer_block[0], 4);
        printf("[findBlock] Buffer lido: %d\n", new_block);

        return new_block;
    }
    if(block_number >= 1026){
        int sector, start_sector, sector_in_block, position, i, second_block;
        unsigned char buffer_sector[SECTOR_SIZE];
        char buffer_block[4];

        start_sector = blocks_start_sector + inode->singleIndPtr * sectors_by_block;
        /* Um bloco tem 1024 ponteiros de 4 bytes. Um setor, 64 ponteiros */
        /* Na indireção dupla, cada ponteiro do bloco inicial endereça 1024 outros blocos */
        sector_in_block = ((block_number - 1026) / 1024) / 64;
        sector = start_sector + sector_in_block;
        position = ((block_number - 1026) / 1024) % 64;

        if (read_sector(sector, &buffer_sector[0]) != 0){
            printf("[writeRecord] Erro ao ler setor do registro inválido no diretório pai.\n");
            return ERRO;
        }

        for(i = 0; i < 4; i++){
            buffer_block[i] = buffer_sector[position*4 + i];
        }
        second_block = *(int *)buffer_block;

        start_sector = blocks_start_sector + second_block * sectors_by_block;
        /* Queremos descobrir o setor no segundo bloco, por isso o resto de 1024 */
        sector_in_block = ((block_number - 1026) % 1024) / sectors_by_block;
        sector = start_sector + sector_in_block;
        position = ((block_number - 1026) % 1024) % sectors_by_block;

        if (read_sector(sector, &buffer_sector[0]) != 0){
            printf("[writeRecord] Erro ao ler setor do registro inválido no diretório pai.\n");
            return ERRO;
        }

        for(i = 0; i < 4; i++){
            buffer_block[i] = buffer_sector[position*4 + i];
        }
        return *(int *)buffer_block;
    }
    return ERRO;
}

int findHandle(FILE2 handle, FILE2 *handles){
    int i;

    if(handle == INVALID_PTR)
        return ERRO;

    for(i = 0; i < 20; i++){
        if(handles[i] == handle){
            return SUCESSO;
        }
    }
    return ERRO;
}

int addHandle(FILE2 handle, FILE2 *handles){
    int i;
    for(i = 0; i < 20; i++){
        if(handles[i] == INVALID_PTR){
            handles[i] = handle;
            return SUCESSO;
        }
    }
    return ERRO;
}

int rmvHandle(FILE2 handle, FILE2 *handles){
    int i;

    if(handle == INVALID_PTR)
        return ERRO;

    for(i = 0; i < 20; i++){
        if(handles[i] == handle){
            handles[i] = INVALID_PTR;
            return SUCESSO;
        }
    }
    return ERRO;
}

int initHandle(FILE2 *handles, DIR2 *dir_handles){
    int i;
    for(i = 0; i < 20; i++){
        handles[i] = INVALID_PTR;
        dir_handles[i] = INVALID_PTR;
    }
    return SUCESSO;
}

int findHandleDir(DIR2 handle, DIR2 *handles){
    int i;

    if(handle == INVALID_PTR)
        return ERRO;

    for(i = 0; i < 20; i++){
        if(handles[i] == handle){
            return SUCESSO;
        }
    }
    return ERRO;
}

int addHandleDir(DIR2 handle, DIR2 *handles){
    int i;

    for(i = 0; i < 20; i++){
        if(handles[i] == INVALID_PTR){
            handles[i] = handle;
            return SUCESSO;
        }
    }
    return ERRO;
}

int rmvHandleDir(DIR2 handle, DIR2 *handles){
    int i;

    if(handle == INVALID_PTR)
        return ERRO;

    for(i = 0; i < 20; i++){
        if(handles[i] == handle){
            handles[i] = INVALID_PTR;
            return SUCESSO;
        }
    }
    return ERRO;
}

int recordRecord(struct file_descriptor *file){
    unsigned char buffer_sector[SECTOR_SIZE];
    unsigned char type_buffer;
    unsigned char name_buffer[32];
    unsigned char size_in_blocks_buffer[4];
    unsigned char size_in_bytes_buffer[4];
    unsigned char inode_number_buffer[4];

    struct t2fs_record record2 = file->record;

    /* Vai gravar as mudanças do registro do arquivo */
    if (read_sector(file->sector_record, &buffer_sector[0]) != 0){
        printf("[recordRecord] Erro ao ler setor do registro inválido no diretório pai.\n");
        return ERRO;
    }

    type_buffer = (char)file->record.TypeVal;
    memcpy(name_buffer, (char*)file->record.name, sizeof(file->record.name));
    memcpy(size_in_blocks_buffer, (char*)&record2.blocksFileSize, 4);
    memcpy(size_in_bytes_buffer, (char*)&record2.bytesFileSize, 4);
    memcpy(inode_number_buffer, (char*)&record2.inodeNumber, 4);

    /* Copia as informações de cada buffer intermediário para o buffer_sector */
    memcpy(&buffer_sector[file->record_index_in_sector * 64], &type_buffer, sizeof(type_buffer));
    memcpy(&buffer_sector[(file->record_index_in_sector * 64) + 1], &name_buffer, sizeof(name_buffer));
    memcpy(&buffer_sector[(file->record_index_in_sector * 64) + 33], &size_in_blocks_buffer, sizeof(size_in_blocks_buffer));
    memcpy(&buffer_sector[(file->record_index_in_sector * 64) + 37], &size_in_bytes_buffer, sizeof(size_in_bytes_buffer));
    memcpy(&buffer_sector[(file->record_index_in_sector * 64) + 41], &inode_number_buffer, sizeof(inode_number_buffer));

    /* Escreve no disco todo o setor */
    if (write_sector(file->sector_record, &buffer_sector[0]) != 0){
        printf("[recordRecord] Erro ao escrever setor de volta no disco (com o novo registro)\n");
        return ERRO;
    }
    return SUCESSO;
}
