#include "operations.h"

int is_directories_changed = 0;
int is_tables_changed = 0;

char** tokenize_path(char* path, int* count) {
   
    if(path[0] != '/'){

        printf("Paths should current_block with root directory '/'. Exiting...\n");
        exit(EXIT_FAILURE);
    } 
    
    // Count the number of segments
    int segments = 0;
    for (char* p = path; *p; ++p) {
        if (*p == '/') {
            ++segments;
        }
    }

    char** result = malloc((segments + 3) * sizeof(char*));
    if (!result) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    result[0]= strdup("/");

    // Use strtok to split the path
    int index = 1;
    char* token = strtok(path, "/");
    while (token) {
        result[index++] = strdup(token);
        token = strtok(NULL, "/");
    }
    result[index] = NULL; // NULL-terminate the array

    // Set the count of segments
    *count = index;

    return result;
}

void dir(const int file_index, const directory_entry directory_entries[]){

    //Look for directories whose parent is this folder in the directory entries
    for(int i = 0; i < MAXNUMBEROFFILE; ++i){
        if(directory_entries[i].isExist && directory_entries[i].parent_index == file_index){
            printf("%s last modified: %s", directory_entries[i].filename,  ctime(&directory_entries[i].last_modification));
            if(strcmp(directory_entries[i].password, "NOPASSWORD") != 0) printf("is protected ");
            int permission = directory_entries[i].permission;
            if(permission == READWRITE) printf("rw\n");
            else if(permission == WRITE) printf("w\n");
            else if(permission == READ) printf("r\n");
            else printf("no permission");
        }
    }
    
    printf("\n");
}

void mkdir(const char* filename, const int* file_indices, const int file_count, directory_entry directory_entries[]){
    
    int index = find_file(filename, file_indices, file_count, directory_entries);

    if(index != -1){
        printf("The file is already exists in the given path\n");
        return;
    }

    for(int i = 0; i < MAXNUMBEROFFILE; ++i){
        if(directory_entries[i].isExist != 1){
            time_t current_time;
            time(&current_time);
            init_directory_entry(&directory_entries[i], filename, file_indices[file_count - 1], current_time, 0, 0, 1);
            is_directories_changed = 1;
            return;
        }
    }
}


// this function is for finding indices of the files in the user given file path
int* find_file_indices(char** file_path, const int file_count, const directory_entry directory_entries[]){

    int* indices = (malloc)((file_count) * sizeof(int));

    for(int i = 0; i < MAXNUMBEROFFILE; ++i){
         
        //check for last file in the path first
        int file_index = file_count - 1;

        if(directory_entries[i].isExist && strcmp(directory_entries[i].filename, file_path[file_index]) ==  0){

            indices[file_index] = i;
            directory_entry my_directory_entry = directory_entries[i];
            
            --file_index;

            //if it has reached the root directory return
            if(file_index == -1) return indices;

            while(file_index > -1 && 
            (strcmp(directory_entries[my_directory_entry.parent_index].filename, file_path[file_index]) == 0)){
                
                //look for next file in the path
                indices[file_index] = my_directory_entry.parent_index;
                my_directory_entry = directory_entries[my_directory_entry.parent_index];
                --file_index;

                if(my_directory_entry.parent_index == -1) return indices;

            }    

        }

    }

    //if it cannot reach the root all the way from the bottom
    //then it is a false path
    printf("False filepath. Exiting...\n");
    exit(EXIT_FAILURE);
}


//checking for individual file 
//for some operations it is okey for the last file not to exist
//but it is still needed to be known whether the file exists
int find_file(const char* filename, const int* file_indices, const int file_count, directory_entry directory_entries[]){

    for(int i = 0; i < MAXNUMBEROFFILE; ++i){
        
        int file_index = file_count - 1;

        if(directory_entries[i].isExist && strcmp(directory_entries[i].filename, filename) ==  0){

            directory_entry my_directory_entry = directory_entries[i];

            int the_index = i;

            --file_index;
            while (file_index > -1)
            {
                if(my_directory_entry.parent_index != file_indices[file_index]) break;
                
                my_directory_entry = directory_entries[my_directory_entry.parent_index];
                --file_index;
            }

            if (file_index <= 0) return the_index;
        }
    }

    return -1;

}


