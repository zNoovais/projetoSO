#include "library.h"

int main(int argc, char* argv[]){

    if ( argc < 2 )
    {
        printf("Usage:\n");
        printf("Index request: ./dclient -a [Title] [Author] [Year] [Document name] \n");
        printf("Consult document: ./dclient -c [n]\n");
        printf("Remove request: ./dclient -d [n]\n");
        printf("Search number of lines of keyword: ./dclient -l [n] [keyword]\n");
        printf("Search keyword: ./dclient -s [keyword]\n");
        printf("Search keyword w/ processes: ./dclient -s [number of processes] [keyword]\n");
        printf("Shutdown: ./dclient -f\n");
        return 1;
    }

    char buffer[50]="";

    if ( strcmp(argv[1],"-a") == 0 )
    {
        // TODO
    }

    if ( strcmp(argv[1],"-c") == 0 )
    {
        // TODO
    }

    if ( strcmp(argv[1],"-d") == 0 )
    {
        // TODO
    }

    if ( strcmp(argv[1],"-l") == 0 )
    {
        // TO DO
    }

    if ( strcmp(argv[1],"-s") == 0)
    {
        // TO DO
    }

    return 0;

}