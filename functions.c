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






// Interesting struct tha we could use in the server, if I am not wrong R: yea we can use this for storing in memory the documents that we are indexing
typedef struct Document {
    int id;
    char title[201];
    char authors[201];
    int year;
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
    strcpy(doc->title, title);
    strcpy(doc->authors, authors);
    doc-> year = atoi(year);
    strcpy(doc->path, path); 
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
            printf("Year: %d\n", curr->year);
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

/////// -l


int grep_wc(char *word, char *path, char *msg) { // this is basically those exercices with the dups and the pipes 

    int pip1[2];
    int pip2[2];

    if (pipe(pip1) == -1) {
        perror("pipe failed..");
        exit(1);
    }
    if (pipe(pip2) == -1) {
        perror("pipe failed..");
        exit(1);
    }



    if(fork()==0) {

        close(pip1[0]);
        dup2(pip1[1],1);  // i think thats the order the professor likes it idk at this point
        close(pip1[1]);

        close(pip2[0]);
        close(pip2[1]);

        char *args[] = {"grep",  word, path, NULL};
        execvp(args[0], args);

        perror("execvp failed");
        exit(1);
                
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pip1[1]);
        close(pip2[0]);
        dup2(pip1[0],0);
        close(pip1[0]);

        dup2(pip2[1],1);
        close(pip2[1]);

        char *args[] = {"wc", "-l", NULL};
        execvp(args[0], args);

        perror("execvp failed");
        exit(1);
        }

            close(pip1[1]);
            close(pip2[1]);
            close(pip1[0]);
            
            char buffer[128];
            ssize_t bytes_read = read(pip2[0], buffer, sizeof(buffer) - 1);
            if (bytes_read <= 0) {
                perror("read failed");
                close(pip2[0]);
                wait(NULL);
                wait(NULL);
                return -1;
            }

            buffer[bytes_read] = '\0';

            int count = 0;
            sscanf(buffer, "%d", &count);

            
            close(pip2[0]);

            wait(NULL);
            wait(NULL);

            sprintf(msg,"Found %s", buffer); // the %s already has the '\n' cuz of the wc x)

            return count;

}




int search_keyword(Linked* cache[CACHE_LINES], char *word, int index, int fd_file_read, char *msg, int *cache_occupation, int virtual_time) {

    int count = -1;
            
    Linked* curr = cache[hash(index)];
        while (curr != NULL) {                  // traversing the cache on the hash index !!
                
        if (curr->file.id == index) {
            printf("Found in cache\n");
            curr->last_accessed = virtual_time; // last time used
            count = grep_wc(word, curr->file.path, msg); // when its found in the cache 
            break;

        }
        
        curr = curr->next;          
    }

    if (curr == NULL) { // didnt find it in the cache so its goint thru the storage.
        
        int res;
        
        printf("Document with ID %d not found in cache.\n", index);
        printf("Searching in storage...\n");
        lseek(fd_file_read, 0, SEEK_SET); // going back to the start of the file
                
        indexed_file file_struct;

        while((res = read(fd_file_read,&file_struct,sizeof(indexed_file))) > 0) {
                    
            if(file_struct.active && file_struct.id == index) {
                
                break;
            }
                    
        }

        if (res == 0) {
            sprintf(msg,"didnt find nothing...\n");
        }

        else {
            
            count = grep_wc(word, file_struct.path, msg); // when its found in the storage
    
            Linked* new_file = malloc(sizeof(Linked));  
            
            (*cache_occupation)++;
            new_file->last_accessed = virtual_time; // last time used
            new_file->file.id = file_struct.id;
            strcpy(new_file->file.title, file_struct.title);
            strcpy(new_file->file.author, file_struct.author);
            new_file->file.year = file_struct.year;
            strcpy(new_file->file.path, file_struct.path);

            new_file->next = cache[hash(file_struct.id)]; // hash function to get the index in the cache
            cache[hash(file_struct.id)] = new_file; // adding the new file to the cache :DD

        }
    }
    
    return count;
}

// int file_contains_word(const char *filepath, const char *keyword) { // samir's function  to search if a path contains a word I really liked it
//     int pipefd[2];
//     if (pipe(pipefd) == -1) {
//         perror("pipe");
//         return 0;
//     }

