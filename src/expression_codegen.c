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

void var_init_check(DLList *instruction_list, tDynamicBuffer *var_id){
    tDynamicBuffer *instruction = dynamicBuffer_INIT();
    tDynamicBuffer *var_init_true = label_name_gen("var_init_true");

    dynamicBuffer_ADD_STRING(instruction, "TYPE GF@expr_var_type LF@");
    dynamicBuffer_ADD_STRING(instruction, var_id->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFNEQ ");
    dynamicBuffer_ADD_STRING(instruction, var_init_true->data);
    dynamicBuffer_ADD_STRING(instruction, " GF@expr_var_type string@\n");

    dynamicBuffer_ADD_STRING(instruction, "EXIT int@5\n");

    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, var_init_true->data);

    insert_instruction(instruction_list, instruction);
    dynamicBufferFREE(var_init_true);
}

void save_create_tf(tDynamicBuffer *instruction){
    dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME\n");
}

void insert_instruction(DLList *instruction_list, tDynamicBuffer *instruction){
    if(!strcmp(instruction_list->called_from->key,"$$main")){
        DLL_InsertAfter_main(instruction_list, instruction);
        if (instruction_list->active == instruction_list->main_body){
            DLL_Next(instruction_list);
        }
        DLL_Next_main(instruction_list);
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

void gen_divs(DLList *instruction_list, tDynamicBuffer *instruction){
    save_create_tf(instruction);
    def_tmp_get_type(instruction);

    tDynamicBuffer *div_op_1_float = label_name_gen("div_op_1_float");
    tDynamicBuffer *div_op_1_int2float = label_name_gen("div_op_1_int2float");
    tDynamicBuffer *div_op_1_null2float = label_name_gen("div_op_1_null2float");
    tDynamicBuffer *div_op_2_float = label_name_gen("div_op_2_float");
    tDynamicBuffer *div_op_2_int2float = label_name_gen("div_op_2_int2float");
    tDynamicBuffer *div_op_2_null2float = label_name_gen("div_op_2_null2float");

    // IF OPERAND 1 IS FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@float\n");

    // IF OPERAND 1 IS INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@int\n");

    // IF OPERAND 1 IS NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_null2float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@nil\n");

    // IF OPERAND 1 IS [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // OPERAND 1 INT TO FLOAT
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_1 TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // OPERAND 1 NULL TO FLOAT
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_null2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // OPERAND 1 WAS CONVERTED TO FLOAT (or was float already)
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // IF OPERAND 2 IS FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@float\n");

    // IF OPERAND 2 IS INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@int\n");

    // IF OPERAND 2 IS NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_null2float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@nil\n");

    // IF OPERAND 2 IS [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // OPERAND 2 INT TO FLOAT
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_2 TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // OPERAND 2 NULL TO FLOAT
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_null2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // OPERAND 2 WAS CONVERTED TO FLOAT (or was float already)
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");

    dynamicBuffer_ADD_STRING(instruction, "DIVS");

    dynamicBufferFREE(div_op_1_float);
    dynamicBufferFREE(div_op_1_int2float);
    dynamicBufferFREE(div_op_1_null2float);
    dynamicBufferFREE(div_op_2_float);
    dynamicBufferFREE(div_op_2_int2float);
    dynamicBufferFREE(div_op_2_null2float);

    insert_instruction(instruction_list, instruction);
}

void gen_adds(DLList *instruction_list, tDynamicBuffer *instruction){
    save_create_tf(instruction);
    def_tmp_get_type(instruction);

    tDynamicBuffer *add_operand_1_float = label_name_gen("add_operand_1_float");
    tDynamicBuffer *add_operand_1_int = label_name_gen("add_operand_1_int");
    tDynamicBuffer *add_operand_1_null = label_name_gen("add_operand_1_null");
    tDynamicBuffer *add_operand_1_float_operand_2_null = label_name_gen("add_operand_1_float_operand_2_null");
    tDynamicBuffer *add_operand_1_int_operand_2_null = label_name_gen("add_operand_1_int_operand_2_null");
    tDynamicBuffer *add_operand_1_null_operand_2_null = label_name_gen("add_operand_1_null_operand_2_null");
    tDynamicBuffer *add_operand_2_int_operand_1_null = label_name_gen("add_operand_2_int_operand_1_null");
    tDynamicBuffer *add_operand_2_float_operand_1_null = label_name_gen("add_operand_2_float_operand_1_null");
    tDynamicBuffer *add_operand_2_int2float = label_name_gen("add_operand_2_int2float");
    tDynamicBuffer *add_operand_1_int2float = label_name_gen("add_operand_1_int2float");
    tDynamicBuffer *add_calc = label_name_gen("add_calc");

    // IF OPERAND 1 IS FLOAT JUMP TO OPERAND_1_float
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_1_TYPE\n");

    // IF OPERAND 1 IS INT JUMP TO OPERAND_1_int
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_1_TYPE\n");

    // IF OPERAND 1 IS NULL JUMP TO OPERAND_1_null
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_1_TYPE\n");

    // OPERAND 1 IS INVALID
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // add_operand_1_float
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // FLOAT + FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // FLOAT + INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // FLOAT + NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // FLOAT + [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // add_operand_1_int
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // INT + FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // INT + INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // INT + NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // INT + [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // add_operand_1_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // NULL + FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_2_float_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // NULL + INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_2_int_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // NULL + NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_null_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // NULL + [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // add_operand_1_int2float
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_1 TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // add_operand_2_int2float
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_2 TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // add_operand_1_int_operand_2_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // add_operand_1_float_operand_2_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // add_operand_2_int_operand_1_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_2_int_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // add_operand_2_float_operand_1_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_2_float_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // add_operand_1_null_operand_2_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_operand_1_null_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");


    // add_calc
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");

    dynamicBuffer_ADD_STRING(instruction, "ADDS");

    dynamicBufferFREE(add_operand_1_float);
    dynamicBufferFREE(add_operand_1_int);
    dynamicBufferFREE(add_operand_1_null);
    dynamicBufferFREE(add_operand_1_int2float);
    dynamicBufferFREE(add_operand_1_float_operand_2_null);
    dynamicBufferFREE(add_operand_1_int_operand_2_null);
    dynamicBufferFREE(add_operand_2_int2float);
    dynamicBufferFREE(add_operand_1_null_operand_2_null);
    dynamicBufferFREE(add_operand_2_int_operand_1_null);
    dynamicBufferFREE(add_operand_2_float_operand_1_null);
    dynamicBufferFREE(add_calc);

    insert_instruction(instruction_list, instruction);
}

void gen_subs(DLList *instruction_list, tDynamicBuffer *instruction){
    save_create_tf(instruction);
    def_tmp_get_type(instruction);

    tDynamicBuffer *sub_operand_1_float = label_name_gen("sub_operand_1_float");
    tDynamicBuffer *sub_operand_1_int = label_name_gen("sub_operand_1_int");
    tDynamicBuffer *sub_operand_1_null = label_name_gen("sub_operand_1_null");
    tDynamicBuffer *sub_operand_1_float_operand_2_null = label_name_gen("sub_operand_1_float_operand_2_null");
    tDynamicBuffer *sub_operand_1_int_operand_2_null = label_name_gen("sub_operand_1_int_operand_2_null");
    tDynamicBuffer *sub_operand_1_null_operand_2_null = label_name_gen("sub_operand_1_null_operand_2_null");
    tDynamicBuffer *sub_operand_2_int_operand_1_null = label_name_gen("sub_operand_2_int_operand_1_null");
    tDynamicBuffer *sub_operand_2_float_operand_1_null = label_name_gen("sub_operand_2_float_operand_1_null");
    tDynamicBuffer *sub_operand_2_int2float = label_name_gen("sub_operand_2_int2float");
    tDynamicBuffer *sub_operand_1_int2float = label_name_gen("sub_operand_1_int2float");
    tDynamicBuffer *sub_calc = label_name_gen("sub_calc");

    // IF OPERAND 1 IS FLOAT JUMP TO OPERAND_1_float
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_1_TYPE\n");

    // IF OPERAND 1 IS INT JUMP TO OPERAND_1_int
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_1_TYPE\n");

    // IF OPERAND 1 IS NULL JUMP TO OPERAND_1_null
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_1_TYPE\n");

    // sub_operand_1_float
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // FLOAT + FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // FLOAT + INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // FLOAT + NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // FLOAT + [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // sub_operand_1_int
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // INT + FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // INT + INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // INT + NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // INT + [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // sub_operand_1_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // NULL + FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_2_float_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // NULL + INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_2_int_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // NULL + NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_null_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // NULL + [STRING, BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // sub_operand_1_int2float
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_1 TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // sub_operand_2_int2float
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_2 TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // sub_operand_1_int_operand_2_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // sub_operand_1_float_operand_2_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // sub_operand_2_int_operand_1_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_2_int_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // sub_operand_2_float_operand_1_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_2_float_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // sub_operand_1_null_operand_2_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_operand_1_null_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");


    // sub_calc
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");

    dynamicBuffer_ADD_STRING(instruction, "SUBS");

    dynamicBufferFREE(sub_operand_1_float);
    dynamicBufferFREE(sub_operand_1_int);
    dynamicBufferFREE(sub_operand_1_null);
    dynamicBufferFREE(sub_operand_1_int2float);
    dynamicBufferFREE(sub_operand_1_float_operand_2_null);
    dynamicBufferFREE(sub_operand_1_int_operand_2_null);
    dynamicBufferFREE(sub_operand_2_int2float);
    dynamicBufferFREE(sub_operand_1_null_operand_2_null);
    dynamicBufferFREE(sub_operand_2_int_operand_1_null);
    dynamicBufferFREE(sub_operand_2_float_operand_1_null);
    dynamicBufferFREE(sub_calc);

    insert_instruction(instruction_list, instruction);
}

void gen_concat(DLList *instruction_list, tDynamicBuffer *instruction){
    save_create_tf(instruction);
    def_tmp_get_type(instruction);

    tDynamicBuffer *concat_operand_1_str = label_name_gen("concat_operand_1_str");
    tDynamicBuffer *concat_operand_1_null = label_name_gen("concat_operand_1_null");
    tDynamicBuffer *concat_operand_1_null2str = label_name_gen("concat_operand_1_null2str");
    tDynamicBuffer *concat_operand_2_null2str = label_name_gen("concat_operand_2_null2str");
    tDynamicBuffer *concat_operands_null2str = label_name_gen("concat_operands_null2str");
    tDynamicBuffer *concat_calc = label_name_gen("concat_calc");

    // IF OPERAND 1 IS STRING JUMP TO concat_operand_1_str
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_str->data);
    dynamicBuffer_ADD_STRING(instruction, " string@string TF@$TMP_1_TYPE\n");

    // IF OPERAND 1 IS NULL JUMP TO concat_operand_1_null
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_1_TYPE\n");

    // concat_operand_1_str
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // STRING . STRING
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@string TF@$TMP_2_TYPE\n");

    // STRING . NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_2_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // STRING . [INT, FLOAT. BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // concat_operand_1_null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // NULL . STRING
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, " string@string TF@$TMP_2_TYPE\n");

    // NULL . NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_operands_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // STRING . [INT, FLOAT. BOOL]
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // concat_operand_2_null2str
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_2_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // concat_operand_1_null2str
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // concat_operands_null2str
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operands_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // concat_calc
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "CONCAT TF@$TMP_1 TF@$TMP_1 TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME");

    dynamicBufferFREE(concat_calc);
    dynamicBufferFREE(concat_operand_2_null2str);
    dynamicBufferFREE(concat_operand_1_null2str);
    dynamicBufferFREE(concat_operands_null2str);
    dynamicBufferFREE(concat_operand_1_str);
    dynamicBufferFREE(concat_operand_1_null);

    insert_instruction(instruction_list, instruction);
}

