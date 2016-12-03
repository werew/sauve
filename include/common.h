#ifndef _COMMON_H_
#define _COMMON_H_

#include "types.h"

#ifndef _COMMON_C_ 
    #define _EXTERN_ extern
#else
    #define _EXTERN_ 
#endif


#ifdef SENV
#error "Macro SENV is already defined"
#endif

/**
 * @brief Global environement of the program
 */
_EXTERN_ struct {
    int debug_opt;              // 1 if the -n option was set

    const char* source;         // Source folder

    const char* destination;    // Destination folder

    const char* previous;       // Previous copy (could be NULL)
} _sauve_env_;
#define SENV _sauve_env_



/**
 * @brief A simple FIFO used to be able to join the
 *        terminated threads in the good order.
 */
_EXTERN_ struct {
    struct list* list;          // List of the pthread_t 

    pthread_mutex_t mutex;      // Guarantees safe write/read access 

    pthread_cond_t  push_cond;  // Dispatched when a new thread has 
                                // been pushed
} term_queue;



/**
 * @brief Syncronized folders queue for the scanners
 */
_EXTERN_ struct {
    struct list* list;          // The folders buffer

    int active_scanners;        // Number of active scanners. A value
                                // of -1 shall indicate that all the
                                // threads have terminated or are
                                // terminating

    pthread_mutex_t mutex;      // Guarantees safe write/read access 

    pthread_cond_t pt_cond;     // Dispatched when a new folder is 
                                // available or when no more folders
                                // need to be processed
} folders_queue;


    
/**
 * @brief Syncronized files queue for the analyzers
 */
_EXTERN_ struct {
    struct ring_buf* buf;       // The files buffer

    int receiving;              // Indicates whether the buffer will
                                // receive data in the future or not

    pthread_mutex_t mutex;      // Guarantees safe write/read access

    pthread_cond_t read;        // Condition to wait if the buffer is 
                                // empty when trying to read

    pthread_cond_t write;       // Condition to wait if the buffer is 
                                // full when trying to write
} files_queue;




/* Fast and dirty solution to reduce the verbosity of the 
   controls that should be done after the execution of some
   functions of the pthread's family
*/
#define PT_CHK(call) {if ((errno=(call)) != 0) fail("");}

#define fail(x) _fail(x, __LINE__, __func__)
void _fail(const char* msg, int line, const char* func);

void signal_term();
char* change_base
(const char* path, const char* old_basedir, const char* new_basedir);


#endif
