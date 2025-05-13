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

// void loading(){
//    // Clear screen (cross-platform)
//     system("clear");
    
//     int total=50;
//     printf("Loading: ");
//     for (int i = 0; i <= total; i++)
//     {
//         int percentage=(i*100)/total;
//         printf("\r"); // <<--- return to start of line
//         printf("loading: [");
//         for(int j = 0; j < i; j++) {
//             printf("#");
//         }
//         for(int j = i; j < total; j++) {
//             printf(" ");
//         }
//         printf("] %d%%", percentage);
//         fflush(stdout);
//         usleep(100000);
//     }
//     printf("\nloading complate!\n");
// }


// //temp function to search for file, needs fixing 
// void search_file(const char *filename) {
//     DIR *dir;
//     struct dirent *entry;
    
//     dir = opendir(" the file path");
//     if (dir == NULL) {
//         perror("Error opening directory");
//         return;
//     }
    
//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, filename) == 0) {
//             printf("File found: %s\n", filename);
//             closedir(dir); // file found
//             return;
//         }
//     }
    
//     closedir(dir);
//     return; // file not found
// }

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


//command -l
//================================================================================================================================================================
//================================================================================================================================================================

void search_keyword_in_document(int doc_id, const char *keyword, Linked* cache[CACHE_SIZE]) {
    printf("Searching for lines containing '%s' in document ID %d\n", keyword, doc_id);
    
    // First try to find the document in the cache
    char doc_path[65] = {0};
    int found_in_cache = 0;
    
    // Look for the document in the cache
    Linked* curr = cache[hash(doc_id)];
    while (curr != NULL) {
        if (curr->file.id == doc_id) {
            strcpy(doc_path, curr->file.path);
            found_in_cache = 1;
            printf("Document found in cache: ID %d, Path: %s\n", doc_id, doc_path);
            break;
        }
        curr = curr->next;
    }
    
    // If not found in cache, search in storage
    if (!found_in_cache) {
        printf("Document with ID %d not found in cache. Searching in storage...\n", doc_id);
        
        int fd_file_read = open(storage_path, O_RDONLY);
        if (fd_file_read == -1) {
            perror("Error opening storage file");
            printf("Could not open storage file\n");
            return;
        }
        
        indexed_file file;
        ssize_t bytes_read;
        
        // Read each document from storage until we find the one with matching ID
        while ((bytes_read = read(fd_file_read, &file, sizeof(file))) > 0) {
            if (file.active && file.id == doc_id) {
                strcpy(doc_path, file.path);
                printf("Document found in storage: ID %d, Path: %s\n", doc_id, doc_path);
                
                // Add to cache for future reference
                Linked* new_file = malloc(sizeof(Linked));
                if (new_file) {
                    new_file->file.id = file.id;
                    strcpy(new_file->file.title, file.title);
                    strcpy(new_file->file.author, file.author);
                    new_file->file.year = file.year;
                    strcpy(new_file->file.path, file.path);
                    
                    new_file->next = cache[hash(file.id)];
                    cache[hash(file.id)] = new_file;
                }
                
                found_in_cache = 1;  // We found it in storage
                break;
            }
        }
        
        close(fd_file_read);
    }
    
    // If document wasn't found in either cache or storage
    if (!found_in_cache || doc_path[0] == '\0') {
        printf("Document with ID %d not found\n", doc_id);
        return;
    }
    
    // Check if we need to prepend "Gdataset/" to the path
    char full_path[100] = {0};
    if (strncmp(doc_path, "Gdataset/", 9) == 0) {
        strcpy(full_path, doc_path);
    } else {
        strcpy(full_path, "Gdataset/");
        strcat(full_path, doc_path);
    }
    
    // Open the document file
    int doc_fd = open(full_path, O_RDONLY);
    if (doc_fd == -1) {
        // Try without Gdataset prefix if the first attempt failed
        doc_fd = open(doc_path, O_RDONLY);
        if (doc_fd == -1) {
            perror("Error opening document file");
            printf("Could not open document file: %s\n", full_path);
            return;
        }
    }
    
    // Read the file line by line and count matches
    char buffer[4096];
    char line_buffer[1024] = {0};
    int line_pos = 0;
    int line_count = 0;
    int matched_lines = 0;
    ssize_t bytes_read;
    
    while ((bytes_read = read(doc_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        
        // Process each character
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                // End of line
                line_buffer[line_pos] = '\0';  // Null-terminate the line
                line_count++;
                
                // Check if line contains the keyword
                if (strstr(line_buffer, keyword) != NULL) {
                    matched_lines++;
                }
                
                // Reset for next line
                line_pos = 0;
                memset(line_buffer, 0, sizeof(line_buffer));
            } else {
                // Add character to current line buffer
                if (line_pos < sizeof(line_buffer) - 1) {
                    line_buffer[line_pos++] = buffer[i];
                }
            }
        }
    }
    
    // Check the last line if it doesn't end with a newline
    if (line_pos > 0) {
        line_buffer[line_pos] = '\0';
        line_count++;
        
        if (strstr(line_buffer, keyword) != NULL) {
            matched_lines++;
        }
    }
    
    close(doc_fd);
    
    // Print results
    printf("Document ID: %d\n", doc_id);
    printf("Keyword: \"%s\"\n", keyword);
    printf("Total number of lines containing the keyword: %d\n", matched_lines);
}










