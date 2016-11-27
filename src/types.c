#include <stdlib.h>
#include <stdio.h>
#include "types.h"

struct list* create_list(){
    struct list* l = calloc(1, sizeof(struct list));
    return l;
}

int list_push(struct list* l, void* data){
    puts("---> PUSH");
    printf("list_push %s\n", data);
    struct cell* new = malloc(sizeof(struct cell)); 
    if (new == NULL) return -1;
    
    new->data = data;
    new->next = NULL;

    if (l->head == NULL) l->head = new;
    if (l->foot != NULL) l->foot->next = new;

    l->foot = new;

    return 0;
}

void print_list(struct list* l){
    printf("   PRINT LIST\n");
    struct cell* c = l->head;
    while (c != NULL){
        printf("   %s\n",(char*) c->data);
        c = c->next;
    }
}

void* list_pop(struct list* l){
    puts("---> POP");
    struct cell* h = l->head;
    if (h == NULL) return NULL;

    l->head = h->next;
    if (l->foot == h) l->foot = h->next;

    void* data = h->data; 
    free(h);

    printf("list_pop %s\n", data);
    return data;
}

struct ring_buf* create_ring_buf(unsigned int max_elements){
    size_t s = sizeof(struct ring_buf) + sizeof(void*) * max_elements;
    struct ring_buf* rb = calloc(1,s);
    if (rb == NULL) return NULL;

    rb->max = max_elements;
    return rb;
}

int ring_buf_push(struct ring_buf* rb, void* data){
    if (rb->total >= rb->max) return -1;

    printf("tot: %d push file %s\n",rb->total,data);

    int n = (rb->first+rb->total) % rb->max;
    rb->buf[n] = data;
    rb->total++;

    return 0;
}


void* ring_buf_pop(struct ring_buf* rb){
    if (rb->total == 0) return NULL;

    void* data = rb->buf[rb->first];
    rb->first = (rb->first+1) % rb->max;
    rb->total--;

    return data;
}