void gen_lts_gts(DLList *instruction_list, tDynamicBuffer *instruction, char *cmp_instruction){
    save_create_tf(instruction);
    def_tmp_get_type(instruction);

    tDynamicBuffer *lt_gt_calc = label_name_gen("lt_gt_calc");
    tDynamicBuffer *lt_gt_op_error = label_name_gen("lt_gt_op_error");
    tDynamicBuffer *lt_gt_op_1_null = label_name_gen("lt_gt_op_1_null");
    tDynamicBuffer *lt_gt_op_2_null = label_name_gen("lt_gt_op_2_null");
    tDynamicBuffer *lt_gt_op_1_null2str = label_name_gen("lt_gt_op_1_null2str");
    tDynamicBuffer *lt_gt_op_1_null2bool = label_name_gen("lt_gt_op_1_null2bool");
    tDynamicBuffer *lt_gt_op_2_2bool = label_name_gen("lt_gt_op_2_2bool");
    tDynamicBuffer *lt_gt_op_2_null2str = label_name_gen("lt_gt_op_2_null2str");
    tDynamicBuffer *lt_gt_op_2_null2bool = label_name_gen("lt_gt_op_2_null2bool");
    tDynamicBuffer *lt_gt_op_1_2bool = label_name_gen("lt_gt_op_1_2bool");
    tDynamicBuffer *lt_gt_op_1_int2float = label_name_gen("lt_gt_op_1_int2float");
    tDynamicBuffer *lt_gt_op_1_null_op_2_int2bool = label_name_gen("lt_gt_op_1_null_op_2_int2bool");
    tDynamicBuffer *lt_gt_op_2_set_false_calc = label_name_gen("lt_gt_op_2_set_false_calc");
    tDynamicBuffer *lt_gt_op_2_set_true_calc = label_name_gen("lt_gt_op_2_set_true_calc");
    tDynamicBuffer *lt_gt_op_1_null_op_2_float2bool = label_name_gen("lt_gt_op_1_null_op_2_float2bool");
    tDynamicBuffer *lt_gt_op_1_set_false_calc = label_name_gen("lt_gt_op_1_set_false_calc");
    tDynamicBuffer *lt_gt_op_1_set_true_calc = label_name_gen("lt_gt_op_1_set_true_calc");
    tDynamicBuffer *lt_gt_op_2_null_op_1_int2bool = label_name_gen("lt_gt_op_2_null_op_1_int2bool");
    tDynamicBuffer *lt_gt_op_2_null_op_1_float2bool = label_name_gen("lt_gt_op_2_null_op_1_float2bool");

    // CONDITIONS FOLLOWED BY FALSE BRANCHES

    // if(OPERAND_1_TYPE == null)
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@nil\n");

    // if(OPERAND_2_TYPE == null)
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@nil\n");

    // if(OPERAND_1_TYPE == bool)
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_2bool->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@bool\n");

    // if(OPERAND_2_TYPE == bool)
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_2bool->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@bool\n");

    // if(OPERAND_1_TYPE == OPERAND_2_TYPE)
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE TF@$TMP_2_TYPE\n");

    // if(OPERAND_1_TYPE == string)
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_error->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@string\n");

    // if(OPERAND_2_TYPE == string)
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_error->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@string\n");

    // if(OPERAND_1_TYPE == int)
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@int\n");

    // OPERAND 2 INT TO FLOAT
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_2 TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // TRUE BRANCHES OF CONDITIONS ABOVE

    // OPERAND_1_TYPE == null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@string\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // OPERAND_2_TYPE == null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@string\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_1_null2str
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_1_null2bool
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 bool@false\n");

    // lt_gt_op_2_2bool
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_set_false_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@nil\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null_op_2_int2bool->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@int\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null_op_2_float2bool->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@float\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@bool\n");

    // OPERAND 2 IS INVALID
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_error->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_1_null_op_2_int2bool
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null_op_2_int2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_set_false_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2 int@0\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_set_true_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_1_null_op_2_float2bool
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_null_op_2_float2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_set_false_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2 float@0x0p+0\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_set_true_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_2_set_false_calc
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_set_false_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 bool@false\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_2_set_true_calc
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_set_true_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 bool@true\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_2_null2str
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_2_null2bool
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 bool@false\n");

    // lt_gt_op_1_2bool
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_set_false_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@null\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null_op_1_int2bool->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@int\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null_op_1_float2bool->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@float\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@bool\n");

    // OPERAND 1 IS INVALID
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_error->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_2_null_op_1_int2bool
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null_op_1_int2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_set_false_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1 int@0\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_set_true_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_2_null_op_1_float2bool
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_2_null_op_1_float2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_set_false_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1 float@0x0p+0\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_set_true_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_1_set_false_calc
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_set_false_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 bool@false\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_1_set_true_calc
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_set_true_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 bool@true\n");

    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // OPERAND 1 INT TO FLOAT
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_1 TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // lt_gt_op_error
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_op_error->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // lt_gt_calc
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, lt_gt_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_2\n");

    dynamicBuffer_ADD_STRING(instruction, cmp_instruction);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "POPFRAME");

    dynamicBufferFREE(lt_gt_calc);
    dynamicBufferFREE(lt_gt_op_error);
    dynamicBufferFREE(lt_gt_op_1_null);
    dynamicBufferFREE(lt_gt_op_2_null);
    dynamicBufferFREE(lt_gt_op_1_null2str);
    dynamicBufferFREE(lt_gt_op_1_null2bool);
    dynamicBufferFREE(lt_gt_op_2_2bool);
    dynamicBufferFREE(lt_gt_op_2_null2str);
    dynamicBufferFREE(lt_gt_op_2_null2bool);
    dynamicBufferFREE(lt_gt_op_1_2bool);
    dynamicBufferFREE(lt_gt_op_1_int2float);
    dynamicBufferFREE(lt_gt_op_1_null_op_2_int2bool);
    dynamicBufferFREE(lt_gt_op_2_set_false_calc);
    dynamicBufferFREE(lt_gt_op_2_set_true_calc);
    dynamicBufferFREE(lt_gt_op_1_null_op_2_float2bool);
    dynamicBufferFREE(lt_gt_op_1_set_false_calc);
    dynamicBufferFREE(lt_gt_op_1_set_true_calc);
    dynamicBufferFREE(lt_gt_op_2_null_op_1_int2bool);
    dynamicBufferFREE(lt_gt_op_2_null_op_1_float2bool);

    insert_instruction(instruction_list, instruction);
}

