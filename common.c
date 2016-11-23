#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "common.h"

void _fail(const char* msg, int line, const char* func){
    fprintf(stderr, "Error at line %d (function %s):\n",line,func);
    perror(msg);
    exit(EXIT_FAILURE);
}
