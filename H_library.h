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
#define PIPE_TO_CLIENT "fifo_client" 

typedef struct FileInfo { 

    int id; 
    char op[3];
    int year; 
    char keyword[20];
    char title[100]; 
    char author[100];
    char path[64]; 

} FileInfo;  // we needed to change to static values not * it wont work with a write on the server fifo



void loading();
void search_file(const char *filename);