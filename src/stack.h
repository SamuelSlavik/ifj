/**
 * @file stack.h
 * @author Michal Ľaš (xlasmi00)
 * @brief Implemantation of Stack data type storing data type void *
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



/**
 * @brief Initialization of new Stack
 * 
 * @param s pointer to Stack
 */
void StackInit(tStack *s);


/**
 * @brief Function push data to top of the Stack
 * 
 * @param s pointer to Stack
 * @param data 
 * @return true if function success
 * @return false if function fail
 */
bool StackPush(tStack *s, void *data);


/**
 * @brief Function remove element on top of the Stack
 * 
 * @param s pointer to Stack
 */
void StackPop(tStack *s);


/**
 * @brief Function for getting data from top of the Stack
 * 
 * @param s pointer to Stack
 * @return void* pointer to data stored in top element of Stack or NULL if Stack is empty
 */
void *StackTop(tStack *s);


/**
 * @brief Function Check if Stack is empty
 * 
 * @param s pointer to Stack
 * @return true if stack is empty
 * @return false if stack is not empty
 */
bool StackIsEmpty(tStack *s);


#endif // STACK_H