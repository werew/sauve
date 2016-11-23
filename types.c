#include <stdlib.h>
#include "types.h"

struct list* create_list(){
    struct list* l = calloc(1, sizeof(struct list));
    return l;
}

void* list_push(struct list* l, void* data){
    struct cell* new = malloc(sizeof(struct cell)); 
    if (new == -1) return NULL;
    
    new->data = data;
    new->next = NULL;

    if (l->head == NULL) l->head = new;
    l->foot = next;

    return data;
}

void* list_pop(struct list* l){
    struct cell* h = l->head;
    if (h == NULL) return NULL;

    l->head = h->next;
    void* data = h->data; 
    free(h);

    return data;
}


