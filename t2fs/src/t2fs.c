/* 	Arquivo de Código da Biblioteca t2fs.c
Implementado por Camila Haas Primieri e Isadora Pedrini Possebon
Sistemas Operacionais I - N
Universidade Federal do Rio Grande do Sul - UFRGS */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/utilities.h"

/* Globais */
int initialized = 0;
int opened_files_count = 0;
int opened_dirs_count = 0;

struct t2fs_superbloco superblock;
FILE2 handles[20];
DIR2 dir_handles[20];

/* Função de Inicialização */
void initialize_data(){
    int aux, block_number;
    const int inode_number = 0;
    struct t2fs_inode inode;

    /* Lê o super bloco com as informações necessárias e inicializa os dados */
    aux = readSuperBlock(&superblock);

    if(getBitmap2(BITMAP_INODE, inode_number) != OCUPADO){
        aux += setBitmap2 (BITMAP_INODE, inode_number, OCUPADO);
    }

    /* Lê o i-node 0 que possui as informações do diretório raiz */
    aux += readInode(&inode, inode_number);

    /* Se o i-node estiver vazio, inicializa o i-node e o bloco de dados */
    if(inode.dataPtr[0] == INVALID_PTR){
        /* Procura por um bloco livre no bitmap */
        block_number = searchBitmap2 (BITMAP_DADOS, LIVRE);

        if(block_number <= 0){
            printf("[initialize_data] Erro ao localizar bloco livre.\n");
            aux = ERRO;
        }
        else {
            aux += setBitmap2 (BITMAP_DADOS, block_number, OCUPADO);
        }

        inode.dataPtr[0] = block_number;
        inode.dataPtr[1] = INVALID_PTR;
        inode.singleIndPtr = INVALID_PTR;
        inode.doubleIndPtr = INVALID_PTR;

        /* Grava o Inode do Diretório Raiz */
        aux += writeInode(inode_number, inode);
        //printf("Gravado o i-node %d no disco, apontando para o bloco %d.\n", inode_number, block_number);

        /* Formata o bloco alocado para o diretório raiz */
        aux += formatDirBlock(block_number);
    }

    aux += initHandle(&handles[0], &dir_handles[0]);

    if(aux == SUCESSO){
        //printf("Inicialização concluída corretamente\n");
        initialized = 1;
    }
    else{
        printf("[initialize_data] Erro na inicialização.\n");
    }
}

int identify2 (char *name, int size){
    if(name == NULL){
        name = malloc(sizeof(char) * size);
    }

    if(size <= 0){
        printf("[cidentify] Erro: o tamanho não pode ser negativo.\n");
        return ERRO;
    }

    char students[] = "Camila Haas Primieri - 00172662 \nIsadora Pedrini Possebon - 00228551\n";
    int i = 0;

    while (i < size && i < sizeof(students)){
        name[i] = students[i];
        i++;
    }
    return SUCESSO;
}

