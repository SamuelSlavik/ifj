/**
 * @file expression_codegen.c
 * @author Adam Pekný (xpekny00)
 * @brief
 * @version 0.1
 * @date 2022-11-18
 *
 */

#include "expression_codegen.h"

tDynamicBuffer *label_name_gen(char* name){
    static long int id;

    long long int digit_count = 0;
    long int tmp_id = id;

    while (tmp_id != 0){
        tmp_id /= 10;
        digit_count++;
    }

    char *idstr = calloc(sizeof(char), digit_count + 2);
    sprintf(idstr,"%ld",id);

    tDynamicBuffer *buffer = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(buffer,name);
    dynamicBuffer_ADD_STRING(buffer,idstr);
    free(idstr);
    id += 1;
    return buffer;
}

void save_create_tf(tDynamicBuffer *instruction){
    dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME\n");
}

void insert_instruction(DLList *instruction_list, tDynamicBuffer *instruction){
    if(!strcmp(instruction_list->called_from->key,"$$main")){
        DLL_InsertAfter_main(instruction_list, instruction);
        DLL_Next_main(instruction_list);
        if (instruction_list->active == instruction_list->main_body){
            DLL_Next(instruction_list);
        }
    } else {
        DLL_InsertAfter(instruction_list, instruction);
        DLL_Next(instruction_list);
        if (instruction_list->active == instruction_list->main_body){
            DLL_Next_main(instruction_list);
        }
    }
    dynamicBufferFREE(instruction);
}

void def_tmp_get_type(tDynamicBuffer *instruction){
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@$TMP_1_TYPE\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@$TMP_2_TYPE\n");

    dynamicBuffer_ADD_STRING(instruction, "POPS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "POPS TF@$TMP_2\n");

    dynamicBuffer_ADD_STRING(instruction, "TYPE TF@$TMP_1_TYPE TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "TYPE TF@$TMP_2_TYPE TF@$TMP_2\n");
}
