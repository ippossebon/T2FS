CC=gcc
EXE_DIR=../exemplos/
LIB_DIR=../lib/
INC_DIR=../include/
BIN_DIR=../bin/
SRC_DIR=../src/
CFLAGS= -Wall -c

OBJETOS= test_initialize test_create2 test_open_close test_mkDir test_open_close_dir test_delete test_rmdir2 test_read test_read_duplo test_readdir2 teste2 test_seek

all: $(OBJETOS)

test_initialize: test_initialize.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_initialize test_initialize.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_create2: test_create2.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_create2 test_create2.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_open_close: test_open_close.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_open_close test_open_close.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_mkDir: test_mkDir.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_mkDir test_mkDir.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_open_close_dir: test_open_close_dir.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_open_close_dir test_open_close_dir.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_delete: test_delete.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_delete test_delete.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_rmdir2: test_rmdir2.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_rmdir2 test_rmdir2.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_read: test_read.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_read test_read.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_read_duplo: test_read_duplo.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_read_duplo test_read_duplo.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_readdir2: test_readdir2.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_readdir2 test_readdir2.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

teste2: teste2.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o teste2 teste2.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

test_seek: test_seek.c $(LIB_DIR)libt2fs.a
	$(CC) -g -o test_seek test_seek.c -L$(LIB_DIR) -lt2fs -I$(INC_DIR) -lm -Wall

clean:
	rm -rf $(OBJETOS)
