CC=gcc
EXE_DIR=../exemplos
LIB_DIR=../lib/
INC_DIR=../include
BIN_DIR=../bin
SRC_DIR=../src
CFLAGS= -Wall -c
OBJETOS= test_utilities

all: test_utilities

test_utilities: test_utilities.c $(LIB_DIR)/libt2fs.a
	$(CC) -g -o test_utilities test_utilities.c -L../lib/ -lt2fs -I$(INC_DIR) -lm -Wall

clean:
	rm -rf $(OBJETOS)
