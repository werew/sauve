#ifndef _TYPES_H_
#define _TYPES_H_


/**
 * Ring buffer
 */
#define MAX_BUF_ENTRIES 100
struct ring_buf {
    void* buf[MAX_BUF_ENTRIES]; 
    int first;
    int last;
}


/**
 * List
 */
struct cell {
    void* data;
    struct cell* next;
}

struct list {
    struct cell* head;
    struct cell* foot;
}



struct list* create_list();
void* list_push(struct list* l, void* data);
void* list_pop(struct list* l);


#endif
