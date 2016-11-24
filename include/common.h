#ifndef _COMMON_H_
#define _COMMON_H_

#include "types.h"

// Global variables
#ifndef _COMMON_C_ 
    #define _EXTERN_ extern
#else
    #define _EXTERN_ 
#endif

_EXTERN_ int debug_opt;
_EXTERN_ struct list* term_queue;
_EXTERN_ struct list* folders_queue;
_EXTERN_ struct ring_buf* files_queue;

#define fail(x) _fail(x, __LINE__, __func__)
void _fail(const char* msg, int line, const char* func);
void exit_thread(void* ret);


#endif
