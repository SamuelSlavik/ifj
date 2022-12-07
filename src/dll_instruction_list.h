/**
 * @file dll_instruction_list.h
 * @brief Implementation of double linked list for manipulation with instructions
 * @author Jakub Kontrík (xkontr00)  
 */

#ifndef DDL_INSTRUCTION_LIST
#define DDL_INSTRUCTION_LIST

#include "dynamic_buffer.h"
#include "htab.h"

typedef struct DLL_instruction_list_item {
	tDynamicBuffer *instruction; //ifjcode 22 instruction
	struct DLL_instruction_list_item *previousElement;
	struct DLL_instruction_list_item *nextElement;
} DLL_instruction;

typedef struct {	
	htab_data_t *curr_var; //pointer to current variable that is currently being processed
	htab_data_t *curr_fun; //pointer to current function that is currently being processed
	htab_data_t *called_from; //pointer to function from where is instruction called
	DLL_instruction *first; //pointer to first element
	DLL_instruction *active; //pointer to active element
	DLL_instruction *main_body; //pointer to last main body element
	DLL_instruction *last; //pointer to last element
	DLL_instruction *if_while; //pointer to if while instruction that is on top
	char *label;  // name of if while label that is on top
	size_t num_of_params_called; //numbers of parameters called
	tStack called_args; // stack with called arguments
} DLList;

/**
 * @brief Initialization of double linked list
 * 
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_Init( DLList *instruction_list);

/**
 * @brief Disposes list and free all allocated memory
 * 
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_Dispose( DLList *instruction_list);

/**
 * @brief Adds new element at the begining of the list
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_InsertFirst( DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Sets first element as active
 * 
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_First( DLList *instruction_list );

/**
 * @brief Inserts element after active element
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_InsertAfter( DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Sets activity on the next element
 * 
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_Next( DLList *instruction_list );

/**
 * @brief Checks activity of given element
 * 
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 *
 * @return 0 value if the element is not active or non-zero value if the element is active
 */
int DLL_IsActive( DLList *instruction_list );

/**
 * @brief Inserts element after last element from main body
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_InsertAfter_main( DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Inserts element before instruction that contains current if or while
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_InsertBefore_if_while( DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Sets first element as main body element
 * 
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_First_main( DLList *instruction_list );

/**
 * @brief Sets pointer on main body to the next element
 * 
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void DLL_Next_main( DLList *instruction_list );

#endif //DDL_INSTRUCTION_LIST
