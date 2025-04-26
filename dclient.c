#include "H_library.h"

int main(int argc, char* argv[]) {
    if (argc < 2)
    {
        printf("Usage:\n");
        printf("Index request: ./dclient -a [Title] [Author] [Year] [Document name] \n");
        printf("Consult document: ./dclient -c [n]\n");
        printf("Remove request: ./dclient -d [n]\n");
        printf("Search number of lines of keyword: ./dclient -l [n] [keyword]\n");
        printf("Search keyword: ./dclient -s [keyword]\n");
        printf("Search keyword w/ processes: ./dclient -s [keyword] [number of processes]\n");
        printf("Shutdown: ./dclient -f\n");
        return 1;
    }

    char buffer[512] = "";
    FileInfo fileinfo; // create a single fileinfo struct to use for all commands, because i dont want to creat the same struct over and over again 
    memset(&fileinfo, 0, sizeof(FileInfo)); // initializing the struct with zeros

    pid_t pid = getpid();
    fileinfo.id = pid; //set
    
    // create client FIFO for receiving responses, im creating the pipe now so that i can send the id to the server
    char client_fifo[30]; //
    sprintf(client_fifo, "fifo_client:%d", pid);
    
    int res = mkfifo(client_fifo, 0600);
    if (res == -1 && errno != EEXIST) { //checking if errno generated because errno is a global value in <errno.h> and i might as well use it 
        perror("error creating client fifo");
        return 1;
    }

    
    int fd_to_server = open(PIPE_TO_SERVER, O_WRONLY); //opening the writing pipe to the server
    if (fd_to_server == -1) {
        perror("Error opening server FIFO");
        unlink(client_fifo);
        return 1;
    }

    //-a cmd
    // ./dclient -a "Romeo and Juliet" "William Shakespeare" "1997" "1112.txt"
    if (strcmp(argv[1], "-a") == 0) {
        
        if (argc != 6) {
            printf("Usage: ./dclient -a [title] [author] [year] [path]\n");
            close(fd_to_server);
            unlink(client_fifo);
            return 1;
        }
        
        
        strcpy(fileinfo.cmd, "-a");
        strcpy(fileinfo.title, argv[2]);
        strcpy(fileinfo.author, argv[3]);
        fileinfo.year = atoi(argv[4]);
        strcpy(fileinfo.path, argv[5]);
        
    } 

    //-c
    //Submit a request the meta-information of a document
    /*
     --input:  $ ./dclient -c 1
     --out:
     Title: Romeo and Juliet
     Authors: William Shakespeare
     Year: 1997
     Path: 1112.txt
    */
    else if (strcmp(argv[1], "-c") == 0) {
        
        if (argc != 3) {
            printf("Usage: ./dclient -c [index_of_document]\n");
            close(fd_to_server);
            unlink(client_fifo);
            return 1;
        }
        
        strcpy(fileinfo.cmd, "-c");
        fileinfo.index = atoi(argv[2]);
        
    } 

    //-d
    //Submit a request to remove an index
    // $ ./dclient -d 1
    else if (strcmp(argv[1], "-d") == 0) {
        
        if (argc != 3) {
            printf("Usage: ./dclient -d [n]\n");
            close(fd_to_server);
            unlink(client_fifo);
            return 1;
        }
        
        strcpy(fileinfo.cmd, "-d");
        fileinfo.index = atoi(argv[2]);
        
    } 

    //-l
    //Search number of lines containing a certain keyword
    /*
     input: $ ./dclient -l 1 "Romeo"
     output: 150
    */
    else if (strcmp(argv[1], "-l") == 0) {
        
        if (argc != 4) {
            printf("Usage: ./dclient -l [index_of_document] [keyword]\n");
            close(fd_to_server);
            unlink(client_fifo);
            return 1;
        }
        
        strcpy(fileinfo.cmd, "-l");
        fileinfo.index = atoi(argv[2]);
        strcpy(fileinfo.word, argv[3]);
        
    } 

    //-s
    /*
      Search for a list of document identifiers containing a certain keyword.
      $ ./dclient -s "praia"
      [2, 3, 1438]
      
      Search for a list of document identifiers containing a certain keyword using multiple processes (e.g., 5).
      $ ./dclient -s "praia" 5
      [2, 3, 1438]
    */
    else if (strcmp(argv[1], "-s") == 0) {
        
        if (argc < 3 || argc > 4) {
            printf("Usage: ./dclient -s [keyword] or ./dclient -s [keyword] [number of processes]\n");
            close(fd_to_server);
            unlink(client_fifo);
            return 1;
        }
        
        strcpy(fileinfo.cmd, "-s");
        strcpy(fileinfo.word, argv[2]);
        
        if (argc == 4) { //instead if having two if statment, im checking with one, because i know its only to casses and im checking if its the secund case
                        // which is this: ./dclient -s [keyword] [number of processes] 
            fileinfo.processes = atoi(argv[3]);
        } 
        else {
            // here's the first case: ./dclient -s [keyword]
            fileinfo.processes = 1;
        }

        
    } 


    //-f 
    //kill 
    else if (strcmp(argv[1], "-f") == 0) {
        
        strcpy(fileinfo.cmd, "f");
        printf("Server is shutting down...\n");
        
    } 
    
    //if its anything else, which isnt valid
    else {
        printf("Invalid command. Please check usage.\n");
        close(fd_to_server);
        unlink(client_fifo);
        return 1;
    }


    // because its a struct so i can just send it once after updating the struct, so i dont need to make a shit tun of write statments on every case / cmd
    if (write(fd_to_server, &fileinfo, sizeof(FileInfo)) == -1) {
        perror("Error writing to server FIFO");
        close(fd_to_server);
        unlink(client_fifo);
        return 1;

    }
    
    close(fd_to_server);


    //==================================================================================================================================
    //==================================================================================================================================


    // if it's the shutdown command, we're done, the server is dead so leave
    if (strcmp(argv[1], "-f") == 0) {
        unlink(client_fifo);
        return 0;
    }
    
    // openning the client fifo to read server's response
    int fd_from_server = open(client_fifo, O_RDONLY); //client_fifo <-- the pipe respon 
    if (fd_from_server == -1) {
        perror("Error opening client FIFO for reading");
        unlink(client_fifo);
        return 1;
    }
    
    //ze i need you to check if this is ok::::

    while (1) {  // reading the response of the server
        res = read(fd_from_server, buffer, sizeof(buffer) - 1);
        if (res <= 0) {
            if (res == -1) {
                perror("error reading server response");
            }
            break; 
        }
    
        buffer[res] = '\0';  // Ensure null termination
        printf("%s", buffer);
    }
    
   
 
    // clean up
    close(fd_from_server);
    unlink(client_fifo);
    
    return 0;
}