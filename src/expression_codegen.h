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

/**
 * @brief Function for generating string with unique id
 *
 * @param name String to which the id should be added
 *
 * @return Dynamic buffer containing unique string as data
 * */
tDynamicBuffer *label_name_gen(char* name);

/**
 * @brief Function for generating IFJcode22 that checks whether variable has been initialized on runtime
 *
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 * @param var_id Dynamic buffer containing variable identificator as data
 *
 * */
void var_init_check(DLList *instruction_list, tDynamicBuffer *var_id);

/**
 * @brief Function for generating IFJcode22 that pushes TF to stack of LF
 *
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 * @param var_id Dynamic buffer containing variable identificator as data
 *
 * */
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
