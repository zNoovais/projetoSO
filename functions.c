#include <stdio.h>   // For printf function
#include <stdlib.h>  // For system function
#include <unistd.h>  // For sleep/usleep functions (to replace delay)
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "H_library.h"

/*
#include <ncurses.h>

initscr();  // Initialize ncurses
clear();    // Clear the screen
refresh();  // Update the screen
*/

void loading(){
   // Clear screen (cross-platform)
    system("clear");
    
    int total=50;
    printf("Loading: ");
    for (int i = 0; i <= total; i++)
    {
        int percentage=(i*100)/total;
        printf("\r"); // <<--- return to start of line
        printf("loading: [");
        for(int j = 0; j < i; j++) {
            printf("#");
        }
        for(int j = i; j < total; j++) {
            printf(" ");
        }
        printf("] %d%%", percentage);
        fflush(stdout);
        usleep(100000);
    }
    printf("\nloading complate!\n");
}


//temp function to search for file, needs fixing 
void search_file(const char *filename) {
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(" the file path");
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, filename) == 0) {
            printf("File found: %s\n", filename);
            closedir(dir); // file found
            return;
        }
    }
    
    closedir(dir);
    return; // file not found
}

// Interesting struct tha we could use in the server, if I am not wrong R: yea we can use this for storing in memory the documents that we are indexing
typedef struct Document {
    int id;
    char title[201];
    char authors[201];
    char year[5];
    char path[65];
    struct Document *next;
} Document;

Document *doc_list = NULL;
int next_id = 1;

// Command -a
// This function I think it's okay. It would be called by the server, but the arguments will come from client.
// Finally, the result of this function will have to go to the client, and printed in the client
int index_document(const char *title, const char *authors, const char *year, const char *path) {
    Document *doc = malloc(sizeof(Document));
    if (!doc) {
        perror("malloc");
        return -1;
    }

    doc->id = next_id++;
    strncpy(doc->title, title, 200); doc->title[200] = '\0';
    strncpy(doc->authors, authors, 200); doc->authors[200] = '\0';
    strncpy(doc->year, year, 4); doc->year[4] = '\0';
    strncpy(doc->path, path, 64); doc->path[64] = '\0';
    doc->next = doc_list;
    doc_list = doc;

    return doc->id;
}

// Command -c
// This function has to be changed soon: 1. All of these prints will have to be printed in the client. 
// 2. Problably, this function will return either 1 or 0 ( the docuent with that key already exists - return 1
// otherwise, return 0 ). 3. If this function returns 1, we will have to find a good way to carry these informations 
// ( Title, Authors, Year and Path ) to the client, in order to they be printed.
void consult_document(int key) {
    Document *curr = doc_list;
    while (curr != NULL) {
        if (curr->id == key){
            printf("Title: %s\n", curr->title);
            printf("Authors: %s\n", curr->authors);
            printf("Year: %s\n", curr->year);
            printf("Path: %s\n", curr->path);
            return;
        }
        curr = curr->next;
    }

    printf("Error: Document with ID %d not found.\n", key);
}

// Command -d
// This function has to be changed soon: 1. All of these prints will have to be printed in the client.
// 2. Suggestion: This function have to return either 1 or 0. If returns 1, we already have the key, and then we only
// print in the client: "Index entry %d deleted".
void remove_document_metadata(int key) {
    Document *curr = doc_list;
    Document *prev = NULL;

    while (curr != NULL) {
        if (curr->id == key) {
            if (prev == NULL) {
                doc_list = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            printf("Metadata for document with ID %d successfully removed.\n", key);
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    printf("Error: Document with ID %d not found.\n", key);
}

// Command -l
// In general, I think this function is good. We only have to:
// 1. Make it more friendly ( use more functions that are knows for us, instead of some "weird" functions )
// 2. This function will work in the server, but we have to send the return value to the client, and then the 
// client have to print it.
int count_lines_with_keyword(int key, const char *keyword) {
    Document *curr = doc_list;
    while (curr != NULL) {
        if (curr->id == key) {
            int fd = open(curr->path, O_RDONLY);
            if (fd < 0) {
                perror("open");
                return -1;
            }

            char buffer[4096];
            ssize_t bytes_read;
            int lines_with_keyword = 0;
            char *line_start = buffer;
            size_t total = 0;

            while ((bytes_read = read(fd, buffer + total, sizeof(buffer) - total - 1)) > 0) {
                buffer[total + bytes_read] = '\0';
                char *line = strtok(buffer, "\n");
                while (line != NULL) {
                    if (strstr(line, keyword) != NULL) {
                        lines_with_keyword++;
                    }
                    line = strtok(NULL, "\n");
                }
                total = 0;
            }
            close(fd);
            return lines_with_keyword;
        }
        curr = curr->next;
    }
    return -1;
}

// Command -s
// This function is bad ( Sorry !!! ). 1. We have to decide if we are going to return a String, then we send
// it to the client, and then the client print it ( It would be a String that in the beggining there is a "[" and 
// at the end there is a "]". If you guys want, we can choose other approach to this function.
void list_documents_with_keyword(const char *keyword) {
    Document *curr = doc_list;
    int first = 1;

    printf("[");
    while (curr != NULL) {
        char command[512];
        snprintf(command, sizeof(command), "grep -q \"%s\" \"%s\"", keyword, curr->path);
        int result = system(command);

        if (WIFEXITED(result) && WEXITSTATUS(result) == 0) {
            if (!first) {
                printf(", ");
            }
            printf("%d", curr->id);
            first = 0;
        }

        curr = curr->next;
    }
    printf("]\n");
}

// Command -f : I didn't do yet, because I think it depends a lot on how the server is built.
