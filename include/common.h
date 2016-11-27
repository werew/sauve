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
_EXTERN_ int active_scanners;

_EXTERN_ struct {
    struct list* list;
    pthread_mutex_t mutex;      // Guarantees safe write/read access 
    pthread_cond_t  push_cond;  // Dispatched when a new thread has been pushed
} term_queue;

_EXTERN_ struct {
    struct list* list;
    pthread_mutex_t mutex;      // Guarantees safe write/read access 
    pthread_cond_t pt_cond;     // Dispatched when a new folder is available or
                                // if no more folders need to be processed
} folders_queue;
    
_EXTERN_ struct {
    struct ring_buf* buf;
    int receiving;
    pthread_mutex_t mutex;      // Guarantees safe write/read access
    pthread_cond_t read;        // Condition to wait if buf is empty at reading
    pthread_cond_t write;       // Condition to wait if buf is full at writing
} files_queue;

#define fail(x) _fail(x, __LINE__, __func__)
void _fail(const char* msg, int line, const char* func);
void signal_term();

#define PT_CHK(call) {if ((errno=(call)) != 0) fail("");}

#endif
