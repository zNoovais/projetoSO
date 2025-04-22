#include <stdio.h>   // For printf function
#include <stdlib.h>  // For system function
#include <unistd.h>  // For sleep/usleep functions (to replace delay)
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
    
    dir = opendir("/Users/samirMansour_1/Desktop/UNI/Year 2/Secund semester/Operating System/proj/DatasetTest/Gdataset");
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