/*-----------------------------------------------------------------------------
Função: Criar um novo arquivo.
	O nome desse novo arquivo é aquele informado pelo parâmetro "filename".
	O contador de posição do arquivo (current pointer) deve ser colocado na posição zero.
	Caso já exista um arquivo ou diretório com o mesmo nome, a função deverá retornar um erro de criação.
	A função deve retornar o identificador (handle) do arquivo.
	Esse handle será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do arquivo criado.
Entra:	filename -> nome do arquivo a ser criado.
Saída:	Se a operação foi realizada com sucesso, a função retorna o handle do arquivo (número positivo).
	Em caso de erro, deve ser retornado um valor negativo.
-----------------------------------------------------------------------------*/
FILE2 create2 (char *filename){
    int aux;
    struct record_location location;
    struct t2fs_record parent_record;
    struct t2fs_record record;
    char filename_copy[32];

    strcpy(filename_copy, filename);

    if(!initialized){
        initialize_data();
    }

    if (opened_files_count >= 20){
        printf("[create2] Foi atingindo o máximo de arquivos abertos simultaneamente\n");
        return ERRO;
    }

    if(isFileNameValid(filename) == ERRO){
        printf("[create2] O nome do arquivo informado não é válido.\n");
        return ERRO;
    }

    printf("[create2] filename = %s\n", filename);

    /* Verifica se o caminho em questão existe e, se existe, se já existe um arquivo com o mesmo nome.*/
    aux = findRecord(filename_copy, &location);
    if(aux == ERRO){
        printf("[create2] Não existe o caminho especificado = %s\n", filename);
        return ERRO;
    }
    else if(aux == 1){
        printf("[create2] Já existe arquivo com o nome especificado = %s\n", filename);
        //printf("[create2] Setor do arquivo = %d, posição no setor = %d\n", location.sector, location.position);
        return ERRO;
    }
    else if(aux == 0){
        printf("[create2] Caminho informado válido = %s\n", filename);
        printf("[create2] Setor do diretório-pai = %d, posição no setor = %d\n", location.sector, location.position);
    }

    aux = readRecord(&location, &parent_record);

    if (aux == ERRO){
        printf("[create2] Erro ao ler registro do diretório pai.\n");
        return ERRO;
    }

    int inode = findFreeINode();
    if (inode == ERRO){
        printf("[create2] Não existem i-nodes livres para criar o novo arquivo.\n");
        return ERRO;
    }
    else{
        aux = setBitmap2 (BITMAP_INODE, inode, OCUPADO);
        if(aux == ERRO){
            printf("[create2] Erro ao gravar o bitmap de i-node\n");
            return ERRO;
        }
    }

    /* Seleciona o nome do arquivo, sem o caminho absoluto */
    strcpy(filename_copy, filename);
    char *token = strtok(filename_copy, "//0");
    char name[32];
    while(token) {
        strcpy(name, token);
        token = strtok(NULL, "//0");
    }
    // printf("name = %s\n", name);

    /* Cria registro e escreve-o no disco.*/
    record.TypeVal = TYPEVAL_REGULAR;
    strcpy(record.name, name);
    record.blocksFileSize = 0;
    record.bytesFileSize = 0;
    record.inodeNumber = inode;

    /* Grava o arquivo no diretório-pai */
    struct record_location new_file_location;
    aux = writeRecord(&record, &parent_record, &new_file_location);
    if (aux == ERRO){
        printf("[create2] Erro ao gravar o registro no diretório-pai.\n");
        return ERRO;
    }

    struct file_descriptor* descriptor;
    descriptor = malloc(sizeof(struct file_descriptor));
    descriptor->record.TypeVal = record.TypeVal;
    strcpy(descriptor->record.name, record.name);
    descriptor->record.blocksFileSize = record.blocksFileSize;
    descriptor->record.bytesFileSize = record.bytesFileSize;
    descriptor->record.inodeNumber = record.inodeNumber;
    descriptor->current_pointer = 0;
    descriptor->sector_record = new_file_location.sector;
    descriptor->record_index_in_sector = new_file_location.position;

    aux = addHandle((FILE2)descriptor, &handles[0]);
    if (aux == ERRO){
        printf("[create2] Erro ao criar o handle do arquivo.\n");
        return ERRO;
    }

    /* Retorna o ponteiro para o file_descriptor do arquivo e incrementa os arquivos abertos.*/
    opened_files_count++;

    return (FILE2)descriptor;
}

/*-----------------------------------------------------------------------------
Função:	Apagar um arquivo do disco.
	O nome do arquivo a ser apagado é aquele informado pelo parâmetro "filename".
Entra:	filename -> nome do arquivo a ser apagado.
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int delete2 (char *filename){
    char filename_copy[32];
    struct t2fs_record record;
    struct record_location location;
    int aux;

    strcpy(filename_copy, filename);

    if(!initialized){
        initialize_data();
    }

    if(isFileNameValid(filename) == ERRO){
        printf("[delete2] O nome do arquivo informado não é válido.\n");
        return ERRO;
    }

    printf("[delete2] filename = %s\n", filename);

    /* Verifica se o caminho em questão existe e se existe um arquivo com o nome informado.*/
    aux = findRecord(filename_copy, &location);
    if(aux == ERRO){
        printf("[delete2] Não existe o caminho especificado = %s\n", filename);
        return ERRO;
    }
    else if(aux == 0){
        printf("[delete2] Não existe o arquivo %s no diretório informado.\n", filename);
        return ERRO;
    }
    else if(aux == 1){
        printf("[delete2] Localizado arquivo com o nome informado = %s\n", filename);
    }

    /* Lê o registro do arquivo que será apagado */
    aux = readRecord(&location, &record);
    if (aux == ERRO){
        printf("[delete2] Erro ao ler registro do arquivo.\n");
        return ERRO;
    }

    if (record.TypeVal != TYPEVAL_REGULAR){
        printf("[delete2] Este arquivo não é um arquivo regular.\n");
        return ERRO;
    }

    /* Apaga o registro do arquivo */
    aux = eraseRecord(&location);
    if (aux == ERRO){
        printf("[delete2] Erro ao apagar registro do arquivo.\n");
        return ERRO;
    }

    /* Libera o bitmap do i-node do arquivo */
    setBitmap2 (BITMAP_INODE, record.inodeNumber, LIVRE);

    /* Libera o bitmap dos blocos de dados utilizados pelo arquivo */
    aux = freeBlocks(record.inodeNumber);
    if (aux == ERRO){
        printf("[delete2] Erro ao liberar os blocos do arquivo.\n");
        return ERRO;
    }

    printf("[delete2] Arquivo %s apagado com sucesso.\n", filename);
    return SUCESSO;
}


