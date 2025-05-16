#include <stdio.h>   // For printf function
#include <stdlib.h>  // For system function
#include <unistd.h>  // For sleep/usleep functions (to replace delay)
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "H_library.h"


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

int search_keyword(cacheLines cache[CACHE_LINES], char *word, int index, int fd_file_read, char *msg, int *cache_occupation, int virtual_time) {

    int count = -1;
            
    Linked* curr = cache[hash(index)].list;
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

            if (cache[hash(file_struct.id)].occupation >= MAX_LINE_SIZE) {
                remove_least_used(cache[hash(file_struct.id)].list, virtual_time);
            }

            new_file->next = cache[hash(file_struct.id)].list; // hash function to get the index in the cache
            cache[hash(file_struct.id)].list = new_file; // adding the new file to the cache :DD

        }
    }
    
    return count;
}

int search_contains_word(char *keyword, int number_of_processes, int number_of_files, int fd_file_read,int deleted, char *msg, int msg_size) { // this is the function that will be called by the client to search for a word in the storage{

    int res;
    indexed_file file_struct;

    //this part here just make sure we have no gaps
    
    lseek(fd_file_read, 0, SEEK_SET);
   

    if (deleted != 0) {

        int fd_new = open("temp",O_CREAT | O_TRUNC | O_WRONLY, 0600);

        while ((res = read(fd_file_read, &file_struct, sizeof(indexed_file))) > 0) {
            if (res == -1) {
                perror("Error reading from storage file");
                close(fd_file_read);
                
                return 1;
            }

            if (file_struct.active) {
            // Write the active file to the new file
                if (write(fd_new, &file_struct, sizeof(indexed_file)) == -1) {
                    perror("Error writing to new storage file");
                    close(fd_new);
                    close(fd_file_read);
                    
                    return 1;
                }
                        
            }
        }

        deleted = 0;
        close(fd_new);

    }

    lseek(fd_file_read, 0, SEEK_SET);
   

    int pip[number_of_processes][2];
    for (int i = 0; i < number_of_processes; i++) {
        if (pipe(pip[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    int search_amount = number_of_files / number_of_processes;
    

    for (int i = 0; i < number_of_processes; i++) {
        
        int starting_point = i*search_amount;

        if (i + 1 == number_of_processes) {
            search_amount = number_of_files; // distribui o resto
        }

        pid_t pid = fork();
        if (pid == 0) {
            search_process(keyword, search_amount, starting_point, pip[i]);
            exit(0);
        }

        
    }

    char buffer[520];
    
    strncat(msg,"[", msg_size - strlen(msg) - 1);


    for(int i = 0; i < number_of_processes; i++) {

        close(pip[i][1]); // close the write end of the pipe
        

        ssize_t bytes_read = read(pip[i][0], buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; 
            strncat(msg, buffer, msg_size - strlen(msg) - 1);
            
        }
        else if (bytes_read == -1) {
            perror("Error reading from pipe");
        }
        else {
            printf("No data read from pipe\n");
        }

        close(pip[i][0]); // close the read end of the pipe

        
        
    }

    size_t len = strlen(msg);
    if (len > 0 && msg[len - 1] == ',') {
        msg[len - 1] = '\0'; 
    }

    strncat(msg, "]\n", msg_size - strlen(msg) - 1);

    for (int i = 0; i < number_of_processes; i++) {
        wait(NULL);
    }

    return 0;

}


int search_process(char *keyword, int search_amount, int start_point, int pip[2]) { 

   

    int fd = open(storage_path, O_RDONLY);  
    if (fd == -1) {
        perror("open");
        return 1;
    }

    lseek(fd, start_point*sizeof(indexed_file), SEEK_SET);
    close(pip[0]); // closing the read end of the pipe

    int res;
    indexed_file file_struct;
    char msg[8192] = "";
    char buffer[64];

    for (int i = 0; i < search_amount; i++) {
        res = read(fd, &file_struct, sizeof(indexed_file));
        if (res == 0) {
            printf("file's over\n");
            break;
        }

        if (res == -1) {
            perror("Error reading from storage file");
            close(fd);
            
            return 1;
        }

        if (file_struct.active ) { // this should always be active but just to be sure
            
            if (file_contains_word(file_struct.path, keyword)) {
                printf("Found in storage: %s\n", file_struct.path);
                snprintf(buffer, sizeof(buffer), "%d,", file_struct.id);
                strncat(msg, buffer, sizeof(msg) - strlen(msg) - 1);
            } 
        }

        
    }

    if (write(pip[1], msg, strlen(msg)) == -1) {
        perror("Error writing to pipe");
        close(pip[1]);
        return 1;
    }
    close(fd);
    close(pip[1]); // closing the write end of the pipe
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


int remove_least_used(Linked *list, int virtual_time) {

    Linked* curr = list;
    Linked* prev = list;

    int least_id = curr->file.id;
    int time = virtual_time - curr->last_accessed; 

    curr = curr->next;


    while (curr != NULL) {
        if(time < virtual_time - curr->last_accessed) { // seeing who is the least used
            least_id = curr->file.id;
        }
        curr = curr->next;
    }

    curr = list; // going back to the beggining 

    if (curr->file.id == least_id) {
        list = curr->next; // removing the first element
        free(curr);            
    } 
    else {
        prev = curr; // setting the previous element to the first oneeee
        curr = curr->next; // moving to the next element

        while (curr != NULL) {                  // traversing the cache on the hash index !!
            if (curr->file.id == least_id) {
                prev->next = curr->next; // removing the element
                free(curr);
                break;
            }
            prev = curr;
            curr = curr->next;
                }
            }
    return 0;
}




void freeL(Linked *link) {

    if (!link) { // if it is null
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