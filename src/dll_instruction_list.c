#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dll_instruction_list.h"
#include "dynamic_buffer.h"

void DLL_Init( DLList *list ) {
	list->first = NULL;
	list->active = NULL;
	list->last = NULL;
    list->called_from=NULL;
    list->curr_fun = NULL;
    list->curr_var = NULL;
    list->if_while = NULL;
    list->label = NULL;
    list->main_body = NULL;
}

void DLL_Dispose( DLList *list ) {
    while (list->first != NULL){
        list->active = list->first;
        list->first = list->first->nextElement;
        if (list->active->instruction != NULL){
            dynamicBufferFREE(list->active->instruction);
        }
        free(list->active);
    }
    list->last = NULL;
    list->active = NULL;
}

void DLL_InsertFirst( DLList *list, tDynamicBuffer *instruction ) {
	DLL_instruction *new_element = malloc(sizeof(DLL_instruction));
    if (new_element == NULL){
        
        return;
    }
    new_element->instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(new_element->instruction, instruction->data);
    new_element->previousElement = NULL;
    new_element->nextElement = list->first;

    if (list->first == NULL){
        list->last = new_element;
    } else{
        list->first->previousElement = new_element;
    }

    list->first = new_element;
}


void DLL_First( DLList *list ) {
	list->active = list->first;
}

void DLL_First_main( DLList *list ) {
	list->main_body = list->first;
}

void DLL_InsertAfter( DLList *list, tDynamicBuffer *instruction ) {
    // If list has no active element do nothing
    if (list->active != NULL){
        // Allocate space for new element and check whether it was successful
        DLL_instruction *new_element = malloc(sizeof(DLL_instruction));
        if (new_element == NULL){
            
            return;
        }
        new_element->instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(new_element->instruction, instruction->data);
        new_element->nextElement = list->active->nextElement;
        new_element->previousElement = list->active;
        list->active->nextElement = new_element;

        // If active element was also last move pointer to last element
        if (list->active == list->last){
            list->last = new_element;
        } else{
            new_element->nextElement->previousElement = new_element;
        }
    }
}

void DLL_InsertAfter_main( DLList *list, tDynamicBuffer *instruction ) {
    // If list has no active element do nothing
    if (list->main_body != NULL){
        // Allocate space for new element and check whether it was successful
        DLL_instruction *new_element = malloc(sizeof(DLL_instruction));
        if (new_element == NULL){
            
            return;
        }
        new_element->instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(new_element->instruction, instruction->data);
        new_element->nextElement = list->main_body->nextElement;
        new_element->previousElement = list->main_body;
        list->main_body->nextElement = new_element;

        // If active element was also last move pointer to last element
        if (list->main_body == list->last){
            list->last = new_element;
        } else{
            new_element->nextElement->previousElement = new_element;
        }
    }
}

void DLL_InsertBefore_if_while( DLList *list, tDynamicBuffer *instruction ) {
    // If list has no active element do nothing
    if (list->if_while != NULL){
        // Allocate space for new element and check whether it was successful
        DLL_instruction *new_element = malloc(sizeof(DLL_instruction));
        if (new_element == NULL){
            
            return;
        }
        new_element->instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(new_element->instruction, instruction->data);
        new_element->nextElement = list->if_while;
        new_element->previousElement = list->if_while->previousElement;
        list->if_while->previousElement = new_element;

        // If active element was also first move pointer to last element
        if (list->if_while == list->first){
            list->first = new_element;
        } else{
            new_element->previousElement->nextElement = new_element;
        }
    }
}

void DLL_Next( DLList *list ) {
    // If list has no active element do nothing
	if (list->active != NULL){
        list->active = list->active->nextElement;
    }
}

void DLL_Next_main( DLList *list ) {
    // If list has no active element do nothing
	if (list->main_body != NULL){
        list->main_body = list->main_body->nextElement;
    }
}

int DLL_IsActive( DLList *list ) {
	return list->active != NULL;
}