/*-----------------------------------------------------------------------------
Função:	Abre um arquivo existente no disco.
	O nome desse novo arquivo é aquele informado pelo parâmetro "filename".
	Ao abrir um arquivo, o contador de posição do arquivo (current pointer) deve ser colocado na posição zero.
	A função deve retornar o identificador (handle) do arquivo.
	Esse handle será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do arquivo criado.
	Todos os arquivos abertos por esta chamada são abertos em leitura e em escrita.
	O ponto em que a leitura, ou escrita, será realizada é fornecido pelo valor current_pointer (ver função seek2).
Entra:	filename -> nome do arquivo a ser apagado.
Saída:	Se a operação foi realizada com sucesso, a função retorna o handle do arquivo (número positivo)
	Em caso de erro, deve ser retornado um valor negativo
-----------------------------------------------------------------------------*/
FILE2 open2 (char *filename){
    char filename_copy[32];
    struct t2fs_record record;
    struct file_descriptor* descriptor;
    struct record_location location;
    int aux;

    strcpy(filename_copy, filename);

    if(!initialized){
        initialize_data();
    }

    if (opened_files_count >= 20){
        printf("[open2] Foi atingindo o máximo de arquivos abertos simultaneamente\n");
        return ERRO;
    }

    if(isFileNameValid(filename) == ERRO){
        printf("[open2] O nome do arquivo informado não é válido.\n");
        return ERRO;
    }

    printf("[open2] filename = %s\n", filename);

    /* Verifica se o caminho em questão existe e se existe um arquivo com o nome informado.*/
    aux = findRecord(filename_copy, &location);
    if(aux == ERRO){
        printf("[open2] Não existe o caminho especificado = %s\n", filename);
        return ERRO;
    }
    else if(aux == 0){
        printf("[open2] Não existe o arquivo %s no diretório informado.\n", filename);
        return ERRO;
    }
    else if(aux == 1){
        printf("[open2] Localizado arquivo com o nome informado = %s\n", filename);
    }

    /* Lê o registro do arquivo que será aberto */
    aux = readRecord(&location, &record);
    if (aux == ERRO){
        printf("[open2] Erro ao ler registro do arquivo.\n");
        return ERRO;
    }

    if (record.TypeVal != TYPEVAL_REGULAR){
        printf("[open2] Este arquivo não é um arquivo regular.\n");
        return ERRO;
    }

    /* Cria a estrutura para armazenar o descritor do arquivo aberto*/
    descriptor = malloc(sizeof(struct file_descriptor));
    descriptor->record.TypeVal = record.TypeVal;
    strcpy(descriptor->record.name, record.name);
    descriptor->record.blocksFileSize = record.blocksFileSize;
    descriptor->record.bytesFileSize = record.bytesFileSize;
    descriptor->record.inodeNumber = record.inodeNumber;
    descriptor->current_pointer = 0;
    descriptor->sector_record = location.sector;
    descriptor->record_index_in_sector = location.position;

    aux = addHandle((FILE2)descriptor, &handles[0]);
    if (aux == ERRO){
        printf("[open2] Erro ao criar o handle do arquivo.\n");
        return ERRO;
    }

    /* Retorna o ponteiro para o file_descriptor do arquivo e incrementa os arquivos abertos.*/
    opened_files_count++;

    return (FILE2)descriptor;
}


