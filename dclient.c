#include "H_library.h"

int main(int argc, char* argv[]) { // i changed to this version just keep it simple and for now on we have to start commenting code... :D
    if ( argc < 2 )
    {
        printf("Usage:\n");
        printf("Index request: ./dclient -a [Title] [Author] [Year] [Document name] \n");
        printf("Consult document: ./dclient -c [n]\n");
        printf("Remove request: ./dclient -d [n]\n");
        printf("Search number of lines of keyword: ./dclient -l [n] [keyword]\n");
        printf("Search keyword: ./dclient -s [keyword]\n");
        printf("Search keyword w/ processes: ./dclient -s [number of processes] [keyword]\n");
        printf("Shutdown: ./dclient -f\n");
        return 1;
    }

    char buffer[50]="";



    if ( strcmp(argv[1],"-a") == 0 )
    {
        if ( argc != 6 ) // Case we dont have the right number of arguments
        {
            printf("Usage:\n");
            printf("Index request: ./dclient -a [Title] [Author] [Year] [Document name] \n");
            return 1;
        }
       
        pid_t pid = getpid();
        char name[20];
        sprintf(name, "fifo_client:%d", pid); // the pid is like a unique id for the client very cooooooolllllll 

        int res = mkfifo(name, 0600);
        if (res == -1) {
            perror("Error creating client FIFO too bad...");
            return 1;
        }
        
        int fdserver = open(SERVER, O_WRONLY); // starting to write on the server fifo in order to the server see us <3
        if (fdserver == -1) {
            perror("Error opening server FIFO lmaooo");
            return 1;
        }
        
        FileInfo fileinfo;

        fileinfo.id = pid;
        fileinfo.title = argv[2];
        fileinfo.author = argv[3];
        fileinfo.year = atoi(argv[4]);
        fileinfo.path = argv[5];

        write(fdserver, &fileinfo, sizeof(FileInfo)); // writing the fileinfo struct in the server fifo to the server see this and do NASTY stuff with it 
        
        int fdclient = open(name, O_RDONLY); // opening the client fifo to read the response from the server
        if (fdclient == -1) {
            perror("Error opening client FIFO..");
            return 1;
        }

        while((res = read(fdclient, buffer, sizeof(buffer))) > 0) {

            printf("%s\n", buffer);  // for now im assuming the server will send us a string with the response and maybe will... 

        }
        

        
    }

    if ( strcmp(argv[1],"-c") == 0 )
    {
        // TODO
    }

    if ( strcmp(argv[1],"-d") == 0 )
    {
        // TODO
    }

    if ( strcmp(argv[1],"-l") == 0 )
    {
        // TO DO
    }

    if ( strcmp(argv[1],"-s") == 0)
    {
        // TO DO
    }

    return 0;

}