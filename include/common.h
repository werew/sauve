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

_EXTERN_ struct {
    struct list* list;
    pthread_mutex_t mutex;      // Guarantees safe write/read access 
    pthread_cond_t  push_cond;  // Dispatched when a new thread has been pushed
} term_queue;

_EXTERN_ struct {
    struct list* list;
    unsigned int active_threads;
    pthread_mutex_t mutex;      // Guarantees safe write/read access 
    pthread_cond_t pt_cond;     // Dispatched when a new folder is available or
                                // if no more folders need to be processed
} folders_queue;
    
_EXTERN_ struct ring_buf* files_queue;

#define fail(x) _fail(x, __LINE__, __func__)
void _fail(const char* msg, int line, const char* func);
void signal_term();

#define PT_CHK(call) {if ((errno=(call)) != 0) fail("");}

#endif
