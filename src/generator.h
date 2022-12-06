#ifndef GENERATOR_H
#define GENERATOR_H
#include <stdlib.h>
#include "dll_instruction_list.h"
#include "dynamic_buffer.h"
#include "stack.h"

void generate_write(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_reads(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_readi(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_readf(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_intval(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_floatval(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_strval(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_strlen(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_substr(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_chr(tDynamicBuffer *instruction, DLList *instruction_list);
void generate_ord(tDynamicBuffer *instruction, DLList *instruction_list);
void check_fn_arguments(tStack *defined_params, tStack *called, tDynamicBuffer *instruction, DLList *instruction_list);
void convert_into_bool(tDynamicBuffer *instruction, DLList *instruction_list,tDynamicBuffer *labelname);
void check_return_type(tDynamicBuffer *instruction, DLList *instruction_list);
void print_stack(tStack *expr_stack, tDynamicBuffer *instruction, DLList *instruction_list,char *code);
void print_instructions(DLList *instruction_list);

#endif