#include "H_library.h"

//int argc, char * argv[]
int main() {
    if ((mkfifo("pipe_to_server",0666))==-1)
    {
        if (errno!=EEXIST)
        {
            perror("\nperror messege: ");
            return 1;
        }
        
    }
    
    printf("\nwellcome to group (group name) project");
    printf("\nplease enter the name of the file you are searching for: ");
    char name_of_file[100];
    scanf("%s", name_of_file);
    printf("\nplease conferm the name: %s", name_of_file);
    printf("\nif the name of the file is wrong enter (-1) else press (1): \n");
    int ans;
    scanf("%d", &ans);
    if (ans==-1)
    {
        printf("\nplease re-enter the name\n");
        scanf("%s", name_of_file);
    }
    sleep(1);
    printf("\nconnecting the server............\n");
    sleep(2);
    loading();
    printf("server connected\n");
    // sleep(1);
    // printf("......................\n");
    // sleep(1);
    // printf("......................\n");
    // sleep(1);
    // printf("......................\n");
    // printf("\n");

    int fd=open("pipe_to_server",O_WRONLY); //pipe
    if (fd==-1)
    {
        printf("\nerror in opening the file\n");
        perror("\n --> perror message: ");
        return 1;
    }
    if ((write(fd,name_of_file,sizeof(name_of_file))+1)==-1)// +1 for null terminator
    {
        printf("\nerror in writing in pipe\n");
        perror("\n --> perror message: ");
        return 2;
    }
    close(fd);
    
    

    




    return 0;
}