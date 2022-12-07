/**
 * @file stack.c
 * @author Samuel Slávik (xslavi37)
 * @brief Implemantation of Stack data type storing data type void *
 */

#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"


void StackInit(tStack *s) {
    s->top = NULL;
}

bool StackPush(tStack *s, void *data) {
    tStack_item *new = (tStack_item *) malloc(sizeof(tStack_item));
    if (new == NULL){ // if malloc fails
        return false;
    }
    new->data = data;
    new->next_item = s->top; // move current top item as next to the top
    s->top = new; // set new item as stack's top
    return true;
}

void StackPop(tStack *s) {
    tStack_item *tmp;
    if (s->top != NULL){ // if exists
        tmp = s->top;
        s->top = s->top->next_item; // set next to the top item as the stack's top
        free(tmp);
    }
}

void *StackTop(tStack *s) {
    if (s->top != NULL){ // if exists
        return (s->top->data);
    }
    return NULL;
}

bool StackIsEmpty(tStack *s) {
    return (s->top == NULL);
}
