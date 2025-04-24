#include "H_library.h"


int main(int argc, char * argv[]){
    printf("Server started. waiting for client requests...\n");

    if ((mkfifo("pipe_to_server",0666))==-1) //pipe to server
    {
        if (errno!=EEXIST)
        {
            perror("\nerror creating the pipe: ");
            return 1;
        }
    }

    if ((mkfifo("pipe_to_client",0666))==-1) //pipe to client
    {
        if (errno!=EEXIST)
        {
            perror("\nerror creating the pipe: ");
            return 1;
        }
    }

    while (1)//keeping the server open 
    {
        int recever_fd=open("pipe_to_server",O_RDONLY);
        if (recever_fd==-1)
        {
            printf("\nerror in the receving file");
            perror("\n --> perror message: ");
            return 1;
        }
        char cmd;
        if ((read(recever_fd,&cmd, sizeof(char)))==-1)
        {
            printf("\ncouldn't get the command\n");
            perror("\n --> perror message: ");
            continue;
        }
        printf("command received......sex\n");
        printf("\ncommand: -%c \n",cmd);
        switch (cmd)
        {
            case 'a':
            {
                int len;
                char title[256]={0}, auther[256]={0}, year[10]={0}, path[256]={0};
                //im reading th len first because itll help me determing the length of the thing im sending 
                //and thats how i sent it, the len first then the thing(title,auther,year,path) 

                //the title
                if ((read(recever_fd, &len,sizeof(int)))<=0) 
                {
                    perror("1-error: ");
                    break;
                } printf("title len: %d\n", len);
                if ((read(recever_fd, title,len))<=0)
                {
                    perror("title --> error: ");
                    break;
                }printf("the title: %s\n", auther);

                //the auther
                if ((read(recever_fd, &len,sizeof(int)))<=0) 
                {
                    perror("2-error: ");
                    break;
                }printf("auther len: %d\n", len);
                if ((read(recever_fd, auther,len))<=0)
                {
                    perror("auther --> error: ");
                    break;
                }printf("the auther: %s\n", auther);

                //the year
                if ((read(recever_fd, &len,sizeof(int)))<=0) 
                {
                    perror("3-error: ");
                    break;
                }printf("year len: %d\n", len);
                if ((read(recever_fd, year,len))<=0)
                {
                    perror("year --> error: ");
                    break;
                }printf("the year: %s\n",year);
                
                //the path
                if ((read(recever_fd, &len,sizeof(int)))<=0) 
                {
                    perror("4-error: ");
                    break;
                }printf("path len: %d\n", len);
                if ((read(recever_fd, path,len))<=0)
                {
                    perror("path --> error: ");
                    break;
                }printf("the path: %s\n", path);

                printf("\nthe data: -%c %s %s %s %s\n", cmd, title, auther, year, path);
                
                // /*
                // the code for the search
                // waiting for fabio.........
                
                // */

                break;
            }
            case 'c':{
                int book_id;
                //book id to ask for info
                if ((read(recever_fd, &book_id,sizeof(int)))<=0)
                {
                    perror("book_id --> error: ");
                    break;
                }printf("the book_id: %d\n", book_id);
                break;

                /*
                code for remmoving the book
                */
            }
            case 'd':{
                int book_id;
                //book id to remove
                if ((read(recever_fd, &book_id,sizeof(int)))<=0)
                {
                    perror("book_id --> error: ");
                    break;
                }printf("the book_id: %d\n", book_id);
                break;

                /*
                code for remmoving the book
                */
            }
            case 'l':{
                int book_id;
                int book_size;
                char book_name[100];
                if ((read(recever_fd, &book_id,sizeof(int))<=0))
                {
                    perror("error: ");
                    break;
                }printf("the book id: %d\n", book_id);
                if ((read(recever_fd,&book_size,sizeof(int))<=0))
                {
                    printf("error in getting the book size");
                    perror("error: ");
                    break;
                } //printf("book size: %d\n", book_size);
                if ((read(recever_fd,book_name,book_size))<=0)
                {
                    printf("error getting the name");
                    perror("error: ");
                    break;
                }printf("book name: %s\n",book_name);

                /*
                code......
                */

                break;
            }
            case 's':
            {
                int word_len;
                char word[100];
                int n; //num of chil
                int k; //k is my key, face to face
                if ((read(recever_fd,&k,sizeof(int)))==-1)
                {
                    printf("error in getting the word len");
                    perror("error: ");
                    break;
                }
                if (k==2)
                {
                     if ((read(recever_fd,&word_len,sizeof(int)))==-1)
                    {
                        printf("error in getting the word len");
                        perror("error: ");
                        break;
                    }printf("word len: %d\n",word_len);
                    if ((read(recever_fd,word,word_len))==-1)
                    {
                        printf("error in getting the word");
                        perror("error: ");
                        break;
                    }printf("the word: %s\n",word);
                }
                else if(k==3)
                {
                    if ((read(recever_fd,&word_len,sizeof(int)))==-1)
                    {
                        printf("error in getting the word len");
                        perror("error: ");
                        break;
                    }printf("word len: %d\n",word_len);
                    if ((read(recever_fd,word,word_len))==-1)
                    {
                        printf("error in getting the word");
                        perror("error: ");
                        break;
                    }printf("the word: %s\n",word);

                    if ((read(recever_fd,&n,sizeof(int)))==-1)
                    {
                        printf("error in getting the number");
                        perror("error: ");
                        break; 
                    }
                    else printf("the nimber: %d\n",n);
                }
                else break;
                
            }

            
         
         default:
            break;
        }
        
        close(recever_fd);
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        //search_file(name_of_the_file);
        printf("thats it\n");
        break;
    }
    



    return 0;
}