void gen_ltes(DLList *instruction_list, tDynamicBuffer *instruction){
    gen_lts_gts(instruction_list, instruction, "GTS");

    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction, "NOTS");
    insert_instruction(instruction_list, instruction);
}

void gen_gtes(DLList *instruction_list, tDynamicBuffer *instruction){
    gen_lts_gts(instruction_list, instruction, "LTS");

    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction, "NOTS");
    insert_instruction(instruction_list, instruction);
}

void gen_eqs(DLList *instruction_list, tDynamicBuffer *instruction){
    save_create_tf(instruction);
    def_tmp_get_type(instruction);

    tDynamicBuffer *eq_neq_type = label_name_gen("eq_neq_type");
    tDynamicBuffer *eq_end = label_name_gen("eq_end");

    // IF NOT SAME TYPE RETURN FALSE
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFNEQ ");
    dynamicBuffer_ADD_STRING(instruction, eq_neq_type->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE TF@$TMP_2_TYPE\n");

    // calc_eq
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");

    dynamicBuffer_ADD_STRING(instruction, "EQS\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, eq_end->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // eq_neq_type
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, eq_neq_type->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "PUSHS bool@false\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, eq_end->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // eq_end
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, eq_end->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "POPFRAME");

    dynamicBufferFREE(eq_neq_type);
    dynamicBufferFREE(eq_end);

    insert_instruction(instruction_list, instruction);
}

void gen_neqs(DLList *instruction_list, tDynamicBuffer *instruction){
    gen_eqs(instruction_list, instruction);

    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction, "NOTS");
    insert_instruction(instruction_list, instruction);
}
