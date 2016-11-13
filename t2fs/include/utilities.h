#define UTILITIES_H

#include "t2fs.h"
#include "apidisk.h"

struct t2fs_superbloco superblock;
struct t2fs_record current_working_directory;

/* Informações contidas no superbloco. Quantidade em número de setores */
int blocks_bitmap_size;
int inodes_bitmap_size;
int inodes_area_size;
int sectors_per_block;
int total_sectors_count;
