#include <stdlib.h>
#include <stdio.h>
#include "types.h"

struct list* create_list(){
    struct list* l = calloc(1, sizeof(struct list));
    return l;
}

int list_push(struct list* l, void* data){
    struct cell* new = malloc(sizeof(struct cell)); 
    if (new == NULL) return -1;
    
    new->data = data;
    new->next = NULL;

    if (l->head == NULL) l->head = new;
    l->foot->next = new;
    l->foot = new;

    return 0;
}

void* list_pop(struct list* l){
    struct cell* h = l->head;
    if (h == NULL) return NULL;

    l->head = h->next;
    void* data = h->data; 
    free(h);

    return data;
}

struct ring_buf* create_ring_buf(unsigned int max_elements){
    size_t s = sizeof(struct ring_buf) + sizeof(void*) * max_elements;
    struct ring_buf* rb = calloc(1,s);
    if (rb == NULL) return NULL;

    rb->max = max_elements;
    return rb;
}

void ring_buf_push(struct ring_buf* rb, void* data){
    if (rb->total >= rb->max){
        // TODO
        printf("wait to push\n");
    }
    int n = (rb->first+rb->total) % rb->max;
    rb->buf[n] = data;
    rb->total++;
}


void* ring_buf_pop(struct ring_buf* rb){
    if (rb->total == 0){
        // TODO ???
        printf("wait to pop\n");
    }

    void* data = rb->buf[rb->first];
    rb->first = (rb->first+1) % rb->max;

    return data;
}
