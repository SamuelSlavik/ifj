#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"
#include "dll_instruction_list.h"

bool f_start(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_prog(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

bool f_body(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

//bool f_body_as(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

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

bool f_func_mparam(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

#endif //__PARSER_H__
