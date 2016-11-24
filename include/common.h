#ifndef _COMMON_H_
#define _COMMON_H_

#include "types.h"

// Global variables
struct list* term_queue;
struct list* folders_queue;
struct ring_buf* files_queue;

#define fail(x) _fail(x, __LINE__, __func__)
void _fail(const char* msg, int line, const char* func);
void exit_thread(void* ret);


#endif