/*-----------------------------------------------------------------------------
Função:	Fecha o arquivo identificado pelo parâmetro "handle".
Entra:	handle -> identificador do arquivo a ser fechado
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle){
    struct file_descriptor *file;
    int aux;

    if(!initialized){
        initialize_data();
    }

    aux = rmvHandle(handle, &handles[0]);
    if (aux == ERRO){
        printf("[close2] Erro ao remover o handle do arquivo.\n");
        return ERRO;
    }

    file = (struct file_descriptor *)handle;

    if (file->record.TypeVal != TYPEVAL_REGULAR){
        printf("[closedir2] Este arquivo não é um arquivo regular\n");
        return ERRO;
    }

    printf("[close2] Fechando o arquivo com o handle número = %d\n", handle);
    free(file);
    file = NULL;

    return SUCESSO;
}


/*-----------------------------------------------------------------------------
Função:	Realiza a leitura de "size" bytes do arquivo identificado por "handle".
	Os bytes lidos são colocados na área apontada por "buffer".
	Após a leitura, o contador de posição (current pointer) deve ser ajustado para o byte seguinte ao último lido.
Entra:	handle -> identificador do arquivo a ser lido
	buffer -> buffer onde colocar os bytes lidos do arquivo
	size -> número de bytes a serem lidos
Saída:	Se a operação foi realizada com sucesso, a função retorna o número de bytes lidos.
	Se o valor retornado for menor do que "size", então o contador de posição atingiu o final do arquivo.
	Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
int read2 (FILE2 handle, char *buffer, int size){
    struct file_descriptor *file;
    struct t2fs_inode inode;
    int current, file_size, read_limit, aux, file_block, block_number, buffer_index = 0, bytes_to_read, blocks_start_sector;

    if(!initialized){
        initialize_data();
    }

    aux = findHandle(handle, &handles[0]);
    if (aux == ERRO){
        printf("[read2] O arquivo especificado não está aberto.\n");
        return ERRO;
    }

    file = (struct file_descriptor *)handle;

    if (file->record.TypeVal != TYPEVAL_REGULAR){
        printf("[read2] Este arquivo não é um arquivo regular\n");
        return ERRO;
    }

    current = file->current_pointer;
    if (current < 0){
        return ERRO;
    }
    file_size = file->record.bytesFileSize;
    if (file_size < 0){
        return ERRO;
    }

    /* read_limit é o valor em bytes de até onde se deseja ler no arquivo */
    read_limit = size + current;

    if (file_size == 0){
        printf("[read2] Arquivo vazio.\n");
        return 0;
    }
    if (current > file_size){
        printf("[read2] O current_pointer é maior que o tamanho do arquivo.\n");
        return ERRO;
    }
    /* teste para ver se a quantidade de bytes do arquivo é maior que o tamanho solicitado
    Se for, diminui a quantidade de bytes a serem lidos */
    if (file_size < read_limit){
        printf("[read2] Você deseja ler mais bytes do que existem a partir da posição atual do arquivo.\n");
        read_limit = file_size;
    }
    bytes_to_read = read_limit - current;

    /* Lê o i-node do arquivo */
    aux = readInode(&inode, file->record.inodeNumber);
    if(aux != 0){
        printf("[read2] Inode de diretório inválido\n");
        return ERRO;
    }

    /* 4096 bytes por bloco. Logo, achamos pra qual bloco o current apontada
        P.S: não é o número do bloco no disco, mas referente aos blocos do arquivo */
    file_block = current / 4096;
    blocks_start_sector = (int)superblock.superblockSize + (int)superblock.freeBlocksBitmapSize + (int)superblock.freeInodeBitmapSize + (int)superblock.inodeAreaSize;

    while (buffer_index < size) {
        int i, j, sector;
        unsigned char buffer_sector[SECTOR_SIZE];

        block_number = FindBlock(file_block, &inode);
        if(block_number == ERRO){
            printf("[read2] Erro ao procurar os blocos do arquivo.\n");
            return ERRO;
        }

        sector = blocks_start_sector + block_number * 16;
        /* Vai ler os 16 setores do bloco */
        for(i = 0; i < 16; i++){
            if (read_sector(sector + i, &buffer_sector[0]) != 0){
                printf("[read2] Erro ao ler setor do bloco do arquivo.\n");
                return ERRO;
            }
            for(j = 0; j < 256; j++){
                buffer[buffer_index] = buffer_sector[j];
                current++;
                buffer_index++;

                if(current > read_limit){
                    file->current_pointer = current;
                    return bytes_to_read;
                }
            }
        }
        file_block++;
    }
    return ERRO;
}


