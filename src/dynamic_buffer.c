/**
 * @file dynamic_buffer.c
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.1
 * @date 2022-10-13
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "dynamic_buffer.h"


tDynamicBuffer* dynamicBuffer_INIT() {
    
    tDynamicBuffer* buffer = malloc(sizeof(tDynamicBuffer));

    if (buffer == NULL){
        return NULL;
    }
    else {
        buffer->data = calloc(BUFFER_SIZE, sizeof(char));
        if (buffer->data == NULL) {
            free(buffer);
            return NULL;
        }
        else {
            buffer->size = 0;
            buffer->allocated_size = BUFFER_SIZE;
            return buffer;
        }
    }
}


bool dynamicBuffer_ADD_CHAR(tDynamicBuffer* buffer ,char c) {

    if (buffer == NULL){
        return false;
    }

    if (buffer->size + 1 < buffer->allocated_size){
        buffer->data[buffer->size] = c;
        buffer->size++;
        return true;
    }
    else {
        char* tmp;
        tmp = realloc(buffer->data, buffer->allocated_size * 2);
        if (tmp == NULL) {
            return false;
        }
        else {
            buffer->data = tmp;
            buffer->allocated_size *= 2;
            buffer->data[buffer->size] = c;
            buffer->size++;
            return true;
        }
    }
}


void dynamicBufferFREE(tDynamicBuffer* buffer){
    if (buffer != NULL) {
        free(buffer->data);
        free(buffer);
    }
}

void dynamicBuffer_ADD_STRING(tDynamicBuffer* buffer, const char *c){ // magia skontrolovat
    size_t len = strlen(c); 
    if (buffer->allocated_size <= buffer->size + len + 1){
        buffer->data = realloc(buffer->data, buffer->allocated_size + len + 1);
        buffer->allocated_size += len + 1;
    }
    buffer->size += len + 1;
    strcat(buffer->data, c);
    
}

tDynamicBuffer *dynamicBuffer_RESET(tDynamicBuffer* buffer){
    dynamicBufferFREE(buffer);
    return dynamicBuffer_INIT();
}


#ifdef TESTING

int main(){

    tDynamicBuffer* buffer = dynamicBuffer_INIT();

    if (buffer == NULL){
        fprintf(stderr, "Allocation fail !\n");
    }

    int c;
    bool err;

    while ((c = getchar()) != EOF) {
        err = dynamicBuffer_ADD_CHAR(buffer, c);
        if (err == false) {
            fprintf(stderr, "Buffer allocation fail !\n");
        }
        for (unsigned long i = 0; (buffer->size - 1) > i; i++){
            printf("%c", buffer->data[i]);
        }
        printf("\n");
    }

    dynamicBufferFREE(buffer);

    return 0;
}


#endif // TESTING