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
    }
    dynamicBufferFREE(instruction);
}

void def_tmp_get_type(tDynamicBuffer *instruction){
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@$TMP_1_TYPE\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@$TMP_2_TYPE\n");

    dynamicBuffer_ADD_STRING(instruction, "POPS TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "POPS TF@$TMP_1\n");

    dynamicBuffer_ADD_STRING(instruction, "TYPE TF@$TMP_1_TYPE TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "TYPE TF@$TMP_2_TYPE TF@$TMP_2\n");
}

void gen_muls(DLList *instruction_list, tDynamicBuffer *instruction){
    save_create_tf(instruction);
    def_tmp_get_type(instruction);

    tDynamicBuffer *mul_operand_1_float = label_name_gen("mul_operand_1_float");
    tDynamicBuffer *mul_operand_1_int = label_name_gen("mul_operand_1_int");
    tDynamicBuffer *mul_operand_1_null = label_name_gen("mul_operand_1_null");
    tDynamicBuffer *mul_operand_1_float_operand_2_null = label_name_gen("mul_operand_1_float_operand_2_null");
    tDynamicBuffer *mul_operand_1_int_operand_2_null = label_name_gen("mul_operand_1_int_operand_2_null");
    tDynamicBuffer *mul_operand_2_int2float = label_name_gen("mul_operand_2_int2float");
    tDynamicBuffer *mul_operand_1_int2float = label_name_gen("mul_operand_1_int2float");
    tDynamicBuffer *mul_calc = label_name_gen("mul_calc");

    // IF OPERAND 1 IS FLOAT JUMP TO OPERAND_1_float
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_1_TYPE\n");

    // IF OPERAND 1 IS INT JUMP TO OPERAND_1_int
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_1_TYPE\n");

    // IF OPERAND 1 IS NULL JUMP TO OPERAND_1_null
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_1_TYPE\n");

    // OPERAND 1 IS OF ILLEGAL TYPE
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // mul_operand_1_float
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // FLOAT * FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // FLOAT * INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // FLOAT * NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // FLOAT * [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // mul_operand_1_int
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // INT * FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // INT * INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // INT * NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // INT * [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // mul_operand_1_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // NULL * FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // NULL * INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // NULL * NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // NULL * [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // mul_operand_1_int2float
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_1 TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // mul_operand_2_int2float
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_2 TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // mul_operand_1_int_operand_2_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // mul_operand_1_float_operand_2_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");


    // mul_calc
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");

    dynamicBuffer_ADD_STRING(instruction, "MULS");

    dynamicBufferFREE(mul_operand_1_float);
    dynamicBufferFREE(mul_operand_1_int);
    dynamicBufferFREE(mul_operand_1_null);
    dynamicBufferFREE(mul_operand_1_int2float);
    dynamicBufferFREE(mul_operand_1_float_operand_2_null);
    dynamicBufferFREE(mul_operand_1_int_operand_2_null);
    dynamicBufferFREE(mul_operand_2_int2float);
    dynamicBufferFREE(mul_calc);

    insert_instruction(instruction_list, instruction);
}


