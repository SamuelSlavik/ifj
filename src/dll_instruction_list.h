#ifndef DDL_INSTRUCTION_LIST
#define DDL_INSTRUCTION_LIST

#include "dynamic_buffer.h"
#include "htab.h"

typedef struct DLL_instruction_list_item {
	tDynamicBuffer *instruction;
	htab_data_t *curr_var;
	htab_data_t *curr_fun;
	struct DLL_instruction_list_item *previousElement;
	struct DLL_instruction_list_item *nextElement;
} DLL_instruction;

typedef struct {
	DLL_instruction *first;
	DLL_instruction *active;
	DLL_instruction *last;
} DLList;

void DLL_Init( DLList * );

void DLL_Dispose( DLList * );

void DLL_InsertFirst( DLList *, tDynamicBuffer * );

void DLL_InsertLast( DLList *, tDynamicBuffer * );

void DLL_First( DLList * );

void DLL_Last( DLList * );

void DLL_GetFirst( DLList *, tDynamicBuffer * );

void DLL_GetLast( DLList *, tDynamicBuffer * );

void DLL_DeleteFirst( DLList * );

void DLL_DeleteLast( DLList * );

void DLL_DeleteAfter( DLList * );

void DLL_DeleteBefore( DLList * );

void DLL_InsertAfter( DLList *, tDynamicBuffer * );

void DLL_InsertBefore( DLList *, tDynamicBuffer * );

void DLL_GetValue( DLList *, tDynamicBuffer * );

void DLL_SetValue( DLList *, tDynamicBuffer * );

void DLL_Next( DLList * );

void DLL_Previous( DLList * );

int DLL_IsActive( DLList * );

#endif //DDL_INSTRUCTION_LIST