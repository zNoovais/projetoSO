#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>

// this is our library. we put here our function for shit to work B)

typedef struct FileInfo{ //this should not exceed 512bytes

    int year; // max 4bytes ez
    char* title; // max 200bytes the author and the title together
    char* author;
    char* path; // max 64bytes (idk why)

} FileInfo;

// maybe this will be parameters in a pipe...


