#include "H_library.h"


int main(int argc, char * argv[]){
    printf("Server started. Waiting for client requests...\n");
    //function to search in the file
    //function to print loading time and a loading percentege 

    //creating a named pipe if the pipe doesnt exist------------------
    
    char name_of_the_file[100];
    while (1)//keeping the server open 
    {
        printf("\nWaiting for client request...\n");
        int recever_fd=open("pipe_to_server",O_RDONLY);
        if (recever_fd==-1)
        {
            printf("\nerror in the receving file");
            perror("\n --> perror message: ");
            return 1;
        }
        
        read(recever_fd, name_of_the_file,sizeof(name_of_the_file));
        //name_of_the_file[sizeof(name_of_the_file) - 1] = '\0';
        close(recever_fd);
        printf("\nthe name of the file was receved.");
        printf("\nthe name of the fiel: %s\n", name_of_the_file);
        printf("\n searching in files............\n");
        sleep(6);
        //loading();
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        search_file(name_of_the_file);
        break;
    }
    



    return 0;
}