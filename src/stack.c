/**
 * @file stack.c
 * @author Michal Ľaš (xlasmi00)
 * @brief Implemantation of Stack data type storing data type void *
 * @version 0.1
 * @date 2022-11-11
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"



void StackInit(tStack *s){
    s->top = NULL;
}


bool StackPush(tStack *s, void *data){
    tStack_item *new = (tStack_item *) malloc(sizeof(tStack_item));
    if (new == NULL){
        return false;
    }
    new->data = data;
    new->next_item = s->top;
    s->top = new;
    return true;
}


void StackPop(tStack *s){
    tStack_item *tmp;
    if (s->top != NULL){
        tmp = s->top;
        s->top = s->top->next_item;
        free(tmp);
    }
}


void *StackTop(tStack *s){
    if (s->top != NULL){
        return (s->top->data);
    }
    return NULL;
}


bool StackIsEmpty(tStack *s){
    return (s->top == NULL);
}

int main(){

    return 0;
}