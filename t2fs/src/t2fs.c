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
struct t2fs_superbloco superblock;
struct file_descriptor opened_files[20];

/* Função de Inicialização */
void initialize_data(){
    int aux, block_number;
    const int inode_number = 0;
    struct t2fs_inode inode;

    /* Lê o super bloco com as informações necessárias e inicializa os dados */
    aux = readSuperBlock(&superblock);

    if(getBitmap2(BITMAP_INODE, inode_number) != OCUPADO){
        aux += setBitmap2 (BITMAP_INODE, inode_number, OCUPADO);
        //printf("setBitmap2 do I-node 0.\n");
    }

    /* Lê o i-node 0 que possui as informações do diretório raiz */
    aux += readInode(&inode, inode_number);

    /* Se o i-node estiver vazio, inicializa o i-node e o bloco de dados */
    if(inode.dataPtr[0] == INVALID_PTR){
        /* Procura por um bloco livre no bitmap */
        block_number = searchBitmap2 (BITMAP_DADOS, LIVRE);

        if(block_number <= 0){
            printf("Erro ao localizar bloco livre.\n");
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

    if(aux == SUCESSO){
        //printf("Inicialização concluída corretamente\n");
        initialized = 1;
    }
    else{
        printf("Erro na inicialização.\n");
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
    char copy[32];

    strcpy(copy, filename);

    if(!initialized){
        initialize_data();
    }

    if (opened_files_count >= 20){
        printf("Foi atingindo o máximo de arquivos abertos simultaneamente\n");
        return ERRO;
    }

    if(isFileNameValid(filename) == ERRO){
        printf("O nome do arquivo informado não é válido.\n");
        return ERRO;
    }

    /* Verifica se o caminho em questão existe e, se existe,
    se já existe um arquivo com o mesmo nome.*/
    aux = findRecord(filename, &location);
    if(aux == ERRO){
        printf("Não existe o caminho especificado = %s\n", filename);
        return ERRO;
    }
    else if(aux == 1){
        printf("Já existe arquivo com o nome especificado = %s\n", filename);
        printf("Setor do arquivo = %d, posição no setor = %d\n", location.sector, location.position);
        return ERRO;
    }
    else if(aux == 0){
        // printf("Caminho informado válido = %s\n", filename);
        // printf("Setor do diretório-pai = %d, posição no setor = %d\n", location.sector, location.position);
    }

    aux = readRecord(&location, &parent_record);

    if (aux == ERRO){
        printf("Erro ao ler registro do diretório pai.\n");
        return ERRO;
    }

    int inode = findFreeINode();
    if (inode == ERRO){
        printf("Não existem i-nodes livres para criar o novo arquivo.\n");
        return ERRO;
    }
    else{
        aux = setBitmap2 (BITMAP_INODE, inode, OCUPADO);
        if(aux == ERRO){
            printf("Erro ao gravar o bitmap de i-node\n");
            return ERRO;
        }
    }

    /* Seleciona o nome do arquivo, sem o caminho absoluto */
    char *token = strtok(copy, "//0");
    char name[32];
    while(token) {
        strcpy(name, token);
        token = strtok(NULL, "//0");
    }
    // printf("name = %s\n", name);

    /* Cria registro e escreve-o no disco.*/
    record.TypeVal = TYPEVAL_REGULAR;
    strcpy(record.name, name);
    record.blocksFileSize = 1;
    record.bytesFileSize = 0;
    record.inodeNumber = inode;

    /* Grava o arquivo no diretório-pai */
    struct record_location new_file_location;
    aux = writeRecord(&record, &parent_record, &new_file_location);
    if (aux == ERRO){
        printf("Erro ao gravar o registro no diretório-pai\n");
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

    /* Retorna o ponteiro para o file_descriptor do arquivo e incrementa os arquivos abertos.*/
    opened_files_count++;

    return (int)descriptor;
}

/*-----------------------------------------------------------------------------
Função:	Apagar um arquivo do disco.
	O nome do arquivo a ser apagado é aquele informado pelo parâmetro "filename".

Entra:	filename -> nome do arquivo a ser apagado.

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int delete2 (char *filename){
    return ERRO;
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
    // Encontrar o i-node relativo ao arquivo (pesquisa nos diretórios da partição -> lookup)
    // Se o arquivo não existe: return -1
    // Senão, verifica se o arquivo já está aberto (procura por ele na tabela dos descritores de arquivos abertos)
    // Se o arquivo já está aberto: fim
    // Senão, aloca uma entrada livre na tabela TDAA e copia o i-node/descritor do arquivo para essa entrada (obs.: inicializar os campos adicionais)
    // (talvez) verificar se as permissões são satisfeitas

    return ERRO;
}


/*-----------------------------------------------------------------------------
Função:	Fecha o arquivo identificado pelo parâmetro "handle".

Entra:	handle -> identificador do arquivo a ser fechado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle){
    return ERRO;
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
    if (!initialized){
        initialize_data();
    }

    // Deve ser vazio, exceto por . e ..
    // newBlock()
    // createRecord(2, ...);

    return ERRO;
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
    return ERRO;
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
    return ERRO;
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
    return ERRO;
}


/*-----------------------------------------------------------------------------
Função:	Fecha o diretório identificado pelo parâmetro "handle".

Entra:	handle -> identificador do diretório que se deseja fechar (encerrar a operação).

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int closedir2 (DIR2 handle){
    return ERRO;
}
