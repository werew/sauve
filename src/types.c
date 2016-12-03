#include <stdlib.h>
#include <stdio.h>
#include "types.h"


/**
 * Creates an empty list
 * @return A pointer to an empty list or NULL in case of error
 */
struct list* create_list(){
    struct list* l = calloc(1, sizeof(struct list));
    return l;
}


/**
 * Adds a new element at the top of the list
 * @param l A pointer to a list
 * @param data A pointer to the element to add
 * @return 0 in case of success, -1 otherwise
 */
int list_push(struct list* l, void* data){
    struct cell* new = malloc(sizeof(struct cell)); 
    if (new == NULL) return -1;
    
    new->data = data;
    new->next = NULL;

    if (l->head == NULL) l->head = new;
    if (l->foot != NULL) l->foot->next = new;

    l->foot = new;

    return 0;
}

/**
 * Pop an element from the end of a list
 * @param l A pointer to a list 
 * @return The last element of the list, of NULL in case of error
 */
void* list_pop(struct list* l){
    struct cell* h = l->head;
    if (h == NULL) return NULL;

    l->head = h->next;
    if (l->foot == h) l->foot = h->next;

    void* data = h->data; 
    free(h);

    return data;
}



/**
 * Creates an empty ring buffer
 * @return A pointer to an empty ring buffer or NULL in case of error
 */
struct ring_buf* create_ring_buf(unsigned int max_elements){
    size_t s = sizeof(struct ring_buf) + sizeof(void*) * max_elements;
    struct ring_buf* rb = calloc(1,s);
    if (rb == NULL) return NULL;

    rb->max = max_elements;
    return rb;
}


/**
 * Adds a new element to a ring buffer
 * @param rb A pointer to a ring buffer
 * @param data A pointer to the element to add
 * @return 0 in case of success, -1 otherwise
 */
int ring_buf_push(struct ring_buf* rb, void* data){
    if (rb->total >= rb->max) return -1;

    int n = (rb->first+rb->total) % rb->max;
    rb->buf[n] = data;
    rb->total++;

    return 0;
}


/**
 * Pops an the last element (least recent) of a ring buffer
 * @param rb A pointer to a ring buffer 
 * @return The last element of the buffer or NULL in case of error
 */
void* ring_buf_pop(struct ring_buf* rb){
    if (rb->total == 0) return NULL;

    void* data = rb->buf[rb->first];
    rb->first = (rb->first+1) % rb->max;
    rb->total--;

    return data;
}
