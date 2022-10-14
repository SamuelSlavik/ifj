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
#include "dynamic_buffer.h"


tDynamicBuffer* dynamicBuffer_INIT() {
    
    tDynamicBuffer* buffer = malloc(sizeof(tDynamicBuffer));

    if (buffer == NULL){
        return NULL;
    }
    else {
        buffer->data = malloc(BUFFER_SIZE);
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
        buffer->size++;
        buffer->data[buffer->size] = c;
        return true;
    }
    else {
        char* tmp;
        tmp = realloc(buffer->data, buffer->allocated_size + BUFFER_SIZE);
        if (tmp == NULL) {
            return false;
        }
        else {
            buffer->size++;
            buffer->allocated_size += BUFFER_SIZE;
            buffer->data = tmp;
            buffer->data[buffer->size] = c;
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


#ifndef NOT_TESTING

int main(){

    tDynamicBuffer* buffer = dynamicBuffer_INIT();

    if (buffer == NULL){
        printf("Chyba alokácie !");
    }

    int c;
    bool err;

    while ((c = getchar()) != EOF) {
        err = dynamicBuffer_ADD_CHAR(buffer, c);
        if (err == false) {
            printf("Chyba bufferu !\n");
        }
        
        for (unsigned long i = 0; buffer->size > i; i++){
            printf("%c", buffer->data[i]);
        }
        printf("\n");
    }

    dynamicBufferFREE(buffer);

    return 0;
}


#endif // NOT_TESTING