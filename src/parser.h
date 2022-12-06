#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"
#include "dll_instruction_list.h"

#define ERROR_EXIT(flag,token,errcode) if (!(flag)){error_exit(token,errcode);}
#define DETECT_MAIN(instruction_list,instruction,key) if(!strcmp(key,"$$main")){DLL_InsertAfter_main(instruction_list,instruction);\
if(instruction_list->active==instruction_list->main_body)\
{DLL_Next(instruction_list);}\
DLL_Next_main(instruction_list);}\
else{DLL_InsertAfter(instruction_list,instruction); DLL_Next(instruction_list);}

bool f_start(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_prog(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_body(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_body_var(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_body_ret(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_fn_call_l(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_fn_call_lc(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_fn_call_lparam(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list);

bool f_func(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_func_dedf(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_func_type(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_in_body(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_func_param(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_func_dedf_param_type(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list);
bool f_func_dedf_param_var(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list);
#endif //__PARSER_H__
