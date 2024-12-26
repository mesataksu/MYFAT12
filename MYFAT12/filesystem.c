#include "filesystem.h"

void init_directory_entry(directory_entry *entry, const char filename[], const int parent_index, const time_t last_modification, 
const int size, const int first_block, const int isDirectory){

    strcpy(entry->filename, filename);
    entry->parent_index = parent_index;
    entry->last_modification = last_modification;
    entry->size = size;
    entry->first_block = first_block;
    entry->isDirectory = isDirectory;
    entry->isExist = 1;
    entry->permission = READWRITE; //on defaul every file has READWRITE permisson
    strcpy(entry->password, "NOPASSWORD");

}