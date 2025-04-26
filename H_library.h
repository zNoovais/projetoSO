#include <stdio.h>
#include <string.h>  
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>

//#define SERVER "fifo_server"
#define PIPE_TO_SERVER "fifo_server" // this is the name of the server fifo just to make it in one place! (like in guide 5 again..)
// i changed ^^^^^^this fucker to PIPE_TO_SERVER so that we can understand where it going 

//infromation abiout the books and shit
typedef struct FileInfo { //this should not exceed 512bytes

    int id; // this will be the pid so that we can find the client fifo (check guide 5 is smth like that) aka pipe: fd_from_server
    int processes; //the number of children (-s)

    char cmd[3]; //op
    
    char title[100]; // max 200bytes the author and the title together
    char author[100];
    int year; // max 4bytes ez
    char path[64]; // max 64bytes (idk why)

    char word[20]; //the key_word to search for 
    int index; //the index of the word (-a)


} FileInfo;