//     pid_t pid = fork();
//     if (pid == -1) {
//         perror("fork");
//         close(pipefd[0]);
//         close(pipefd[1]);
//         return 0;
//     }

//     if (pid == 0) {  // child process
//         close(pipefd[0]);
        
//         // redirect stdout to pipe
//         dup2(pipefd[1], STDOUT_FILENO);
//         close(pipefd[1]);
        
//         // Execute grep to search for keyword
//         execlp("grep", "grep", "-q", keyword, filepath, NULL);
        
//         // if execlp fails
//         perror("execlp");
//         exit(1);
//     } 
//     else {  // parent process
//         close(pipefd[1]);
        
//         int status;
//         waitpid(pid, &status, 0);
        
//         // check if grep found the keyword (exit status 0 means found)
//         if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
//             close(pipefd[0]);
//             return 1;  // found
//         } else {
//             close(pipefd[0]);
//             return 0;  // not found
//         }
//     }
// }

int search_contains_word(char *keyword, int number_of_processes, int number_of_files, int fd_file_read, int fd_file_write, int deleted)   {

    int res;
    indexed_file file_struct;

    //this part here just make sure we have no gaps
    
    lseek(fd_file_read, 0, SEEK_SET);
    lseek(fd_file_write, 0, SEEK_SET);

    if (deleted != 0) {

        int fd_new = open("temp",O_CREAT | O_TRUNC | O_WRONLY, 0600);

        while ((res = read(fd_file_read, &file_struct, sizeof(indexed_file))) > 0) {
            if (res == -1) {
                perror("Error reading from storage file");
                close(fd_file_read);
                close(fd_file_write); 
                return 1;
            }

            if (file_struct.active) {
            // Write the active file to the new file
                if (write(fd_new, &file_struct, sizeof(indexed_file)) == -1) {
                    perror("Error writing to new storage file");
                    close(fd_new);
                    close(fd_file_read);
                    close(fd_file_write); 
                    return 1;
                }
                        
            }
        }

        deleted = 0;

    }

    lseek(fd_file_read, 0, SEEK_SET);
    lseek(fd_file_write, 0, SEEK_SET);

    int search_amount = number_of_files / number_of_processes;
    
    return 0;

}






int file_contains_word(char *filepath, char *keyword) { // 1 or 0
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork  errorr");
        return 0;
    }
    int status;

    if (pid == 0) { 
        execlp("grep", "grep", "-q", keyword, filepath, NULL);
        perror("execlp");
        exit(1);
    } else {  
        
        waitpid(pid, &status, 0); // im getting the status by using this 


        return (WIFEXITED(status) && WEXITSTATUS(status) == 0); // the left one is if the exit went good the other one is the exit status
    }
}



//================================================================================================================================================================
            // the shit i made ::::::::::


//-s without the n after keyword
void search_keyword_without_processes(const char *keyword) {
    int MAX_FILES=2220, MAX_FILENAME=2220, searchers=4;   
    const char *folder = "Gdataset";
    char files[MAX_FILES][MAX_FILENAME];
    int file_count = 0;

    // step 1: parent collects all .txt files
    DIR *dir = opendir(folder);
    if (!dir) { perror("opendir"); exit(1); }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        int len = strlen(entry->d_name);
        if (len > 4 && strcmp(entry->d_name + len - 4, ".txt") == 0) {
            snprintf(files[file_count++], MAX_FILENAME, "%s/%s", folder, entry->d_name);
        }
    }
    closedir(dir);

    // pipe for each child to send results to parent
    int pipes[searchers][2];

    // step 2: fork children and assign files
    for (int i = 0; i < searchers; i++) {
        if (pipe(pipes[i]) == -1) { perror("pipe"); exit(1); }
        pid_t pid = fork();
        if (pid == 0) {
            // child
            close(pipes[i][0]); // close read end

            int found_ids[MAX_FILES];
            int found_count = 0;

            for (int j = i; j < file_count; j += searchers) {
                // inline file_contains_word logic
                FILE *fp = fopen(files[j], "r");
                if (fp) {
                    char line[4096];
                    int found = 0;
                    while (fgets(line, sizeof(line), fp)) {
                        if (strstr(line, keyword)) {
                            found = 1;
                            break;
                        }
                    }
                    fclose(fp);
                    if (found) {
                        found_ids[found_count++] = j + 1; // IDs start at 1
                    }
                }
            }

            // write found_count and then all found_ids to pipe
            write(pipes[i][1], &found_count, sizeof(int));
            if (found_count > 0) {
                write(pipes[i][1], found_ids, found_count * sizeof(int));
            }
            close(pipes[i][1]);
            exit(0);
        } else {
            close(pipes[i][1]); // parent closes write end
        }
    }

    // Step 3: parent waits and collects all IDs
    int all_ids[MAX_FILES];
    int all_count = 0;

    for (int i = 0; i < searchers; i++) {
        int found_count = 0;
        read(pipes[i][0], &found_count, sizeof(int));
        if (found_count > 0) {
            int ids[found_count];
            read(pipes[i][0], ids, found_count * sizeof(int));
            for (int k = 0; k < found_count; k++) {
                all_ids[all_count++] = ids[k];
            }
        }
        close(pipes[i][0]);
    }

    // wait for all children
    for (int i = 0; i < searchers; i++) wait(NULL);

    // step 4: print all IDs in the required format
    printf("[");
    for (int i = 0; i < all_count; i++) {
        printf("%d", all_ids[i]);
        if (i != all_count - 1) printf(",");
    }
    printf("]\n");
}

