#include "operations.h"

extern int is_directories_changed;
extern int is_tables_changed;

int main(int argc, char* argv[]){


    if(argc < 3){
        printf("Insufficient number of elements...\n");
        exit(-1);
    }

    char* system_file = argv[1];

    char* operation = argv[2];

    char* paths = argv[3];

    char* other_file;
    if(argc > 4) other_file = argv[4];

    char password[128] = "NOPASSWORD";
    if(argc == 6) strcpy(password, argv[5]);
    if(argc == 5) strcpy(password, argv[4]);


    FILE* file = fopen(system_file, "rb");
    if(file == NULL){
        perror("fopen");
        exit(-1);
    }
 

    int block_size, free_table_blocks, fat_blocks, directory_blocks, metadata_blocks;

    fread(&block_size, sizeof(int), 1, file);
    fread(&free_table_blocks, sizeof(int), 1, file);
    fread(&fat_blocks, sizeof(int), 1, file);
    fread(&directory_blocks, sizeof(int), 1, file);
    fread(&metadata_blocks, sizeof(int), 1, file);

    fseek(file, (metadata_blocks * block_size) - (5 * sizeof(int)), SEEK_CUR);

    int free_table[NUMBEROFBLOCKS];

    fread(&free_table, sizeof(int), NUMBEROFBLOCKS, file);
    
    int fat_table[NUMBEROFBLOCKS];

    fread(&fat_table, sizeof(int), NUMBEROFBLOCKS, file);

    directory_entry directory_entries[MAXNUMBEROFFILE];

    fread(directory_entries, sizeof(directory_entry), MAXNUMBEROFFILE, file);

    int total_filled_blocks = free_table_blocks + fat_blocks + directory_blocks + metadata_blocks;
        
    fclose(file);

    int file_count;
    char ** file_path;
    int* file_indices;

    if(argc > 3) file_path = tokenize_path(paths, &file_count);


    if(strcmp(operation, "mkdir") == 0){
        
        if(file_count < 2){
            printf("Insufficent path!\n");
            return -1;
        }

        int* file_indices = find_file_indices(file_path, file_count - 1, directory_entries);

        mkdir(file_path[file_count - 1], file_indices, file_count - 1, directory_entries);

    }else if(strcmp(operation, "dir") == 0){

        int* file_indices = find_file_indices(file_path, file_count, directory_entries);

        dir(file_indices[file_count - 1], directory_entries);
        
    }else if(strcmp(operation, "write") == 0){

        if(argc < 5){ 
            printf("No source file is provided\n");
            return -1;
        }

        int* file_indices = find_file_indices(file_path, file_count - 1, directory_entries);

        write_file(other_file, file_path[file_count - 1], system_file, file_indices, file_count, 
        total_filled_blocks, block_size, directory_entries, fat_table, free_table, password);
  
    }else if(strcmp(operation, "read") == 0){
        
        if(argc < 5){ 
            printf("No destination file is provided\n");
            return -1;
        }

        int* file_indices = find_file_indices(file_path, file_count, directory_entries);

        read_file(other_file, system_file, 
        file_indices[file_count-1], block_size, directory_entries, fat_table, password);

    }    
    else if(strcmp(operation, "del") == 0){

        int* file_indices = find_file_indices(file_path, file_count, directory_entries);

        del(file_indices[file_count - 1] ,directory_entries, fat_table, free_table, password);
    }
    else if(strncmp(operation, "rmdir", 5) == 0){

        int* file_indices = find_file_indices(file_path, file_count, directory_entries);

        rmdirectory(file_indices[file_count - 1], directory_entries);

    } else if(strcmp(operation, "chmod") == 0){

        int* file_indices = find_file_indices(file_path, file_count, directory_entries);
        
        change_permission(file_indices[file_count - 1], directory_entries, other_file, password);

    } else if(strcmp(operation, "addpw") == 0){

        int* file_indices = find_file_indices(file_path, file_count, directory_entries);
        
        addpw(file_indices[file_count - 1], directory_entries, other_file);
    }
    else if(strcmp(operation, "dumpe2fs") == 0) dumpe2fs(block_size, free_table, fat_table, directory_entries);
    

    else printf("%s is not a valid command\n", operation);
    

    
    
    file = fopen(system_file, "rb+");
    if(file == NULL){
        perror("fopen");
        exit(-1);
    }

    if(is_tables_changed){

        rewind(file);

        fseek(file, metadata_blocks * block_size, SEEK_SET);

        for(int i = 0; i < NUMBEROFBLOCKS; ++i) fwrite(&free_table[i], sizeof(int), 1, file);

        for(int i = 0; i <  NUMBEROFBLOCKS; ++i) fwrite(&fat_table[i], sizeof(int), 1, file);
    }

    if(is_directories_changed){

        rewind(file);

        fseek(file, (metadata_blocks + free_table_blocks + fat_blocks) * block_size, SEEK_SET);

        for(int i = 0; i <  MAXNUMBEROFFILE; ++i) fwrite(&directory_entries[i], sizeof(directory_entry), 1, file);

    }

    fclose(file);

    return 0;
}