/*-----------------------------------------------------------------------------
Função:	Realiza a escrita de "size" bytes no arquivo identificado por "handle".
	Os bytes a serem escritos estão na área apontada por "buffer".
	Após a escrita, o contador de posição (current pointer) deve ser ajustado para o byte seguinte ao último escrito.
Entra:	handle -> identificador do arquivo a ser escrito
	buffer -> buffer de onde pegar os bytes a serem escritos no arquivo
	size -> número de bytes a serem escritos
Saída:	Se a operação foi realizada com sucesso, a função retorna o número de bytes efetivamente escritos.
	Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
int write2 (FILE2 handle, char *buffer, int size){
    return ERRO;
}


/*-----------------------------------------------------------------------------
Função:	Função usada para truncar um arquivo.
	Remove do arquivo todos os bytes a partir da posição atual do contador de posição (current pointer)
	Todos os bytes desde a posição indicada pelo current pointer até o final do arquivo são removidos do arquivo.
Entra:	handle -> identificador do arquivo a ser truncado
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int truncate2 (FILE2 handle){
    return ERRO;
}


/*-----------------------------------------------------------------------------
Função:	Reposiciona o contador de posições (current pointer) do arquivo identificado por "handle".
	A nova posição é determinada pelo parâmetro "offset".
	O parâmetro "offset" corresponde ao deslocamento, em bytes, contados a partir do início do arquivo.
	Se o valor de "offset" for "-1", o current_pointer deverá ser posicionado no byte seguinte ao final do arquivo,
		Isso é útil para permitir que novos dados sejam adicionados no final de um arquivo já existente.
Entra:	handle -> identificador do arquivo a ser escrito
	offset -> deslocamento, em bytes, onde posicionar o "current pointer".
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int seek2 (FILE2 handle, DWORD offset){
    return ERRO;
}


/*-----------------------------------------------------------------------------
Função:	Criar um novo diretório.
	O caminho desse novo diretório é aquele informado pelo parâmetro "pathname".
		O caminho pode ser ser absoluto ou relativo.
	A criação de um novo subdiretório deve ser acompanhada pela criação, automática, das entradas "." e ".."
	A entrada "." corresponde ao descritor do subdiretório recém criado
	A entrada ".." corresponde à entrada de seu diretório pai.
	São considerados erros de criação quaisquer situações em que o diretório não possa ser criado.
		Isso inclui a existência de um arquivo ou diretório com o mesmo "pathname".
Entra:	pathname -> caminho do diretório a ser criado
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int mkdir2 (char *pathname){
    int aux;
    struct record_location location;
    struct t2fs_record parent_record;
    struct t2fs_record record;
    char copy[32];

    strcpy(copy, pathname);

    if(!initialized){
        initialize_data();
    }

    printf("pathname = %s\n", pathname);
    if(isFileNameValid(pathname) == ERRO){
        printf("[mkdir2] O nome informado para o diretório não é válido.\n");
        return ERRO;
    }

    printf("[mkdir2] pathname = %s\n", pathname);

    /* Verifica se o caminho em questão existe e, se existe,
    se já existe um diretório com o mesmo nome.*/
    aux = findRecord(pathname, &location);
    if(aux == ERRO){
        printf("[mkdir2] Não existe o caminho especificado = %s\n", pathname);
        return ERRO;
    }
    else if(aux == 1){
        printf("[mkdir2] Já existe diretório com o nome especificado = %s\n", copy);
        printf("[mkdir2] Setor do diretório = %d, posição no setor = %d\n", location.sector, location.position);
        return ERRO;
    }
    else if(aux == 0){
        // printf("Caminho informado válido = %s\n", filename);
        // printf("Setor do diretório-pai = %d, posição no setor = %d\n", location.sector, location.position);
    }

    /* Lê registro do diretório pai para escrever dentro dele. */
    aux = readRecord(&location, &parent_record);

    if (aux == ERRO){
        printf("[mkdir2] Erro ao ler registro do diretório pai.\n");
        return ERRO;
    }

    /* Aloca um novo i-node para o novo diretório. */
    int inode = findFreeINode();
    if (inode == ERRO){
        printf("[mkdir2] Não existem i-nodes livres para criar o novo diretório.\n");
        return ERRO;
    }
    else{
        aux = setBitmap2 (BITMAP_INODE, inode, OCUPADO);
        if(aux == ERRO){
            printf("[mkdir2] Erro ao gravar o bitmap de i-node\n");
            return ERRO;
        }
    }

    /* Aloca o primeiro bloco para o i-node do novo diretório, formatando-o como
    diretório. */
    aux = formatDirINode(inode);
    if (aux == ERRO){
        printf("[mkdir] Erro ao alocar/formatar primeiro bloco do diretório.\n");
        return ERRO;
    }

    /* Seleciona o nome do diretório, sem o caminho absoluto */
    char *token = strtok(copy, "//0");
    char name[32];
    while(token) {
        strcpy(name, token);
        token = strtok(NULL, "//0");
    }
    // printf("name = %s\n", name);

    /* Cria registro e escreve-o no disco.*/
    record.TypeVal = TYPEVAL_DIRETORIO;
    strcpy(record.name, name);
    record.blocksFileSize = 1;
    record.bytesFileSize = 0;
    record.inodeNumber = inode;

    /* Grava o arquivo no diretório-pai */
    struct record_location new_dir_location;
    aux = writeRecord(&record, &parent_record, &new_dir_location);
    if (aux == ERRO){
        printf("[mkdir2] Erro ao gravar o registro no diretório-pai\n");
        return ERRO;
    }

    struct file_descriptor* descriptor;
    descriptor = malloc(sizeof(struct file_descriptor));
    descriptor->record.TypeVal = record.TypeVal;
    strcpy(descriptor->record.name, record.name);
    descriptor->record.blocksFileSize = record.blocksFileSize;
    descriptor->record.bytesFileSize = record.bytesFileSize;
    descriptor->record.inodeNumber = record.inodeNumber;
    descriptor->current_pointer = 0;
    descriptor->sector_record = new_dir_location.sector;
    descriptor->record_index_in_sector = new_dir_location.position;

    printf("[mkdir] Diretório criado com sucesso\n");
    return SUCESSO;
}

