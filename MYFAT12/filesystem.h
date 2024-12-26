#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAXNUMBEROFFILE 128
#define NUMBEROFBLOCKS 4096
#define READ 1
#define WRITE 2
#define READWRITE 3

typedef struct directory_entry {

    char filename[128];
    int parent_index;
    time_t last_modification;
    int size;
    int first_block;
    int isDirectory;
    int isExist;
    int permission;
    char password[32];
    
} directory_entry;


void init_directory_entry(directory_entry *entry, const char filename[], const int parent_index, const time_t creation_time, 
const int size, const int first_block, const int isDirectory);

#endif //FILESYSTEM_H
