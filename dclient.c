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


    // Index request: ./dclient -a [Title] [Author] [Year] [Document name]

    char cmd;
    if ( strcmp(argv[1],"-a") == 0 )
    {
        if ( argc != 6 ) // Case we dont have the right number of arguments
        {
            printf("Usage:\n");
            printf("Index request: ./dclient -a [Title] [Author] [Year] [Document name] \n");
            return 1;
        }
       
        pid_t pid = getpid();
        char name[20]; //pipe name 
        sprintf(name, "fifo_client:%d", pid); // the pid is like a unique id for the client very cooooooolllllll 

        int res = mkfifo(name, 0600);
        if (res == -1) {
            perror("Error creating client FIFO too bad...");
            return 1;
        }
        
        int fd_to_server = open(PIPE_TO_SERVER, O_WRONLY); // starting to write on the server fifo in order to the server see us <3
        if (fd_to_server == -1) {
            perror("Error opening server FIFO lmaooo");
            return 1;
        }
       

        FileInfo fileinfo;

        // allocatting memory and copy strings and shit | i changed to strcpy its better guys :D

        fileinfo.id = pid;
        strcpy(fileinfo.title, argv[2]);  
        strcpy(fileinfo.author, argv[3]); 
        fileinfo.year = atoi(argv[4]);    
        strcpy(fileinfo.path, argv[5]);   


        cmd='a';
        //writing the cmd first so that the server knows what the hell the comman is so that it know where to go
        if (write(fd_to_server,&cmd, sizeof(char))==-1)
        {
            printf("\nerror writing in the pipe\n");
            perror("\n --> perror message: ");
            exit(2);
        }

        // writing the fileinfo struct in the server fifo so that the server see's the info and do NASTY stuff with it 
        
        write(fd_to_server, &fileinfo, sizeof(FileInfo)); // read comment below x)
        /*
            ok.. i noticed on your topic (num4) you said it wont work but the
            other solution create a HUGE problem that is processes concorrencies and anothers with the server so we
            need to it like this... and the problem with the pointers i think im solving by just simply putting
            the hole string in the struct!
        */
        


        
        int fd_pipe_to_client = open(name, O_RDONLY); // opening the client fifo to read the response from the server
        if (fd_pipe_to_client == -1) {
            perror("Error opening client FIFO hahahahhahah get L monky"); 
            return 1;
        }

        while((res = read(fd_pipe_to_client, buffer, sizeof(buffer))) > 0) {

            printf("%s\n", buffer);  // for now im assuming the server will send us a string with the response and maybe will... 

        }
        

        // // Later, when its done with the struct, free the memory, but its commented because we may not need it 
        // free(fileinfo.title);
        // free(fileinfo.author);
        // free(fileinfo.path);
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

    else if ( strcmp(argv[1],"-c") == 0 )
    {
        if (argc!=3)
        {
           printf("-d command request a book ID\n");
            exit(1);
        }
        int fd_to_server = open(PIPE_TO_SERVER, O_WRONLY); // starting to write on the server fifo in order to the server see us <3
        if (fd_to_server == -1) {
            perror("Error opening server FIFO lmaooo");
            return 1;
        }

        cmd='c';
        int book_id=atoi(argv[2]);
        if (write(fd_to_server,&cmd, sizeof(char))==-1)
        {
            printf("\nerror writing in the pipe\n");
            perror("\n --> perror message: ");
            exit(2);
        }
        //book id
        if ((write(fd_to_server,&book_id,sizeof(int)))==-1)
        {
            printf("error writing the title");
            perror("\n --> perror message: ");
            exit(2);
        }
        char title[100], auther[100], path[100];
        int title_size, auther_size, year, path_size;

        pid_t pid = getpid(); //the special id
        char name[20]; //pipe name 
        sprintf(name, "fifo_client:%d", pid); // the pid is like a unique id for the client very cooooooolllllll 
        int fd_pipe_to_client = open(name, O_RDONLY); // opening the client fifo to read the response from the server
        if (fd_pipe_to_client == -1) {
            perror("Error opening client FIFO hahahahhahah get fucked monky");
            return 1;
        }

        if ((read(fd_pipe_to_client,&title_size,sizeof(int)))==-1 || (read(fd_pipe_to_client,title,title_size))==-1)
        {
            printf("error reading for the server!!!!\n");
            perror("\n --> perror message: ");
            exit(2);
        }
        if ((read(fd_pipe_to_client,&auther_size,sizeof(int)))==-1 || (read(fd_pipe_to_client,auther,auther_size))==-1)
        {
            printf("error reading for the server!!!!\n");
            perror("\n --> perror message: ");
            exit(2);
        }
        if ((read(fd_pipe_to_client, &year, sizeof(int)))==-1)
        {
            printf("error reading for the server!!!!\n");
            perror("\n --> perror message: ");
            exit(2);
        }
        if ((read(fd_pipe_to_client,&path_size,sizeof(int)))==-1 || (read(fd_pipe_to_client,title,path_size))==-1)
        {
            printf("error reading for the server!!!!\n");
            perror("\n --> perror message: ");
            exit(2);
        }
        printf("the data: \n");
        printf("the Title: %s\n",title);
        printf("the Auther: %s\n",auther);
        printf("the Year: %d\n",year);
        printf("the Path: %s\n",path);
    }

    //-d
    //Submit a request to remove an index
    // $ ./dclient -d 1
    else if ( strcmp(argv[1],"-d") == 0 )
    {
        if (argc!=3)
        {
            printf("-d command request a book ID\n");
            exit(1);
        }
        int fd_to_server = open(PIPE_TO_SERVER, O_WRONLY); // starting to write on the server fifo in order to the server see us <3
        if (fd_to_server == -1) {
            perror("Error opening server FIFO lmaooo");
            return 1;
        }
        cmd='d';
        int book_id=atoi(argv[2]);
        if (write(fd_to_server,&cmd, sizeof(char))==-1)
        {
            printf("\nerror writing in the pipe\n");
            perror("\n --> perror message: ");
            exit(2);
        }
        //book id
        if ((write(fd_to_server,&book_id,sizeof(int)))==-1)
        {
            printf("error writing the title");
            perror("\n --> perror message: ");
            exit(2);
        }

        //the receiving message
        pid_t pid = getpid(); //the special id
        char name[20]; //pipe name 
        sprintf(name, "fifo_client:%d", pid); // the pid is like a unique id for the client very cooooooolllllll 
        int fd_pipe_to_client = open(name, O_RDONLY); // opening the client fifo to read the response from the server

        if (fd_pipe_to_client == -1) {
            perror("Error opening client FIFO hahahahhahah get fucked monky");
            return 1;
        }
        while((read(fd_pipe_to_client, buffer, sizeof(buffer))) > 0) {

            printf("%s\n", buffer);  // here it will send the message saying if the book was removed or not
        }
    }

    //Search number of lines containing a certain keyword
    /*
     input: $ ./dclient -l 1 "Romeo"
     output: 150
    */
    else if ( strcmp(argv[1],"-l") == 0 )
    {
    
        if (argc!=4)
        {
            printf("-l cammand needs the id and a keyword");
            exit(3);
        }
        //sending
        int fd_to_server = open(PIPE_TO_SERVER, O_WRONLY); // starting to write on the server fifo in order to the server see us <3
        if (fd_to_server == -1) {
            perror("Error opening server FIFO lmaooo");
            return 1;
        }
        cmd='l';
        int book_id=atoi(argv[2]);
        int word_size=strlen(argv[3]);
        if (write(fd_to_server,&cmd, sizeof(char))==-1)
        {
            printf("\nerror writing in the pipe\n");
            perror("\n --> perror message: ");
            exit(2);
        }
        //keyword
        if ((write(fd_to_server,&book_id,sizeof(int)))==-1)
        {
            printf("error writing the book_id");
            perror("\n --> perror message: ");
            exit(2);
        }
        if ((write(fd_to_server, &word_size, sizeof(int)))<=0   ||  (write(fd_to_server, argv[3], word_size))<=0)
        {
            printf("error writing the book");
            perror("\n --> perror message: ");
            exit(2);
        }
        
        //the receiving message
        pid_t pid = getpid(); //the special id
        char name[20]; //pipe name 
        sprintf(name, "fifo_client:%d", pid); // the pid is like a unique id for the client very cooooooolllllll 
        int fd_pipe_to_client = open(name, O_RDONLY); // opening the client fifo to read the response from the server

        if (fd_pipe_to_client == -1) {
            perror("Error opening client FIFO hahahahhahah get fucked monky");
            return 1;
        }
        while((read(fd_pipe_to_client, buffer, sizeof(buffer))) > 0) {
            printf("%s\n", buffer);  // here it will send the message saying the number if times it found the keyword
        }
    }

    /*
      Search for a list of document identifiers containing a certain keyword.
      $ ./dclient -s "praia"
      [2, 3, 1438]
      
      Search for a list of document identifiers containing a certain keyword using multiple processes (e.g., 5).
      $ ./dclient -s "praia" 5
      [2, 3, 1438]
    */
    else if ( strcmp(argv[1],"-s") == 0)
    {
        if (argc<3)
        {
         printf("-s command needs the keyword and/with a number\n");
         exit(2);
        }
        int fd_to_server = open(PIPE_TO_SERVER, O_WRONLY); // starting to write on the server fifo in order to the server see us <3
        if (fd_to_server == -1) {
         perror("Error opening server FIFO lmaooo");
         return 1;
        }
        cmd='s';
        if (write(fd_to_server,&cmd, sizeof(char))==-1)
        {
            printf("\nerror writing in the pipe\n");
            perror("\n --> perror message: ");
            exit(2);
        }
        int n;
        int word_len=strlen(argv[2]);
        if (argv[3]==NULL)
        {
         //sending
         n=2; //n helps me, ill explain face to face 
         if ((write(fd_to_server,&n,sizeof(int)))==-1)
         {
             printf("error sending book\n");
             exit(2);
         }
         if ((write(fd_to_server,&word_len,sizeof(int)))==-1 || (write(fd_to_server, argv[2],word_len))==-1)
         {
             printf("error sending book\n");
             exit(2);
         }

         //the receiving message
         pid_t pid = getpid(); //the special id
         char name[20]; //pipe name 
         sprintf(name, "fifo_client:%d", pid); // the pid is like a unique id for the client very cooooooolllllll 
         int fd_pipe_to_client = open(name, O_RDONLY); // opening the client fifo to read the response from the server
 
         if (fd_pipe_to_client == -1) {
             perror("Error opening client FIFO hahahahhahah get fucked monky");
             return 1;
            }
         while((read(fd_pipe_to_client, buffer, sizeof(buffer))) > 0) {
             printf("%s\n", buffer);  // here it will send the list of documents found, need fixing later
            }
        }
        else
        {
          n=3;
          if ((write(fd_to_server,&n,sizeof(int)))==-1)
            {
              printf("error sending book\n");
              exit(2);
            }

          if ((write(fd_to_server,&word_len,sizeof(int)))==-1 || (write(fd_to_server, argv[2],word_len))==-1)
            {
             printf("error sending book\n");
             exit(2);
            }
          int n=atoi(argv[3]); //the number of children
          if ((write(fd_to_server,&n,sizeof(int)))==-1)
            {
             printf("\nerror writing in the pipe\n");
             perror("\n --> perror message: ");
             exit(2);
            }
          //the receiving message
         pid_t pid = getpid(); //the special id
         char name[20]; //pipe name 
         sprintf(name, "fifo_client:%d", pid); // the pid is like a unique id for the client very cooooooolllllll 
         int fd_pipe_to_client = open(name, O_RDONLY); // opening the client fifo to read the response from the server
 
         if (fd_pipe_to_client == -1) {
             perror("Error opening client FIFO hahahahhahah get fucked monky");
             return 1;
            }
         while((read(fd_pipe_to_client, buffer, sizeof(buffer))) > 0) {
             printf("%s\n", buffer);  // here it will send the list of documents found, need fixing later
            }
        }
    }
    

    else if ((strcmp(argv[1],"-f"))==0)
    {
        printf("you enterd '-f' \n");
        printf("\nServer is shuting down.....\n");
        exit(3);
    }
    else printf("\nythe command you enterd isnt valid. the hell is wrong with you??? dont you want data?????\n");



    return 0;

}