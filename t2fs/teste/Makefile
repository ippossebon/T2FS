CC=gcc
EXE_DIR=../exemplos/
LIB_DIR=../lib/
INC_DIR=../include/
BIN_DIR=../bin/
SRC_DIR=../src/
CFLAGS= -Wall -c
OBJETOS= test_initialize test_createINode

all: test_initialize test_createINode

test_initialize: test_initialize.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_initialize test_initialize.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_createINode: test_createINode.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_createINode test_createINode.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall
clean:
	rm -rf $(OBJETOS)
