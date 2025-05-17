#include "H_library.h"




int main(int argc, char * argv[]) {


    cacheLines cache[CACHE_LINES]; //creating the cache

    for (int i = 0; i < CACHE_LINES; i++) {
        cache[i].list = NULL;
        cache[i].occupation = 0;
    }

    int deleted = 0;
    int cache_occupation = 0;
    int virtual_time = 0;
    int next_id = 0;
    int number_of_files = 0;
    char msg[520];
   
    printf("Server started!\n");

    int fd_file_write = open(storage_path, O_CREAT | O_APPEND | O_RDWR, 0600);
    if (fd_file_write == -1) {
        perror("Error opening storage file to write");
        return 1;
    } 

    int fd_file_read = open(storage_path, O_RDWR, 0600); // open the file to store the documents ezz
    if (fd_file_read == -1) {
        perror("Error opening storage file");
        return 1;
    }

    indexed_file file;

    ssize_t bytes_read;
    while ((bytes_read = read(fd_file_read, &file, sizeof(file))) > 0) { // checking the number of files and the next id // this is one calculation in the storage
        if (bytes_read == -1) {
            perror("Error reading from storage file");
            close(fd_file_read);
            return 1;
        }
        
        number_of_files++;
        if (file.active && file.id >= next_id) {
            next_id = file.id + 1;
        }
    }
    
    
    lseek(fd_file_read, 0, SEEK_SET); // going back to the start of the file :)


    if ((mkfifo(PIPE_TO_SERVER,0600))==-1) //pipe to server
    {
        perror("\nerror creating the pipe: ");
    }
    
    
    int fd_to_server = open(PIPE_TO_SERVER, O_RDONLY);
    if (fd_to_server == -1) {
        perror("Error opening server FIFO");
        return 1;
    }

    FileInfo fileinfo;


    open(PIPE_TO_SERVER,O_WRONLY); // this is to keep the pipe open (the server will not be writting in it) (check guide 5)

    int res;
    int server_open = 1;
    while ( server_open && (res = read(fd_to_server,&fileinfo,sizeof(FileInfo))) > 0) {
       
        
        char pipe_name[20]; 
        
        printf("Command received: %d\n", fileinfo.id);
        printf("Commamd %s\n",fileinfo.cmd);

        sprintf(pipe_name, "fifo_client:%d", fileinfo.id);
        
        int fd_to_client = open(pipe_name, O_WRONLY);// the pipe of a specific client
        if (fd_to_client == -1) {
            perror("Error opening client FIFO");
        }

        if (fileinfo.cmd[1] == 'a') { //a [Title] [Author] [Year] [Document name]

            printf("Indexing document: %s\n", fileinfo.title);
            
          
            indexed_file file_struct;

            number_of_files++;
            
            virtual_time++;
                                     

            //filling the struct to put on the storage
            file_struct.active = 1;
            file_struct.id = next_id;
            strcpy(file_struct.title, fileinfo.title);
            strcpy(file_struct.author, fileinfo.author);
            file_struct.year = fileinfo.year;
            strcpy(file_struct.path, fileinfo.path);
            

            write(fd_file_write,&file_struct,sizeof(indexed_file));


            sprintf(msg,"%d\n",next_id);

            next_id++;

            
            // write(fd_to_client, msg, sizeof(msg));
            write(fd_to_client, msg, strlen(msg));


            close(fd_to_client);

        } 
        
        else if (fileinfo.cmd[1] == 'c') {

            printf("-C Consulting document with ID: %d\n", fileinfo.index);
            virtual_time++;
            
            Linked* curr = cache[hash(fileinfo.index)].list;
            while (curr != NULL) {                  // traversing the cache on the hash index !!
                
                if (curr->file.id == fileinfo.index) {
                    curr->last_accessed = virtual_time; // last time used
                    sprintf(msg,"Title: %s\nAuthors: %s\nYear: %d\nPath: %s\n", curr->file.title, curr->file.author, curr->file.year, curr->file.path);
                    break;  
                }
                curr = curr->next;
                
            }


            if (curr == NULL) { // didnt find it in the cache so its goint thru the storage.
                
                printf("Document with ID %d not found in cache.\n", fileinfo.index);
                printf("Searching in storage...\n");
                lseek(fd_file_read, 0, SEEK_SET); // going back to the start of the file
                
                indexed_file file_struct;

                while((res = read(fd_file_read,&file_struct,sizeof(indexed_file))) > 0) {
                    
                    if(file_struct.active && file_struct.id == fileinfo.index) {

                        break;
                    }
                    
                }
                printf("Read result: %d\n", res);

                if (res == 0) {
                    sprintf(msg,"didnt find nothing...\n");
                }

                else {
                    sprintf(msg,"Title: %s\nAuthors: %s\nYear: %d\nPath: %s\n", file_struct.title ,file_struct.author, file_struct.year, file_struct.path);
                    

                   

                    Linked* new_file = malloc(sizeof(Linked));  
                    
                    cache_occupation++;
                    new_file->last_accessed = virtual_time;
                    new_file->file.id = file_struct.id;
                    strcpy(new_file->file.title, file_struct.title);
                    strcpy(new_file->file.author, file_struct.author);
                    new_file->file.year = file_struct.year;
                    strcpy(new_file->file.path, file_struct.path);

                    if (cache[hash(file_struct.id)].occupation >= MAX_LINE_SIZE) {
                        remove_least_used(cache[hash(file_struct.id)].list, virtual_time);
                    }

                    new_file->next = cache[hash(file_struct.id)].list; // hash function to get the index in the cache

                     

                    cache[hash(file_struct.id)].list = new_file; // adding the new file to the cache :DD
                    cache[hash(file_struct.id)].occupation++;
                    

                }
            }
            
                    
                    write(fd_to_client, msg, strlen(msg));
                    close(fd_to_client);


        } 
        
        else if (fileinfo.cmd[1] == 'd') {

            printf("Removing document with ID: %d\n", fileinfo.index);
            virtual_time++;

            Linked* curr = cache[hash(fileinfo.index)].list;
            Linked* prev = cache[hash(fileinfo.index)].list;

            if (curr == NULL) {
                printf("line have nothing!\n");
            }

            else if (curr->file.id == fileinfo.index) {
                cache[hash(fileinfo.index)].list = curr->next; // removing the first element
                free(curr);
                printf("Document with ID %d removed from cache.\n", fileinfo.index);
                cache[hash(fileinfo.index)].occupation--;
                cache_occupation--;
                
            } 
            else {
                prev = curr; // setting the previous element to the first oneeee
                curr = curr->next; // moving to the next element

                while (curr != NULL) {                  // traversing the cache on the hash index !!
                    if (curr->file.id == fileinfo.index) {
                        prev->next = curr->next; // removing the element
                        free(curr);
                        printf("Document with ID %d removed from cache.\n", fileinfo.index);
                        cache_occupation--;
                        break;
                    }
                    prev = curr;
                    curr = curr->next;
                }
            }

            if (curr == NULL) {
                printf("Document with ID %d not found in cache.\n", fileinfo.index);
            }

            printf("Searching in the storage...");

            lseek(fd_file_read, 0, SEEK_SET); // going back to the start of the file
                
            indexed_file file_struct;

            while((res = read(fd_file_read,&file_struct,sizeof(indexed_file))) > 0) {
                    
                if(file_struct.active && file_struct.id == fileinfo.index) {
                    break;
                }
                    
            }

            if (res == 0) {
                printf("nothing on storage too");
                sprintf(msg,"didnt find nothing on the storage and cache\n");

            }
            else {
                number_of_files--;
                file_struct.active = 0;
                lseek(fd_file_read,-sizeof(indexed_file),SEEK_CUR); //going back one space to rewrite the struct :D
                write(fd_file_read,&file_struct,sizeof(indexed_file)); //Rewriting the struct!!!
                sprintf(msg,"document found in storage\n");
                
                }
            

            // write(fd_to_client, msg, sizeof(msg)); 
            write(fd_to_client, msg, strlen(msg)); 
            close(fd_to_client); 
        }

        else if (fileinfo.cmd[1] == 'l') { // ./dclient -l [n] [keyword] 

            virtual_time++;
            printf("Searching for keyword in document with ID: %d\n", fileinfo.id); 
            printf("Keyword: %s\n", fileinfo.word); 
            search_keyword(cache, fileinfo.word, fileinfo.index, fd_file_read, msg, &cache_occupation, virtual_time); // this was a experiment to this all in a function......
            // write(fd_to_client, msg, sizeof(msg));
            write(fd_to_client, msg, strlen(msg));

            close(fd_to_client);
        } 
        
        else if (fileinfo.cmd[1] == 's') {  // this function its tricky because we have to search in the cache and in the storage

        
            printf("Searching for keyword: \"%s\"\n", fileinfo.word);

            int msg_size = 8192;

            char msg[8192] = "";

            if (fileinfo.processes > 1) {
                search_contains_word(fileinfo.word, fileinfo.processes, number_of_files, fd_file_read, deleted, msg, msg_size);
            } else {
                search_contains_word(fileinfo.word, 1, number_of_files, fd_file_read, deleted, msg, msg_size);
            }
            
            write(fd_to_client, msg, sizeof(msg));
            close(fd_to_client);
            
        }  
        
        else if (fileinfo.cmd[1] == 'f') {

            printf("Shutting down server...\n");
            server_open = 0;

            
            indexed_file file_struct;

            if (deleted != 0) { // this a quick optimazation 
                lseek(fd_file_read, 0, SEEK_SET);
                

                int fd_new = open("temp",O_CREAT | O_TRUNC | O_WRONLY, 0600);

                while ((res = read(fd_file_read, &file_struct, sizeof(indexed_file))) > 0) {
                    if (res == -1) {
                        perror("Error reading from storage file");
                        close(fd_file_read);
                        close(fd_file_write); 
                        return 1;
                    }

                    if (file_struct.active) {
                        // Write the active file to the new file
                        if (write(fd_new, &file_struct, sizeof(indexed_file)) == -1) {
                            perror("Error writing to new storage file");
                            close(fd_new);
                            close(fd_file_read);
                            close(fd_file_write); 
                            return 1;
                        }
                        
                    }
                }

                rename("temp", storage_path); //closing everything dear god 
                
                ftruncate(fd_file_write, 0); //
                close(fd_new);
            
            }

            // cleaning the cache
            for (int i = 0; i < CACHE_LINES; i++) {
                freeL(cache[i].list);
            }
            
            unlink(PIPE_TO_SERVER);
            close(fd_to_server); 
            close(fd_file_read);
            close(fd_file_write);
             
            
            
            printf("Server shut down successfully.\n");
            return 0;

        }
    
    
    
    }

    
    close(fd_to_server);
    return 0;
}