/*-----------------------------------------------------------------------------
Função:	Apagar um subdiretório do disco.
	O caminho do diretório a ser apagado é aquele informado pelo parâmetro "pathname".
	São considerados erros quaisquer situações que impeçam a operação.
		Isso inclui:
			(a) o diretório a ser removido não está vazio;
			(b) "pathname" não existente;
			(c) algum dos componentes do "pathname" não existe (caminho inválido);
			(d) o "pathname" indicado não é um arquivo;
			(e) o "pathname" indica os diretórios "." ou "..".
Entra:	pathname -> caminho do diretório a ser criado
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int rmdir2 (char *pathname){
    int aux;
    struct record_location location;
    struct t2fs_record record;
    char copy[32];

    strcpy(copy, pathname);

    if(!initialized){
        initialize_data();
    }

    if(isFileNameValid(pathname) == ERRO){
        printf("[rmdir2] O nome informado para o diretório não é válido.\n");
        return ERRO;
    }

    printf("[rmdir2] pathname = %s\n", pathname);

    /* Verifica se o caminho em questão existe e, se existe,
    se já existe um diretório com o mesmo nome.*/
    aux = findRecord(pathname, &location);
    if(aux == ERRO){
        printf("[rmdir2] Não existe o caminho especificado = %s\n", pathname);
        return ERRO;
    }
    else if(aux == 0){
        printf("[rmdir2] Este subdiretório não existe: %s\n", pathname);
        return ERRO;
    }
    else if(aux == 1){
        printf("[rmdir2] O subdiretório %s está no setor %d, posição %d\n", copy, location.sector, location.position);
    }

    /* Lê o registro do diretório que será apagado */
    aux = readRecord(&location, &record);
    if (aux == ERRO){
        printf("[rmdir2] Erro ao ler registro do diretório.\n");
        return ERRO;
    }

    if (record.TypeVal != TYPEVAL_DIRETORIO){
        printf("[rmdir2] Este arquivo não é um diretório.\n");
        return ERRO;
    }

    if (testEmpty(record.inodeNumber) == ERRO){
        printf("[rmdir2] Este diretório não é vazio.\n");
        return ERRO;
    }

    /* Apaga o registro do arquivo */
    aux = eraseRecord(&location);
    if (aux == ERRO){
        printf("[rmdir2] Erro ao apagar registro do diretório.\n");
        return ERRO;
    }

    /* Libera o bitmap do i-node do arquivo */
    setBitmap2 (BITMAP_INODE, record.inodeNumber, LIVRE);

    /* Libera o bitmap dos blocos de dados utilizados pelo arquivo */
    aux = freeBlocks(record.inodeNumber);
    if (aux == ERRO){
        printf("[rmdir2] Erro ao liberar os blocos do diretório.\n");
        return ERRO;
    }

    printf("[rmdir2] Arquivo %s apagado com sucesso.\n", pathname);
    return SUCESSO;
}


