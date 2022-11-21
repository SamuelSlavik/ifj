/**
 * @file dynamic_buffer.h
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.1
 * @date 2022-10-13
 * 
 */

#include <stdio.h>
#include <stdbool.h>

#ifndef DYNAMIC_BUFFER_H
#define DYNAMIC_BUFFER_H


#define BUFFER_SIZE 8

/**
 * @brief Dynamic Buffer structure
 * 
 */
typedef struct dynamic_buffer
{
    char* data;
    unsigned long size;
    unsigned long allocated_size;
} tDynamicBuffer;



/**
 * @brief Initialization of dynamicly allocated buffer
 * 
 * @return tDynamicBuffer* pointer to dynamicly allocated buffer
 */
tDynamicBuffer* dynamicBuffer_INIT();


/**
 * @brief Function add one char to dynamic buffer
 * 
 * @param buffer buffer where char will be added
 * @param c char that will be added to buffer
 * @return true if success
 * @return false if fail
 */
bool dynamicBuffer_ADD_CHAR(tDynamicBuffer* buffer ,char c);


/**
 * @brief Function free dynamicly allocated memmory in dynamic buffer
 * 
 * @param buffer buffer that will be freed
 */
void dynamicBufferFREE(tDynamicBuffer* buffer);



/**
 * @brief Function add one char to dynamic buffer
 * 
 * @param buffer buffer where char will be added
 * @param c char that will be added to buffer
 * @return true if success
 * @return false if fail
 */
void dynamicBuffer_ADD_STRING(tDynamicBuffer* buffer ,char *c);


#endif // DYNAMIC_BUFFER_H