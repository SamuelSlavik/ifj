#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dll_instruction_list.h"
#include "dynamic_buffer.h"

void DLL_Init( DLList *list ) {
	list->first = NULL;
	list->active = NULL;
	list->last = NULL;
}

void DLL_Dispose( DLList *list ) {
    while (list->first != NULL){
        list->active = list->first;
        list->first = list->first->nextElement;
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

void DLL_InsertLast( DLList *list, tDynamicBuffer *instruction ) {
    DLL_instruction *new_element = malloc(sizeof(DLL_instruction));
    if (new_element == NULL){
        
        return;
    }
    new_element->instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(new_element->instruction, instruction->data);
    new_element->nextElement = NULL;
    new_element->previousElement = list->last;

    if (list->last == NULL){
        list->first = new_element;
    } else{
        list->last->nextElement = new_element;
    }

    list->last = new_element;
}

void DLL_First( DLList *list ) {
	list->active = list->first;
}

void DLL_Last( DLList *list ) {
	list->active = list->last;
}

void DLL_GetFirst( DLList *list, tDynamicBuffer *instruction ) {
	if (list->first == NULL){
        
        return;
    }
    instruction = list->first->instruction;
}

void DLL_GetLast( DLList *list, tDynamicBuffer *instruction ) {
    if (list->last == NULL){
        
        return;
    }
    instruction = list->last->instruction;
}

void DLL_DeleteFirst( DLList *list ) {
	if (list->first != NULL){
        DLL_instruction * temp;
        temp = list->first;

        if (list->first == list->active){
            list->active = NULL;
        }

        if (list->first == list->last){
            list->first = NULL;
            list->last = NULL;
        } else{
            list->first = list->first->nextElement;
            list->first->previousElement = NULL;
        }

        free(temp);
    }
}

void DLL_DeleteLast( DLList *list ) {
    if (list->first != NULL || list->last != NULL){
        DLL_instruction * temp;
        temp = list->last;

        if (list->last == list->active){
            list->active = NULL;
        }

        if (list->last == list->first){
            list->first = NULL;
            list->last = NULL;
        } else{
            list->last = list->last->previousElement;
            list->last->nextElement = NULL;
        }

        free(temp);
    }
}

void DLL_DeleteAfter( DLList *list ) {
    // Continue only if list is active and active element is not last element
	if (list->active != NULL && list->last != list->active){
        DLL_instruction * tmp;
        tmp = list->active->nextElement; // Temporarily store element after active one
        list->active->nextElement = tmp->nextElement; // Set next element as element after one being deleted

        if (tmp == list->last){
            // If last element is being deleted, active element will be last after
            list->last = list->active;
        } else{
            // If deleted element was not last, connect element after with active element (element before)
            tmp->nextElement->previousElement = list->active;
        }

        free(tmp);

    }
}

void DLL_DeleteBefore( DLList *list ) {
    // Continue only if list is active and active element is not first element
    if (list->active != NULL && list->first != list->active){
        DLL_instruction * tmp;
        tmp = list->active->previousElement; // Temporarily store element before active one
        list->active->previousElement = tmp->previousElement; // Set previous element as element before one being deleted

        if (tmp == list->first){
            // If first element is being deleted, active element will be first after
            list->first = list->active;
        } else{
            // If deleted element was not first, connect element before with active element (element before)
            tmp->previousElement->nextElement = list->active;
        }

        free(tmp);

    }
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

void DLL_InsertBefore( DLList *list, tDynamicBuffer *instruction ) {
    // If list has no active element do nothing
    if (list->active != NULL){
        // Allocate space for new element and check whether it was successful
        DLL_instruction *new_element = malloc(sizeof(DLL_instruction));
        if (new_element == NULL){
            
            return;
        }
        new_element->instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(new_element->instruction, instruction->data);
        new_element->nextElement = list->active;
        new_element->previousElement = list->active->previousElement;
        list->active->previousElement = new_element;

        // If active element was also first move pointer to last element
        if (list->active == list->first){
            list->first = new_element;
        } else{
            new_element->previousElement->nextElement = new_element;
        }
    }
}

void DLL_GetValue( DLList *list, tDynamicBuffer *instruction ) {
    // If list has no active element return with error
	if (list->active == NULL){
        
        return;
    }

    instruction = list->active->instruction;
}

void DLL_SetValue( DLList *list, tDynamicBuffer *instruction ) {
    // If list has no active element do nothing
	if (list->active != NULL){
        list->active->instruction = instruction;
    }
}

void DLL_Next( DLList *list ) {
    // If list has no active element do nothing
	if (list->active != NULL){
        list->active = list->active->nextElement;
    }
}

void DLL_Previous( DLList *list ) {
    // If list has no active element do nothing
    if (list->active != NULL){
        list->active = list->active->previousElement;
    }
}

int DLL_IsActive( DLList *list ) {
	return list->active != NULL;
}