/*-----------------------------------------------------------------------------
Função:	Abre um diretório existente no disco.
	O caminho desse diretório é aquele informado pelo parâmetro "pathname".
	Se a operação foi realizada com sucesso, a função:
		(a) deve retornar o identificador (handle) do diretório
		(b) deve posicionar o ponteiro de entradas (current entry) na primeira posição válida do diretório "pathname".
	O handle retornado será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do diretório.
Entra:	pathname -> caminho do diretório a ser aberto
Saída:	Se a operação foi realizada com sucesso, a função retorna o identificador do diretório (handle).
	Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
DIR2 opendir2 (char *pathname){

    char pathname_copy[32];
    struct t2fs_record record;
    struct file_descriptor* descriptor;
    struct record_location location;
    int aux;

    strcpy(pathname_copy, pathname);

    if(!initialized){
        initialize_data();
    }

    if (opened_dirs_count >= 20){
        printf("[opendir2] Foi atingindo o máximo de diretórios abertos simultaneamente\n");
        return ERRO;
    }

    if(isFileNameValid(pathname) == ERRO){
        printf("[opendir2] O nome do diretório informado não é válido.\n");
        return ERRO;
    }

    printf("[opendir2] pathname = %s\n", pathname);

    /* Verifica se o caminho em questão existe e se existe um diretório com o nome informado.*/
    aux = findRecord(pathname_copy, &location);
    if(aux == ERRO){
        printf("[opendir2] Não existe o caminho especificado = %s\n", pathname);
        return ERRO;
    }
    else if(aux == 0){
        printf("[opendir2] Não existe o diretório %s no caminho informado.\n", pathname);
        return ERRO;
    }
    else if(aux == 1){
        printf("[opendir2] Localizado diretório com o nome informado = %s\n", pathname);
    }

    /* Lê o registro do arquivo que será aberto */
    aux = readRecord(&location, &record);

    if (aux == ERRO){
        printf("[opendir2] Erro ao ler registro do arquivo.\n");
        return ERRO;
    }

    if (record.TypeVal != TYPEVAL_DIRETORIO){
        printf("[opendir2] Este arquivo não é um diretório.\n");
        return ERRO;
    }

    /* Cria a estrutura para armazenar o descritor do arquivo aberto*/
    descriptor = malloc(sizeof(struct file_descriptor));
    descriptor->record.TypeVal = record.TypeVal;
    strcpy(descriptor->record.name, record.name);
    descriptor->record.blocksFileSize = record.blocksFileSize;
    descriptor->record.bytesFileSize = record.bytesFileSize;
    descriptor->record.inodeNumber = record.inodeNumber;
    descriptor->current_pointer = 0;
    descriptor->sector_record = location.sector;
    descriptor->record_index_in_sector = location.position;

    aux = addHandleDir((DIR2)descriptor, &dir_handles[0]);
    if (aux == ERRO){
        printf("[opendir2] Erro ao criar o handle do diretório.\n");
        return ERRO;
    }

    /* Retorna o ponteiro para o file_descriptor do arquivo e incrementa os arquivos abertos.*/
    opened_dirs_count++;

    return (DIR2)descriptor;
}


