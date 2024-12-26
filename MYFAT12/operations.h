#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "filesystem.h"

void dir(const int file_index, const directory_entry directory_entries[]);

void mkdir(const char* filename, const int* file_indices, const int file_count, directory_entry directory_entries[]);

void rmdirectory(const int file_index, directory_entry directory_entries[]);

void write_file(const char* source_file, const char* filename, const char* system_file, const int* file_indices, 
const int file_count, int filled_blocks, int block_size, directory_entry directory_entries[], int fat_table[], 
int free_table[], const char* password);

void read_file(const char* destiantion_file, const char* system_file, const int file_index, 
int block_size, directory_entry directory_entries[], int fat_table[], const char* password);

void del(const int file_index, directory_entry directory_entries[], int fat_table[], int free_table[], const char* password);

void dumpe2fs(const int block_size, const int free_table[], const int fat_table[], 
const directory_entry directory_entries[]);

char** tokenize_path(char* path, int* count);

int* find_file_indices(char** file_path, const int file_count, const directory_entry directory_entries[]);

int find_file(const char* filename, const int* file_indices, const int file_count, directory_entry directory_entries[]);

int check_permission(directory_entry my_directory_entry, int isWrite);

void change_permission(const int file_index, directory_entry directory_entries[], const char* new_permission, 
const char* password);

void addpw(const int file_index, directory_entry directory_entries[], const char* new_pw);

#endif //OPERATIONS_H