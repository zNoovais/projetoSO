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


#define PIPE_TO_SERVER "fifo_server" // this is the name of the server fifo just to make it in one place! (like in guide 5 again..)

#define PIPE_TO_CLIENT "fifo_client" 

#define CACHE_LINES 10

#define storage_path "armazenamento"

#define MAX_CACHE_SIZE 30 // max size of the cache

#define MAX_LINE_SIZE MAX_CACHE_SIZE/CACHE_LINES

//infromation abiout the books and shit
typedef struct FileInfo { //this should not exceed 512bytes

    int id; // this will be the pid so that we can find the client fifo (check guide 5 is smth like that) aka pipe: fd_from_server
    int processes; //the number of children (-s)

    char cmd[8]; //op
    
    char title[100]; // max 200bytes the author and the title together
    char author[100];
    int year; // max 4bytes ez
    char path[64];
    int index; 
    char word[20]; //the key_word to search for 
} FileInfo;


typedef struct indexed_file {
    int active;
    int id;
    char title[100]; 
    char author[100];
    int year; 
    char path[64]; 
} indexed_file; 

typedef struct LinkedList {
    indexed_file file;
    int last_accessed; 
    struct LinkedList *next;
} Linked;

typedef struct cacheLines{
    int occupation;
    struct LinkedList *list;

} cacheLines; 

int hash(int key); // hash function key % CACHE_LINES

void freeL(Linked *link); // clean a linked list using free


/// -l ///
int search_keyword(cacheLines cache[CACHE_LINES], char *word, int index, int fd_file_read, char *msg, int *cache_occupation, int virtual_time);
int grep_wc(char *word, char *filename, char *msg);

/// -s funcs ///
int search_contains_word(char *keyword, int number_of_processes, int number_of_files, int fd_file_read,int deleted, char *msg, int msg_size);
int file_contains_word(char *filepath, char *keyword); 
int search_process(char *keyword, int search_amount, int start_point, int pip[2]);


//// cache policie ///
int remove_least_used(Linked *list, int virtual_time);