/*-----------------------------------------------------------------------------
Função:	Realiza a leitura das entradas do diretório identificado por "handle".
	A cada chamada da função é lida a entrada seguinte do diretório representado pelo identificador "handle".
	Algumas das informações dessas entradas devem ser colocadas no parâmetro "dentry".
	Após realizada a leitura de uma entrada, o ponteiro de entradas (current entry) deve ser ajustado para a próxima entrada válida, seguinte à última lida.
	São considerados erros:
		(a) qualquer situação que impeça a realização da operação
		(b) término das entradas válidas do diretório identificado por "handle".
Entra:	handle -> identificador do diretório cujas entradas deseja-se ler.
	dentry -> estrutura de dados onde a função coloca as informações da entrada lida.
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero ( e "dentry" não será válido)
-----------------------------------------------------------------------------*/
int readdir2 (DIR2 handle, DIRENT2 *dentry){
    struct file_descriptor *dir;

    if(!initialized){
        initialize_data();
    }

    int aux = findHandleDir(handle, &dir_handles[0]);
    if (aux == ERRO){
        printf("[readdir2] O diretório especificado não está aberto.\n");
        return ERRO;
    }

    dir = (struct file_descriptor *)handle;

    if (dir->record.TypeVal != TYPEVAL_DIRETORIO){
        printf("[readdir2] Este arquivo não é um diretório\n");
        return ERRO;
    }

    if (testEmpty(dir->record.inodeNumber) == SUCESSO){
        printf("[readdir2] Este diretório é vazio.\n");
        return ERRO;
    }

    /* Recupera i-node do diretório. */
    struct t2fs_inode inode;
    aux = readInode(&inode, dir->record.inodeNumber);

    if (aux == ERRO){
        printf("[readdir2] Erro ao ler i-node do diretório\n");
        return ERRO;
    }

    /* Procura em qual bloco, no disco, o current_pointer está. */
    int blocks_start_sector = (int)superblock.inodeAreaSize + (int)superblock.superblockSize + (int)superblock.freeBlocksBitmapSize + (int)superblock.freeInodeBitmapSize;
    int sectors_by_block = (int)superblock.blockSize;
    int block;
    if (dir->current_pointer < 4096){
        block = inode.dataPtr[0];
    }
    else if (dir->current_pointer < 4096 * 2){
        block = inode.dataPtr[1];
    }
    else if (dir->current_pointer < 1024 * 4096){
        // Procura bloco na lista de indireção simples

        int block_number = (int) (dir->current_pointer / 4096);
        int sector, start_sector, sector_in_block, position, i;
        start_sector = blocks_start_sector + inode.singleIndPtr * sectors_by_block;

        /* Um bloco tem 1024 ponteiros de 4 bytes. Um setor, 64 ponteiros */
        sector_in_block = (block_number - 2) / 64;
        sector = start_sector + sector_in_block;
        position = (block_number - 2) % 64;

        unsigned char buffer_sector[SECTOR_SIZE];
        char buffer_block[4];

        if (read_sector(sector, &buffer_sector[0]) != 0){
            printf("[readdir2] Erro ao ler setor do registro inválido no diretório pai.\n");
            return ERRO;
        }

        for(i = 0; i < 4; i++){
            buffer_block[i] = buffer_sector[position*4 + i];
        }
        block = *(int *)buffer_block;
    }
    /* dir->current_pointer < 1024 * 1024 * 4096, mas 2147483647 é o limite de representação de INT*/
    else if (dir->current_pointer < 2147483646){
        // Procura bloco na lista de indireção dupla

        int block_number = (int) (dir->current_pointer / 4096);
        int sector, start_sector, sector_in_block, position, i, second_block;
        unsigned char buffer_sector[SECTOR_SIZE];
        char buffer_block[4];

        start_sector = blocks_start_sector + inode.singleIndPtr * sectors_by_block;
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
        block = *(int *)buffer_block;
    }
    else{
        printf("[readdir2] Entrada fora dos limites\n");
        return ERRO;
    }

    int entry_number = (int) (dir->current_pointer / 64);
    struct t2fs_record entry_record;
    aux = readNthEntry(block, entry_number, &entry_record);

    if (aux == ERRO){
        printf("[readdir2] Erro ao ler entrada do diretório\n");
        return ERRO;
    }

    /* Copia os dados do registro para a estrutura de retorno. */
    strcpy(dentry->name, entry_record.name);
    dentry->fileType = entry_record.TypeVal;
    dentry->fileSize = entry_record.bytesFileSize;

    /* Aponta para a próxima entrada */
    /**** SALVAR ESTE VALOR NA STRUCT ***/
    dir->current_pointer = dir->current_pointer + 64;

    return SUCESSO;
}


/*-----------------------------------------------------------------------------
Função:	Fecha o diretório identificado pelo parâmetro "handle".
Entra:	handle -> identificador do diretório que se deseja fechar (encerrar a operação).
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int closedir2 (DIR2 handle){
    int aux;
    struct file_descriptor *dir;

    if(!initialized){
        initialize_data();
    }

    aux = rmvHandleDir((DIR2)handle, &dir_handles[0]);
    if (aux == ERRO){
        printf("[closedir2] Erro ao remover o handle do diretório.\n");
        return ERRO;
    }

    dir = (struct file_descriptor *)handle;

    if (dir->record.TypeVal != TYPEVAL_DIRETORIO){
        printf("[closedir2] Este arquivo não é um diretório\n");
        return ERRO;
    }

    printf("[closedir2] Fechando o diretório com o handle número = %d\n", handle);
    free(dir);
    dir = NULL;

    return SUCESSO;
}
