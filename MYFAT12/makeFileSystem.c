#include "filesystem.h"

int main(int argc, char * argv[]){

    if(argc < 3){
        printf("Insufficient number of elements\n");
        exit(-1);
    }

    // Convert argv[1] to a float
    float block_size_kb = atof(argv[1]);

    if (block_size_kb != 1.0 && block_size_kb != 0.5) {
        printf("The system only supports 0.5KB and 1KB sizes\n");
        return -1;
    }

    //  Block size in bytes is calculated
    int block_size = 1024 * block_size_kb; 

    int free_table_blocks = (NUMBEROFBLOCKS * sizeof(int)) / block_size;
    int fat_blocks = free_table_blocks;

    // Number of directory blocks is calculated
    // + block_size - 1 added to avoid false truncation
    int directory_blocks = ((MAXNUMBEROFFILE * sizeof(directory_entry)) + block_size - 1) / block_size;

    // Root directory is /
    // It has no parents
    char rootName[128] = "/"; 
    int root_parent_index = -1;


    // Last modification date is 0
    // Size is 0
    // First block is 0

    // Is Directory
    directory_entry rootDir;
    time_t current_time;
    time(&current_time);
    init_directory_entry(&rootDir, rootName, root_parent_index, current_time, 0, 0, 1);

    // Create the system file
    char * file_system_name = argv[2];

    FILE * file = fopen(file_system_name, "wb");
    if(file == NULL){
        perror("fopen");
        exit(-1);
    }


    // Write the metadata in the system file
    int metadata_blocks= 1;
    fwrite(&block_size, sizeof(int), 1, file);
    fwrite(&free_table_blocks, sizeof(int), 1, file);
    fwrite(&fat_blocks, sizeof(int), 1, file);
    fwrite(&directory_blocks, sizeof(int), 1, file);
    fwrite(&metadata_blocks, sizeof(int), 1, file);

    // Fill the rest of the metadata block with empty data
    int buffer1_size = (metadata_blocks * block_size) - (5 * sizeof(int));
    char buffer1[buffer1_size];

    memset(buffer1, 0, sizeof(buffer1));

    fwrite(buffer1, sizeof(char), buffer1_size, file);


    int free_table[NUMBEROFBLOCKS];

    // fat blocks free table blocks and directory blocks are filled
    // do not mark them as free
    // everything else is free initially
    for(int i = 0; i < NUMBEROFBLOCKS; i++){
        if(i < 1 + free_table_blocks + fat_blocks + directory_blocks)
            free_table[i] = 0; 
        else
            free_table[i] = 1;
        fwrite(&free_table[i], sizeof(int), 1, file);
    }


    int fat_table[NUMBEROFBLOCKS];

    for(int i = 0; i < NUMBEROFBLOCKS; i++){
        fat_table[i] = -1;
        fwrite(&fat_table[i], sizeof(int), 1, file);
    }

    // Write the rootDir to the file
    fwrite(&rootDir, sizeof(directory_entry), 1, file);

    // Fill the rest of the directory blocks with empty data
    int buffer2_size = (directory_blocks * block_size) - sizeof(directory_entry);
    char buffer2[buffer2_size];

    memset(buffer2, 0, sizeof(buffer2));
    
    fwrite(buffer2, sizeof(char), buffer2_size, file);

    //Fill the rest of the system file with empty data
    char buffer3[block_size];

    memset(buffer3, 0, sizeof(buffer3));

    for(int i = 0; i < NUMBEROFBLOCKS - fat_blocks - directory_blocks - free_table_blocks - metadata_blocks; i++){
        fwrite(buffer3, sizeof(char), block_size, file);
    }

    fclose(file);

    return 0;
}
 