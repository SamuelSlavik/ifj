#ifndef DDL_INSTRUCTION_LIST
#define DDL_INSTRUCTION_LIST

#include "dynamic_buffer.h"
#include "htab.h"

typedef struct DLL_instruction_list_item {
	tDynamicBuffer *instruction;
	struct DLL_instruction_list_item *previousElement;
	struct DLL_instruction_list_item *nextElement;
} DLL_instruction;

typedef struct {	
	htab_data_t *curr_var;
	htab_data_t *curr_fun;
	htab_data_t *called_from;
	DLL_instruction *first;
	DLL_instruction *active;
	DLL_instruction *main_body;
	DLL_instruction *last;
	DLL_instruction *if_while;
	char *label; //ifwhilelabel
	size_t num_of_params_called;
	tStack called_args;
} DLList;

void DLL_Init( DLList * );

void DLL_Dispose( DLList * );

void DLL_InsertFirst( DLList *, tDynamicBuffer * );

void DLL_First( DLList * );

void DLL_InsertAfter( DLList *, tDynamicBuffer * );

void DLL_Next( DLList * );

int DLL_IsActive( DLList * );
void DLL_InsertAfter_main( DLList *, tDynamicBuffer *);
void DLL_InsertBefore_if_while( DLList *, tDynamicBuffer *);
void DLL_First_main( DLList * );
void DLL_Next_main( DLList * );

#endif //DDL_INSTRUCTION_LIST