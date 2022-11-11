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


/**
 * @brief Initialization of new Stack
 * 
 * @param s pointer to Stack
 */
void StackInit(tStack *s){
    s->top = NULL;
}


/**
 * @brief Function push data to top of the Stack
 * 
 * @param s pointer to Stack
 * @param data 
 * @return true if function success
 * @return false if function fail
 */
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


/**
 * @brief Function remove element on top of the Stack
 * 
 * @param s pointer to Stack
 */
void StackPop(tStack *s){
    tStack_item *tmp;
    if (s->top != NULL){
        tmp = s->top;
        s->top = s->top->next_item;
        free(tmp);
    }
}


/**
 * @brief Function for getting data from top of the Stack
 * 
 * @param s pointer to Stack
 * @return void* pointer to data stored in top element of Stack or NULL if Stack is empty
 */
void *StackTop(tStack *s){
    if (s->top != NULL){
        return (s->top->data);
    }
    return NULL;
}


/**
 * @brief Function Check if Stack is empty
 * 
 * @param s pointer to Stack
 * @return true if stack is empty
 * @return false if stack is not empty
 */
bool StackIsEmpty(tStack *s){
    return (s->top == NULL);
}