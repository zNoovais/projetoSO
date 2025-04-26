#include "H_library.h"


int main(int argc, char * argv[]) {

    
    Linked* cache[CACHE_SIZE]; //creating the cache
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i] = NULL;
    }

    int next_id = 1;
    int number_of_files = 0;

    
    printf("Server started!\n");

    int fd_file = open(storage_path, O_CREAT | O_RDWR, 0600); // open the file to store the documents ezz
    if (fd_file == -1) {
        perror("Error opening storage file");
        return 1;
    }

    indexed_file file;

    ssize_t bytes_read;
    while ((bytes_read = read(fd_file, &file, sizeof(file))) > 0) { // checking the number of files and the next id
        if (bytes_read == -1) {
            perror("Error reading from storage file");
            close(fd_file);
            return 1;
        }
        number_of_files++;
        if (file.id >= next_id) {
            next_id = file.id + 1;
        }
    }
    
    
    lseek(fd_file, 0, SEEK_SET); // going back to the start of the file :)


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


    open("pipe_to_server",O_WRONLY); // this is to keep the pipe open (the server will not be writting in it) (check guide 5)

    int server_open = 1;
    //while ( server_open && ((read(fd_to_server,&fileinfo,sizeof(FileInfo))) >0) ) {
    while ( server_open ) {
        ssize_t bytes = read(fd_to_server,&fileinfo,sizeof(FileInfo)); 
      //^^^^^^ im using ssize_t type because read returns size_t by defalt so i use size_t because if i use int itll convert it to an int 
        /*
        so read() returns the number of bytes read.
        so it can be:
        --> it can be 0 (meaning EOF — the other end closed)
        --> it can be > 0 (number of bytes read successfully)
        --> it can be -1 (meaning an error happened)
        also i cant use size_t because it only has positive values or 0, and i need to check if its -1
        in other words:
            size_t --> used when size cannot be negative
            ssize_t --> used when you need to represent either positive size or a negative value 
        */
        
        //this loop keeps the server open as long as the server_open!=0
        if (bytes == 0) {
            // no more writers (clients), so re-open the fifo
            close(fd_to_server);
            fd_to_server = open(PIPE_TO_SERVER, O_RDONLY);
            if (fd_to_server == -1) {
                perror("Error reopening server FIFO");
                break;
            }
            continue; // try reading again
        } else if (bytes == -1) {
            perror("Error reading from server FIFO");
            break;
        }


        char pipe_name[20]; 
        
        sprintf(pipe_name, "fifo_client:%d", fileinfo.id);
        
        int fd_to_client = open(pipe_name, O_WRONLY);// the pipe of a specific client
        if (fd_to_client == -1) {
            perror("Error opening client FIFO");
        }

        if (fileinfo.cmd[1] == 'a') { //a [Title] [Author] [Year] [Document name]

            printf("Indexing document: %s\n", fileinfo.title);
            
            Linked* new_file = malloc(sizeof(Linked));  

            new_file->file.id = next_id;
            strcpy(new_file->file.title, fileinfo.title);
            strcpy(new_file->file.author, fileinfo.author);
            new_file->file.year = fileinfo.year;
            strcpy(new_file->file.path, fileinfo.path);

            //zeeeeeee code 
            new_file->next = cache[hash(next_id)]; // hash function to get the index in the cache
            cache[hash(next_id)] = new_file; // adding the new file to the cache :DD

            //////
            /*
            
            fabios fanctions

            */
           ///////


            char msg[] = "Document indexed successfully";
            //example:: 10
            int ex=10;
            write(fd_to_client, msg, sizeof(msg));
            write(fd_to_client,&ex,sizeof(int));
            printf("\n");
            close(fd_to_client);



        } else if (fileinfo.cmd[1] == 'c') {

            printf("Consulting document with ID: %d\n", fileinfo.id);
            
            Linked* curr = cache[hash(fileinfo.id)];
            while (curr != NULL) {                  // traversing the cache on the hash index !!

                if (curr->file.id == fileinfo.id) {
                    
                    char msg[256];

                    sprintf(msg,"Title: %s\nAuthors: %s\nYear: %d\nPath: %s", curr->file.title, curr->file.author, curr->file.year, curr->file.path);
                    
                    write(fd_to_client, msg, sizeof(msg));
                    printf("Document with ID %d found in cache.\n", fileinfo.id);

                    break;
                }
                curr = curr->next;
            }

            if (curr == NULL) {
                printf("Document with ID %d not found in cache.\n", fileinfo.id);
                printf("Searching in storage...\n");
                lseek(fd_file, 0, SEEK_SET); // going back to the start of the file
            }
            close(fd_to_client);


        } 
        
        else if (fileinfo.cmd[1] == 'd') {

            printf("Removing document with ID: %d\n", fileinfo.id);
            // Here we would call the function to remove the document
            // remove_document(fileinfo.id);

            Linked* curr = cache[hash(fileinfo.id)];
            Linked* prev = cache[hash(fileinfo.id)];

            if (curr == NULL) {
                printf("Document with ID %d not found in cache.\n", fileinfo.id);
            }

            else if (curr->file.id == fileinfo.id) {
                cache[hash(fileinfo.id)] = curr->next; // removing the first element
                free(curr);
                printf("Document with ID %d removed from cache.\n", fileinfo.id);
            } 
            else {
                prev = curr; // setting the previous element to the first oneeee
                curr = curr->next; // moving to the next element

                while (curr != NULL) {                  // traversing the cache on the hash index !!
                    if (curr->file.id == fileinfo.id) {
                        prev->next = curr->next; // removing the element
                        free(curr);
                        printf("Document with ID %d removed from cache.\n", fileinfo.id);
                        break;
                    }
                    prev = curr;
                    curr = curr->next;
                }

                if (curr == NULL) {
                    printf("Document with ID %d not found in cache.\n", fileinfo.id);
                }

            }
            close(fd_to_client);


            // Here we would call the function to remove the document from the storage file
            // remove_document_from_storage(fileinfo.id);     
        }

        else if (fileinfo.cmd[1] == 'l') { // ./dclient -l [n] [keyword]

            

            printf("Searching for keyword in document with ID: %d\n", fileinfo.id);
            // Here we would call the function to search for a keyword in the document
            // search_keyword_in_document(fileinfo.id, fileinfo.keyword);

            Linked* curr = cache[hash(fileinfo.id)];
            while (curr != NULL) {                  // traversing the cache on the hash index !!
                if (curr->file.id == fileinfo.id) {
                    printf("Document with ID %d found in cache.\n", fileinfo.id);
                    break;
                }
                curr = curr->next;
            }
            if (curr == NULL) {
                printf("Document with ID %d not found in cache.\n", fileinfo.id);
                printf("Searching in storage...\n");
                lseek(fd_file, 0, SEEK_SET); // going back to the start of the file
            }
            close(fd_to_client);

                //after i find in the storage i want to add it to the cache
                // and then i want to search for the keyword in the document

            // Here we would call the function to search for a keyword in the document
            // search_keyword_in_document(fileinfo.id, fileinfo.keyword);

        } 
        ////======================== fix ====================================================
        else if (fileinfo.cmd[1] == 's') {  // this function its tricky because we have to search in the cache and in the storage
            
            printf("Searching for keyword: %s\n", fileinfo.word);
            
            if (fileinfo.cmd[2] == '1') {

                printf("Searching with processes...\n");
                // Here we would call the function to search for a keyword with processes
                // search_keyword_with_processes(fileinfo.keyword, fileinfo.id);
                
            } else {
                
                printf("Searching without processes...\n");
                // Here we would call the function to search for a keyword without processes
                // search_keyword_without_processes(fileinfo.keyword);
            }
            close(fd_to_client);



        } else if (fileinfo.cmd[1] == 'f') {

            printf("Shutting down server...\n");
            server_open = 0;
            //save before killing 

            // Here we have to rewrite the storage file with the new data
            
        }

            

            
         
         
    
    



    
    }

    //save part will be here

    return 0;
}