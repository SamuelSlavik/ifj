/**
 * @file expression_codegen.h
 * @author Adam Pekný (xpekny00)
 * @brief
 * @version 0.1
 * @date 2022-11-18
 *
 */

#ifndef IFJ_EXPRESSION_CODEGEN_H
#define IFJ_EXPRESSION_CODEGEN_H

#include <stdlib.h>
#include "dynamic_buffer.h"
#include "dll_instruction_list.h"

tDynamicBuffer *label_name_gen(char* name);
void save_create_tf(tDynamicBuffer *instruction);
void insert_instruction(DLList *instruction_list, tDynamicBuffer *instruction);
void def_tmp_get_type(tDynamicBuffer *instruction);

void gen_muls(DLList *instruction_list, tDynamicBuffer *instruction);
void gen_divs(DLList *instruction_list, tDynamicBuffer *instruction);
void gen_adds(DLList *instruction_list, tDynamicBuffer *instruction);
void gen_subs(DLList *instruction_list, tDynamicBuffer *instruction);
void gen_concat(DLList *instruction_list, tDynamicBuffer *instruction);
void gen_lts_gts(DLList *instruction_list, tDynamicBuffer *instruction, char *cmp_instruction);
void gen_ltes(DLList *instruction_list, tDynamicBuffer *instruction);
void gen_gtes(DLList *instruction_list, tDynamicBuffer *instruction);
void gen_eqs(DLList *instruction_list, tDynamicBuffer *instruction);
void gen_neqs(DLList *instruction_list, tDynamicBuffer *instruction);

#endif //IFJ_EXPRESSION_CODEGEN_H
