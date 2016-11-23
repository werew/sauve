#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "types.h"
#include "common.h"
#include "scanner.h"

void usage(char* name, int exit_value){
    printf("usage: %s [-h] [-n] [-s n] [-a n] [-f n] "
           "source [previous] destination\n",name);
    exit(exit_value);
}


int main(int argc, char* argv[]){
    // Default values
    unsigned int n_scanners  = 1, 
                 n_analyzers = 1, 
                 max_buff_entries = 8, 
                 debug = 0;

    char *source, *destination, *previous;


    int opt;
    while ((opt = getopt (argc, argv, "hns:a:f:")) != -1){
        switch (opt){
            case 'h': usage(argv[0], 0);
                break;
            case 'n': debug = 1;
                break;
            case 's': n_scanners = atoi(optarg); // TODO atoi
                break ;
            case 'a': n_analyzers = atoi(optarg); // TODO atoi
                break ;
            case 'f': max_buff_entries = atoi(optarg); // TODO atoi
                break ;
            default: usage(argv[0], 1);
        }
    }

    switch (argc - optind){
        case 2: 
            source = argv[optind];
            previous = NULL;
            destination = argv[optind+1];
            break ;
        case 3 :        
            source = argv[optind];
            previous = argv[optind+1];
            destination = argv[optind+1];
            break ;
        default : usage(argv[0], 1);
    }
    
    launch_scanners(n_scanners, source);
   
    return 0;
}