void write_file(const char* source_file, const char* filename, const char* system_file, const int* file_indices, 
const int file_count, int filled_blocks, int block_size, directory_entry directory_entries[], 
int fat_table[], int free_table[], const char* password){
    
    int index = find_file(filename, file_indices, file_count, directory_entries);

    char current_password[128] = "NOPASSWORD";

    if(index != -1){

        if(directory_entries[index].isDirectory){
            printf("The file is a directory\n");
            return;

        }
        else if(strcmp(directory_entries[index].password, "NOPASSWORD") != 0 && 
        strcmp(directory_entries[index].password, password) != 0){
            printf("False password\n");
            return;

        }
        else if(check_permission(directory_entries[index], 1)){
            del(index, directory_entries, fat_table, free_table, password);
            strcpy(current_password, directory_entries[index].password);
        }
        else {
            printf("The file has no write permission\n");
            return;
        }

    }

   
    FILE* file = fopen(source_file, "rb");
    if(file == NULL){
        perror("fopen");
        exit(-1);
    }

        
    fseek(file, 0, SEEK_END);
    int size = ftell(file);

    rewind(file);

    int file_blocks = (size / block_size) + 1;

    //fill the source file into a char array
    char** fileArray = (char**) malloc(file_blocks * sizeof(char*));

    for(int i = 0; i < file_blocks; ++i){
        fileArray[i] = (char*) malloc(block_size * sizeof(char));
        fread(fileArray[i], sizeof(char), block_size, file);
    }

    fclose(file);

    FILE* file2 = fopen(system_file, "rb+");
    if(file2 == NULL){
        perror("fopen");
        exit(-1);
    }


    //write the file into blocks
    int next_block = -1;
    for(int i = 0, j = 0; i < NUMBEROFBLOCKS && j < file_blocks; ++i){
        if(free_table[i] == 1){
            fat_table[i] = next_block;
            next_block = i;
            free_table[i] = 0;
            fseek(file2, i * block_size, SEEK_SET);
            fwrite(fileArray[file_blocks - j - 1], sizeof(char), block_size, file2);
            rewind(file2);
            ++j;
        }
    }
        
    fclose(file2);

    //create the file
    for(int i = 0; i < MAXNUMBEROFFILE; ++i){
        if(directory_entries[i].isExist != 1){
            time_t current_time;
            time(&current_time);
            init_directory_entry(&directory_entries[i], filename, file_indices[file_count - 2], current_time, size, next_block, 0);
            strcpy(directory_entries[i].password, current_password);
            break;
        }
    }
        
    is_tables_changed = 1;
    is_directories_changed = 1;

}


void read_file(const char* destination_file, const char* system_file, 
const int file_index, int block_size, directory_entry directory_entries[], int fat_table[], const char* password){

    directory_entry source_file = directory_entries[file_index];

    if(!check_permission(source_file, 0)){
        printf("The file has no read permission\n");
        return;
    }

    if(source_file.isDirectory){
        printf("The file to be read is a directory\n");
        return;
    }

    if(strcmp(source_file.password, "NOPASSWORD") != 0 &&
    strcmp(source_file.password, password) != 0){
        printf("False password\n");
        return;
    }

    FILE* file = fopen(destination_file, "w");
    FILE* fsystem = fopen(system_file, "rb+");

    //advance through blocks and read them
    int read_block = source_file.first_block;

    while(read_block != -1){   
        char buffer[block_size];
        fseek(fsystem, read_block * block_size, SEEK_SET);
        fread(buffer, sizeof(char), block_size, fsystem);
        fwrite(buffer, sizeof(char), strlen(buffer), file);
        memset(buffer, 0, sizeof(buffer));
        rewind(fsystem);
        read_block = fat_table[read_block];
    }
    
    fclose(file);
    fclose(fsystem);
}


