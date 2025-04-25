#include "H_library.h"


int main(int argc, char * argv[]) {

    int res;
    printf("Server started!\n");

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
    while ( server_open && ((res = read(fd_to_server,&fileinfo,sizeof(FileInfo))) > 0) ) {
        
        char pipe_name[20]; 
        
        sprintf(pipe_name, "fifo_client:%d", fileinfo.id);
        
        int fd_to_client = open(pipe_name, O_WRONLY);
        if (fd_to_client == -1) {
            perror("Error opening client FIFO");
        }

        if (fileinfo.op[1] == 'a') {

            printf("Indexing document: %s\n", fileinfo.title);
            // Here we would call the function to index the document
            // index_document(fileinfo.title, fileinfo.author, fileinfo.year, fileinfo.path);
            // For now, we just send a success message back to the client
            char msg[] = "Document indexed successfully";
            write(fd_to_client, msg, sizeof(msg));



        } else if (fileinfo.op[1] == 'c') {

            printf("Consulting document with ID: %d\n", fileinfo.id);
            // Here we would call the function to consult the document
            // consult_document(fileinfo.id);


        } else if (fileinfo.op[1] == 'd') {

            printf("Removing document with ID: %d\n", fileinfo.id);
            // Here we would call the function to remove the document
            // remove_document(fileinfo.id);


        } else if (fileinfo.op[1] == 'l') {

            printf("Searching for keyword in document with ID: %d\n", fileinfo.id);
            // Here we would call the function to search for a keyword in the document
            // search_keyword_in_document(fileinfo.id, fileinfo.keyword);

        } else if (fileinfo.op[1] == 's') {
            
            printf("Searching for keyword: %s\n", fileinfo.keyword);
            
            if (fileinfo.op[2] == '1') {

                printf("Searching with processes...\n");
                // Here we would call the function to search for a keyword with processes
                // search_keyword_with_processes(fileinfo.keyword, fileinfo.id);
                
            } else {
                
                printf("Searching without processes...\n");
                // Here we would call the function to search for a keyword without processes
                // search_keyword_without_processes(fileinfo.keyword);
            }


        } else if (fileinfo.op[1] == 'f') {

            printf("Shutting down server...\n");
            server_open = 0;
            
        }

            

            
         
         
    
    



    
    }

    //save part will be here

    return 0;
}