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

#define SERVER "fifo_server" // this is the name of the server fifo just to make it in one place! (like in guide 5 again..)


typedef struct FileInfo { //this should not exceed 512bytes

    int id; // this will be the pid so that we can find the client fifo (check guide 5 is smth like that)

    int year; // max 4bytes ez
    char* title; // max 200bytes the author and the title together
    char* author;
    char* path; // max 64bytes (idk why)

} FileInfo;



void loading();
void search_file(const char *filename);