//dick
//================================================================================================================================================================

void search_keyword_with_processes(const char *keyword, int n){
    int MAX_FILES=2220, MAX_FILENAME=2220, searchers=n;   
    const char *folder = "Gdataset";
    char files[MAX_FILES][MAX_FILENAME];
    int file_count = 0;

    // step 1: parent collects all .txt files
    DIR *dir = opendir(folder);
    if (!dir) { perror("opendir"); exit(1); }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        int len = strlen(entry->d_name);
        if (len > 4 && strcmp(entry->d_name + len - 4, ".txt") == 0) {
            snprintf(files[file_count++], MAX_FILENAME, "%s/%s", folder, entry->d_name);
        }
    }
    closedir(dir);

    // pipe for each child to send results to parent
    int pipes[searchers][2];

    // step 2: Fork children and assign files
    for (int i = 0; i < searchers; i++) {
        if (pipe(pipes[i]) == -1) { perror("pipe"); exit(1); }
        pid_t pid = fork();
        if (pid == 0) {
            // child
            close(pipes[i][0]); // close read end

            int found_ids[MAX_FILES];
            int found_count = 0;

            for (int j = i; j < file_count; j += searchers) {
                // inline file_contains_word logic
                FILE *fp = fopen(files[j], "r");
                if (fp) {
                    char line[4096];
                    int found = 0;
                    while (fgets(line, sizeof(line), fp)) {
                        if (strstr(line, keyword)) {
                            found = 1;
                            break;
                        }
                    }
                    fclose(fp);
                    if (found) {
                        found_ids[found_count++] = j + 1; // IDs start at 1
                    }
                }
            }

            // write found_count and then all found_ids to pipe
            write(pipes[i][1], &found_count, sizeof(int));
            if (found_count > 0) {
                write(pipes[i][1], found_ids, found_count * sizeof(int));
            }
            close(pipes[i][1]);
            exit(0);
        } else {
            close(pipes[i][1]); // parent closes write end
        }
    }

    // step 3: parent waits and collects all IDs
    int all_ids[MAX_FILES];
    int all_count = 0;

    for (int i = 0; i < searchers; i++) {
        int found_count = 0;
        read(pipes[i][0], &found_count, sizeof(int));
        if (found_count > 0) {
            int ids[found_count];
            read(pipes[i][0], ids, found_count * sizeof(int));
            for (int k = 0; k < found_count; k++) {
                all_ids[all_count++] = ids[k];
            }
        }
        close(pipes[i][0]);
    }

    // wait for all children
    for (int i = 0; i < searchers; i++) wait(NULL);

    // step 4: print all IDs in the required format
    printf("[");
    for (int i = 0; i < all_count; i++) {
        printf("%d", all_ids[i]);
        if (i != all_count - 1) printf(",");
    }
    printf("]\n");
  
}




void freeL(Linked *link) {

    if (!link) {
        return;
    }

    Linked *atual = link;

    Linked *pre;

    while(atual) {
        pre = atual->next;
        free(atual);
        atual = pre;
    }

}

int hash(int k) {
    return k % CACHE_LINES;
}