CC=gcc
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/
FLAGS= -g -Wall

all: t2fs utilities $(LIB_DIR)apidisk.o $(LIB_DIR)bitmap2.o
	ar crs $(LIB_DIR)libt2fs.a $(BIN_DIR)t2fs.o $(BIN_DIR)utilities.o $(LIB_DIR)apidisk.o $(LIB_DIR)bitmap2.o

utilities: $(SRC_DIR)utilities.c
	$(CC) -I$(INC_DIR) -o $(BIN_DIR)utilities.o -c $(SRC_DIR)utilities.c $(FLAGS)

t2fs: $(SRC_DIR)t2fs.c utilities
	$(CC) -I$(INC_DIR) -o $(BIN_DIR)t2fs.o -c $(SRC_DIR)t2fs.c $(FLAGS)

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~