//================================================================================================================================================================


int file_contains_word(const char *filepath, const char *keyword) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 0;
    }

    if (pid == 0) {  // child process
        close(pipefd[0]);
        
        // redirect stdout to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        // Execute grep to search for keyword
        execlp("grep", "grep", "-q", keyword, filepath, NULL);
        
        // if execlp fails
        perror("execlp");
        exit(1);
    } 
    else {  // parent process
        close(pipefd[1]);
        
        int status;
        waitpid(pid, &status, 0);
        
        // check if grep found the keyword (exit status 0 means found)
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            close(pipefd[0]);
            return 1;  // found
        } else {
            close(pipefd[0]);
            return 0;  // not found
        }
    }
}

Linked* cache[CACHE_SIZE]; 

void search_keyword_without_processes(const char *keyword, Linked* cache[CACHE_SIZE]) {
    printf("Searching for keyword: '%s'\n", keyword);
    int MAX_FILES = 1000;
    int found_ids[MAX_FILES];
    int found_count = 0;
    
    // PART 1: search through all buckets in the cache
    for (int i = 0; i < CACHE_SIZE; i++) {
        Linked* curr = cache[i]; 
        
        while (curr != NULL) {
            int found = 0;
            
            // check if keyword is in metadata (title or author)
            if (strstr(curr->file.title, keyword) || strstr(curr->file.author, keyword)) {
                found = 1;
                printf("Found in metadata: ID %d, title '%s'\n", curr->file.id, curr->file.title);
            } 
            // if not found in metadata, check document content if needed
            else if (curr->file.path[0] != '\0') {
                int fd = open(curr->file.path, O_RDONLY);
                if (fd != -1) {
                    char buffer[4096];
                    ssize_t bytes_read;
                    
                    while (!found && (bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
                        buffer[bytes_read] = '\0';
                        if (strstr(buffer, keyword) != NULL) {
                            found = 1;
                            printf("Found in content: ID %d, title '%s'\n", curr->file.id, curr->file.title);
                        }
                    }
                    close(fd);
                }
            }
            
            // if keyword was found, add document ID to results
            if (found && found_count < MAX_FILES) {
                // check if we already have this ID
                int already_found = 0;
                for (int j = 0; j < found_count; j++) {
                    if (found_ids[j] == curr->file.id) {
                        already_found = 1;
                        break;
                    }
                }
                
                if (!already_found) {
                    found_ids[found_count++] = curr->file.id;
                }
            }
            
            curr = curr->next;
        }
    }
    
    // PART 2: search through the storage file for any documents not in cache
    int fd_file_read = open(storage_path, O_RDONLY);
    if (fd_file_read != -1) {
        indexed_file file;
        ssize_t bytes_read;
        
        // Read each document from storage
        while ((bytes_read = read(fd_file_read, &file, sizeof(file))) > 0) {
            int found = 0;
            int already_in_cache = 0;
            
            // Check if this document is already in our cache
            for (int i = 0; i < CACHE_SIZE; i++) {
                Linked* curr = cache[i];
                while (curr != NULL) {
                    if (curr->file.id == file.id) {
                        already_in_cache = 1;
                        break;
                    }
                    curr = curr->next;
                }
                if (already_in_cache) break;
            }
            
            // skip if already in cache (we already checked it)
            if (already_in_cache) continue;
            
            // check metadata for keyword
            if (strstr(file.title, keyword) || strstr(file.author, keyword)) {
                found = 1;
                printf("Found in storage metadata: ID %d, title '%s'\n", file.id, file.title);
            }
            // check document content if needed
            else if (file.path[0] != '\0') {
                int doc_fd = open(file.path, O_RDONLY);
                if (doc_fd != -1) {
                    char buffer[4096];
                    ssize_t doc_bytes_read;
                    
                    while (!found && (doc_bytes_read = read(doc_fd, buffer, sizeof(buffer) - 1)) > 0) {
                        buffer[doc_bytes_read] = '\0';
                        if (strstr(buffer, keyword) != NULL) {
                            found = 1;
                            printf("Found in storage content: ID %d, title '%s'\n", file.id, file.title);
                        }
                    }
                    close(doc_fd);
                }
            }
            
            // if keyword was found, add document ID to results
            if (found && found_count < MAX_FILES) {
                // check if we already have this ID
                int already_found = 0;
                for (int j = 0; j < found_count; j++) {
                    if (found_ids[j] == file.id) {
                        already_found = 1;
                        break;
                    }
                }
                
                if (!already_found) {
                    found_ids[found_count++] = file.id;
                }
            }
        }
        
        close(fd_file_read);
    }
    
    
    printf("[");
    for (int i = 0; i < found_count; i++) {
        if (i > 0) {
            printf(", ");
        }
        printf("%d", found_ids[i]);
    }
    printf("]\n");
}


//=============================================================================================================================================================================================
//=============================================================================================================================================================================================


void search_keyword_with_processes(const char *keyword, Linked* cache[CACHE_SIZE], int num_processes) {
    printf("Searching for keyword: '%s' with %d processes\n", keyword, num_processes);
    int MAX_FILES = 1000;
    int found_ids[MAX_FILES];
    int found_count = 0;
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }
    
    // making shared memory for tracking which cache entries have been processed
    int total_items = 0;
    
    // counting total items in cache
    for (int i = 0; i < CACHE_SIZE; i++) {
        Linked* curr = cache[i];
        while (curr != NULL) {
            total_items++;
            curr = curr->next;
        }
    }
    
    // get file size
    int fd_file_size = open(storage_path, O_RDONLY);
    if (fd_file_size == -1) {
        perror("Error opening storage file");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }
    
    off_t file_size = lseek(fd_file_size, 0, SEEK_END);
    close(fd_file_size);
    
    int storage_items = file_size / sizeof(indexed_file);
    int total_work_items = total_items + storage_items;
    
    // determine work distribution
    int items_per_process = (total_work_items + num_processes - 1) / num_processes;
    
    
    pid_t pids[num_processes];
    for (int p = 0; p < num_processes; p++) {
        pids[p] = fork();
        
        if (pids[p] < 0) {
            perror("fork");
            continue;
        }
        
        if (pids[p] == 0) { // child
            close(pipefd[0]); // read
            
            int start_idx = p * items_per_process;
            int end_idx = (p + 1) * items_per_process;
            if (end_idx > total_work_items) end_idx = total_work_items;
            
            int child_found_ids[MAX_FILES];
            int child_found_count = 0;
            
            
            int current_idx = 0;
            
            // PART 1: search through assigned cache items
            for (int i = 0; i < CACHE_SIZE && current_idx < end_idx; i++) {
                Linked* curr = cache[i];
                
                while (curr != NULL && current_idx < end_idx) {
                    if (current_idx >= start_idx) {
                        int found = 0;
                        
                        // checking if keyword is in metadata (title or author)
                        if (strstr(curr->file.title, keyword) || strstr(curr->file.author, keyword)) {
                            found = 1;
                            printf("Child %d found in metadata: ID %d, title '%s'\n", p, curr->file.id, curr->file.title);
                        } 
                        // if not found in metadata, check document content if needed
                        else if (curr->file.path[0] != '\0') {
                            int fd = open(curr->file.path, O_RDONLY);
                            if (fd != -1) {
                                char buffer[4096];
                                ssize_t bytes_read;
                                
                                while (!found && (bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
                                    buffer[bytes_read] = '\0';
                                    if (strstr(buffer, keyword) != NULL) {
                                        found = 1;
                                        printf("Child %d found in content: ID %d, title '%s'\n", p, curr->file.id, curr->file.title);
                                    }
                                }
                                close(fd);
                            }
                        }
                        
                        // if keyword was found, add document ID to results
                        if (found && child_found_count < MAX_FILES) {
                            // check if we already have this ID
                            int already_found = 0;
                            for (int j = 0; j < child_found_count; j++) {
                                if (child_found_ids[j] == curr->file.id) {
                                    already_found = 1;
                                    break;
                                }
                            }
                            
                            if (!already_found) {
                                child_found_ids[child_found_count++] = curr->file.id;
                            }
                        }
                    }
                    
                    current_idx++;
                    curr = curr->next;
                }
            }
            
            // if cache items are less than end_idx, continue with storage items
            if (current_idx < end_idx) {

                // PART 2: search through the storage file for assigned documents
                int fd_file_read = open(storage_path, O_RDONLY);
                if (fd_file_read != -1) {
                    indexed_file file;
                    ssize_t bytes_read;
                    int storage_idx = 0;
                    
                    // Skip to the starting position for this process
                    if (start_idx > total_items) {
                        int skip_items = start_idx - total_items;
                        lseek(fd_file_read, skip_items * sizeof(indexed_file), SEEK_SET);
                        storage_idx = skip_items;
                    }
                    
                    // read assigned documents from storage
                    while (current_idx < end_idx && (bytes_read = read(fd_file_read, &file, sizeof(file))) > 0) {
                        if (storage_idx >= (start_idx - total_items) && storage_idx < (end_idx - total_items)) {
                            int found = 0;
                            int already_in_cache = 0;
                            
                            // checking if this document is already in our cache
                            for (int i = 0; i < CACHE_SIZE; i++) {
                                Linked* curr = cache[i];
                                while (curr != NULL) {
                                    if (curr->file.id == file.id) {
                                        already_in_cache = 1;
                                        break;
                                    }
                                    curr = curr->next;
                                }
                                if (already_in_cache) break;
                            }
                            
                            // skip if already in cache (we already checked it)
                            if (already_in_cache) {
                                storage_idx++;
                                current_idx++;
                                continue;
                            }
                            
                            // check metadata for keyword
                            if (strstr(file.title, keyword) || strstr(file.author, keyword)) {
                                found = 1;
                                printf("Child %d found in storage metadata: ID %d, title '%s'\n", p, file.id, file.title);
                            }
                            // check document content if needed
                            else if (file.path[0] != '\0') {
                                int doc_fd = open(file.path, O_RDONLY);
                                if (doc_fd != -1) {
                                    char buffer[4096];
                                    ssize_t doc_bytes_read;
                                    
                                    while (!found && (doc_bytes_read = read(doc_fd, buffer, sizeof(buffer) - 1)) > 0) {
                                        buffer[doc_bytes_read] = '\0';
                                        if (strstr(buffer, keyword) != NULL) {
                                            found = 1;
                                            printf("Child %d found in storage content: ID %d, title '%s'\n", p, file.id, file.title);
                                        }
                                    }
                                    close(doc_fd);
                                }
                            }
                            
                            // if keyword was found, add document ID to results
                            if (found && child_found_count < MAX_FILES) {
                                // check if we already have this ID
                                int already_found = 0;
                                for (int j = 0; j < child_found_count; j++) {
                                    if (child_found_ids[j] == file.id) {
                                        already_found = 1;
                                        break;
                                    }
                                }
                                
                                if (!already_found) {
                                    child_found_ids[child_found_count++] = file.id;
                                }
                            }
                        }
                        
                        storage_idx++;
                        current_idx++;
                    }
                    
                    close(fd_file_read);
                }
            }
            
            // send results back to parent through pipe
            write(pipefd[1], &child_found_count, sizeof(int));
            write(pipefd[1], child_found_ids, child_found_count * sizeof(int));
            
            close(pipefd[1]);
            exit(0);
        }
    }
    
    // parent
    close(pipefd[1]); // write
    
    // collect results from children
    for (int p = 0; p < num_processes; p++) {
        int status;
        waitpid(pids[p], &status, 0);
        
        // from pipe
        int child_found_count;
        if (read(pipefd[0], &child_found_count, sizeof(int)) > 0) {
            int child_found_ids[MAX_FILES];
            read(pipefd[0], child_found_ids, child_found_count * sizeof(int));
            
            // merge with parent results
            for (int i = 0; i < child_found_count; i++) {
                int already_found = 0;
                for (int j = 0; j < found_count; j++) {
                    if (found_ids[j] == child_found_ids[i]) {
                        already_found = 1;
                        break;
                    }
                }
                
                if (!already_found && found_count < MAX_FILES) {
                    found_ids[found_count++] = child_found_ids[i];
                }
            }
        }
    }
    
    close(pipefd[0]);
    
    // print
    printf("[");
    for (int i = 0; i < found_count; i++) {
        if (i > 0) {
            printf(", ");
        }
        printf("%d", found_ids[i]);
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





int hash(int key) {
    return key % CACHE_SIZE;
}