/**
 * @file stack.h
 * @author Michal Ľaš (xlasmi00)
 * @brief Implemantation of Stack storing data type void *
 * @version 0.1
 * @date 2022-11-11
 * 
 */


#ifndef STACK_H
#define STACK_H

typedef struct stack_item tStack_item;

struct stack_item
{
    void *data;
    tStack_item *next_item;
};


typedef struct stack
{
    tStack_item *top;
} tStack;



#endif // STACK_H