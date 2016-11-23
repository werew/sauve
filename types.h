#ifndef _TYPES_H_
#define _TYPES_H_


/**
 * Ring buffer
 */
struct ring_buf {
    unsigned int first;
    unsigned int total;
    unsigned int max;
    void* buf[];
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
void list_push(struct list* l, void* data);
void* list_pop(struct list* l);

struct ring_buf* create_ring_buf();
void ring_buf_push(struct ring_buf* rb, void* data);
void* ring_buf_pop(struct ring_buf* rb);


#endif
