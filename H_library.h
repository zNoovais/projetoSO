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





typedef struct FileInfo{ //this should not exceed 512bytes

    int year; // max 4bytes ez
    char* title; // max 200bytes the author and the title together
    char* author;
    char* path; // max 64bytes (idk why)

} FileInfo;



void loading();
void search_file(const char *filename);