void dumpe2fs(const int block_size, const int free_table[], const int fat_table[],
const directory_entry directory_entries[]){
    

    printf("Block Count: %d\n", NUMBEROFBLOCKS);
    printf("Block Size: %d\n", block_size);

    int number_of_freeblocks= 0;
    for(int i = 0; i < NUMBEROFBLOCKS; ++i){
        number_of_freeblocks += free_table[i];
    }
    printf("Number of free blocks: %d\n", number_of_freeblocks);

    int number_of_files = 0, number_of_directories = 0;
    for(int i = 0; i < MAXNUMBEROFFILE; ++i){
        if(directory_entries[i].isExist){
            if(directory_entries[i].isDirectory){ ++number_of_directories; }
            else {++number_of_files;}
        }
    }
    
    printf("Number of files: %d\n", number_of_files);
    printf("Number of directories: %d\n", number_of_directories);


    for(int i = 0; i < MAXNUMBEROFFILE; ++i){
        if(directory_entries[i].isExist && !directory_entries[i].isDirectory){
            printf("Occupied blocks for %s: ", directory_entries[i].filename);
            int current_block = directory_entries[i].first_block;
            while(current_block != -1){
                printf("%d ", current_block);
                current_block = fat_table[current_block];
            }
            printf("\n");
        }
    }

}


void del(const int file_index, directory_entry directory_entries[], int fat_table[], int free_table[], const char* password){

    if(directory_entries[file_index].isDirectory) {
        printf("%s is a directory\n", directory_entries[file_index].filename);
        return;
    }

    if(strcmp(directory_entries[file_index].password, "NOPASSWORD") != 0 && 
    strcmp(directory_entries[file_index].password, password) != 0){
        printf("False password\n");
        return;
    }
   
    int block = directory_entries[file_index].first_block; 
    int next_block;

    do{
        next_block = fat_table[block]; 
        free_table[block] = 1;
        fat_table[block] = -1;
        block = next_block; 
    }while(block != -1);

    directory_entries[file_index].isExist = 0;

    is_directories_changed = 1;
    is_tables_changed = 1;

}


void rmdirectory(const int file_index, directory_entry directory_entries[]){

    if(!directory_entries[file_index].isDirectory){
        printf("%s is not a directory\n", directory_entries[file_index].filename);
        return;
    }

    for(int i = 0; i < MAXNUMBEROFFILE; ++i){
        if(directory_entries[i].isExist && directory_entries[i].parent_index == file_index){
            printf("%s is not an empty directory\n", directory_entries[file_index].filename);
            return;
        }
    }

    directory_entries[file_index].isExist = 0;
    is_directories_changed = 1;
}


int check_permission(directory_entry my_direcotry_entry, int isWrite){

    if( (isWrite && (my_direcotry_entry.permission == WRITE || my_direcotry_entry.permission == READWRITE )) 
    || (!isWrite && (my_direcotry_entry.permission == READ || my_direcotry_entry.permission == READWRITE )))
    return 1;
    else return 0;
}


void change_permission(const int file_index, directory_entry directory_entries[], const char* new_permission,
const char* password){


    if(strcmp(directory_entries[file_index].password, "NOPASSWORD") != 0 
    && strcmp(directory_entries[file_index].password, password) != 0){
        printf("False password\n");
        return;
    }

    int permission = directory_entries[file_index].permission;

    if(strcmp(new_permission, "+r") == 0){
        if(permission % 2 == 0)permission += READ;
    }else if(strcmp(new_permission, "+w") == 0){
        if(permission < 2)permission += WRITE;
    }else if(strcmp(new_permission, "+rw") == 0) permission = READWRITE;
    else if(strcmp(new_permission, "-r") == 0){
        if(permission % 2 == 1)permission -= READ;
    }else if(strcmp(new_permission, "-w") == 0){
        if(permission > 1)permission -= WRITE;
    }else if(strcmp(new_permission, "-rw") == 0) permission = 0;
    else {
        printf("Unidentified permission request\n");
        return;
    }

    directory_entries[file_index].permission = permission;

    is_directories_changed = 1;
}


void addpw(const int file_index, directory_entry directory_entries[], const char* new_pw){

    if(strcmp(directory_entries[file_index].password, "NOPASSWORD") != 0){
        
        printf("Enter current password: ");

        char input[128];

        if(fgets(input, sizeof(input), stdin) != NULL){

            int len = strlen(input);
            if (len > 0 && input[len-1] == '\n') {
                input[len-1] = '\0';
            }

            if (strcmp(input, directory_entries[file_index].password) != 0){
                printf("False password\n");
                return;
            }
           
        }else{
            printf("Error reading input.\n");
            return;
        }
    }

    strcpy(directory_entries[file_index].password, new_pw);

    is_directories_changed = 1;
}