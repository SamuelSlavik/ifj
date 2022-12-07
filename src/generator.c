/**
 * @file generator.c
 * @brief Implementation of built in functions and working with ifjcode22
 * @author Adam Pekný (xpekny00), Samuel Slávik (xslavi37), Jakub Kontrík (xkontr00)
 */

#include <stdlib.h>
#include "dll_instruction_list.h"
#include "dynamic_buffer.h"
#include "parser.h"
#include "generator.h"

tDynamicBuffer *label_name_gen(char* name){
    static long int id;
    long int tmp_id = id;
    // Get number of digits of id
    long long int digit_count = 1;

    while (tmp_id != 0){
        tmp_id /= 10;
        digit_count++;
    }
    // Allocate space for id as string
    char *idstr = calloc(sizeof(char), digit_count + 1);
    if (idstr == NULL){
        exit(99);
    }
    sprintf(idstr,"%ld",id);

    // Save desired name with id into buffer and increment id
    tDynamicBuffer *buffer = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(buffer,name);
    dynamicBuffer_ADD_STRING(buffer,idstr);
    free(idstr);
    id += 1;
    return buffer;
}

void var_init_check(DLList *instruction_list, tDynamicBuffer *var_id){
    tDynamicBuffer *instruction = dynamicBuffer_INIT();

    // Generate label for conditional jumps
    tDynamicBuffer *var_init_true = label_name_gen("var_init_true");

    // Get type of desired variable
    dynamicBuffer_ADD_STRING(instruction, "TYPE GF@expr_var_type LF@");
    dynamicBuffer_ADD_STRING(instruction, var_id->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // If type of variable is empty string it is uninitialized, exit with semantic error 5
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFNEQ ");
    dynamicBuffer_ADD_STRING(instruction, var_init_true->data);
    dynamicBuffer_ADD_STRING(instruction, " GF@expr_var_type string@\n");

    dynamicBuffer_ADD_STRING(instruction, "EXIT int@5\n");

    // Previous jump goes here if variable was initialized
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, var_init_true->data);

    insert_instruction(instruction_list, instruction);
    dynamicBufferFREE(var_init_true);
}

void generate_reads(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionReads\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@value\n");
    dynamicBuffer_ADD_STRING(instruction,"READ TF@value string\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@value\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(instruction);
}

void generate_readi(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionReadi\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@value\n");
    dynamicBuffer_ADD_STRING(instruction,"READ TF@value int\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@value\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(instruction);
}

void generate_readf(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionReadf\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@value\n");
    dynamicBuffer_ADD_STRING(instruction,"READ TF@value float\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@value\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(instruction);
}

void generate_write(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    // generate unique labels
    tDynamicBuffer *loopStart = label_name_gen("loopStart");
    tDynamicBuffer *loopWrite = label_name_gen("loopWrite");
    tDynamicBuffer *loopEnd = label_name_gen("loopEnd");

    dynamicBuffer_ADD_STRING(instruction,"LABEL functionWrite\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@term\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@numberOfArguments\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@numberOfArguments\n");
    // Loop start
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction, loopStart->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ "); // Loop condition
    dynamicBuffer_ADD_STRING(instruction, loopEnd->data);
    dynamicBuffer_ADD_STRING(instruction," TF@numberOfArguments int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@term\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction,loopWrite->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    // Loop content
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction, loopWrite->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"WRITE TF@term\n");
    dynamicBuffer_ADD_STRING(instruction,"SUB TF@numberOfArguments TF@numberOfArguments int@1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction, loopStart->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    // Loop end
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,loopEnd->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(loopStart);
    dynamicBufferFREE(loopEnd);
    dynamicBufferFREE(loopWrite);
    dynamicBufferFREE(instruction);
}

void generate_floatval(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    // generate unique labels
    tDynamicBuffer *argumentIsNil = label_name_gen("argumentIsNil");
    tDynamicBuffer *ending = label_name_gen("ending");
    tDynamicBuffer *isUnsupported = label_name_gen("isUnsupported");

    dynamicBuffer_ADD_STRING(instruction,"LABEL functionFloatVal\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argumentType\n");
    // Checking type or argument
    dynamicBuffer_ADD_STRING(instruction,"TYPE TF@argumentType TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ "); 
    dynamicBuffer_ADD_STRING(instruction, argumentIsNil->data);
    dynamicBuffer_ADD_STRING(instruction," string@nil TF@argumentType\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ "); 
    dynamicBuffer_ADD_STRING(instruction, ending->data);
    dynamicBuffer_ADD_STRING(instruction," string@float TF@argumentType\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ "); 
    dynamicBuffer_ADD_STRING(instruction, isUnsupported->data);
    dynamicBuffer_ADD_STRING(instruction," string@string TF@argumentType\n");
    // is int
    dynamicBuffer_ADD_STRING(instruction,"INT2FLOAT TF@argument1 TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction, ending->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    // Is null
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, argumentIsNil->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS float@0x0.0p+0\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    // Is string
    dynamicBuffer_ADD_STRING(instruction, isUnsupported->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction,"EXIT int@4\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction, ending->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(argumentIsNil);
    dynamicBufferFREE(isUnsupported);
    dynamicBufferFREE(ending);
    dynamicBufferFREE(instruction);
}

void generate_intval(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    // generate unique labels
    tDynamicBuffer *argumentIsNil = label_name_gen("argumentIsNil");
    tDynamicBuffer *ending = label_name_gen("ending");
    tDynamicBuffer *isUnsupported = label_name_gen("isUnsupported");
    
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionIntVal\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argumentType\n");
    // Checking type of argument1
    dynamicBuffer_ADD_STRING(instruction,"TYPE TF@argumentType TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, argumentIsNil->data);
    dynamicBuffer_ADD_STRING(instruction," string@nil TF@argumentType\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,ending->data);
    dynamicBuffer_ADD_STRING(instruction," string@int TF@argumentType\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,isUnsupported->data);
    dynamicBuffer_ADD_STRING(instruction," string@string TF@argumentType\n");
    // is float
    dynamicBuffer_ADD_STRING(instruction,"FLOAT2INT TF@argument1 TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction,ending->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    // is null
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,argumentIsNil->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN\n");
    // is string
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,isUnsupported->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"EXIT int@4\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,ending->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(argumentIsNil);
    dynamicBufferFREE(isUnsupported);
    dynamicBufferFREE(ending);
    dynamicBufferFREE(instruction);
}

void generate_strval(tDynamicBuffer *instruction, DLList *instruction_list){
    // generate unique labels
    tDynamicBuffer *isNil = label_name_gen("isNil");
    tDynamicBuffer *isString = label_name_gen("isString");
    tDynamicBuffer *ending = label_name_gen("ending");
    tDynamicBuffer *isUnsupported = label_name_gen("isUnsupported");
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionStrVal\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argumentType\n");
    dynamicBuffer_ADD_STRING(instruction,"TYPE TF@argumentType TF@argument1\n");
    // Check the type of argument1
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,isString->data);
    dynamicBuffer_ADD_STRING(instruction," string@string TF@argumentType\n");    
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,isNil->data);
    dynamicBuffer_ADD_STRING(instruction," string@nill TF@argumentType\n");    
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,isUnsupported->data);
    dynamicBuffer_ADD_STRING(instruction," string@int TF@argumentType\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,isUnsupported->data);
    dynamicBuffer_ADD_STRING(instruction," string@float TF@argumentType\n");    
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,isUnsupported->data);
    dynamicBuffer_ADD_STRING(instruction," string@bool TF@argumentType\n");
    // is null
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction, isNil->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS string@\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction,ending->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    // is int, float or bool
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction, isUnsupported->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"EXIT int@4\n");
    // is string
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,isString->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,ending->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(isNil);
    dynamicBufferFREE(isString);
    dynamicBufferFREE(isUnsupported);
    dynamicBufferFREE(ending);
    dynamicBufferFREE(instruction);
}

void generate_strlen(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionStrlen\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"STRLEN TF@argument1 TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(instruction);
}

void generate_substr(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    // generate unique labels
    tDynamicBuffer *error = label_name_gen("error");
    tDynamicBuffer *loopStart = label_name_gen("loopStart");
    tDynamicBuffer *loopEnd = label_name_gen("loopEnd");

    dynamicBuffer_ADD_STRING(instruction,"LABEL functionSubStr\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@string\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@string\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@i\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@i\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@j\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@j\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@tmp\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@length\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@subString\n");
    dynamicBuffer_ADD_STRING(instruction,"MOVE TF@subString string@\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@tmp2\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@index\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@character\n");
    // error states
    // i > j
    dynamicBuffer_ADD_STRING(instruction,"GT TF@tmp TF@i TF@j\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");
    // i < 0
    dynamicBuffer_ADD_STRING(instruction,"LT TF@tmp TF@i int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");
    // j < 0
    dynamicBuffer_ADD_STRING(instruction,"LT TF@tmp TF@j int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");
    // j or i > strlen
    dynamicBuffer_ADD_STRING(instruction,"STRLEN TF@length TF@string\n");
    dynamicBuffer_ADD_STRING(instruction,"GT TF@tmp TF@i TF@length\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");
    dynamicBuffer_ADD_STRING(instruction,"GT TF@tmp TF@j TF@length\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");
    // get length of substring and set starting index
    dynamicBuffer_ADD_STRING(instruction,"SUB TF@tmp2 TF@j TF@i\n");
    dynamicBuffer_ADD_STRING(instruction,"MOVE TF@index TF@i\n");
    // LOOOP START
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,loopStart->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,loopEnd->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp2 int@0\n");
    // LOOP write
    dynamicBuffer_ADD_STRING(instruction,"GETCHAR TF@character TF@string TF@index\n");
    dynamicBuffer_ADD_STRING(instruction,"CONCAT TF@subString TF@subString TF@character\n");
    dynamicBuffer_ADD_STRING(instruction,"ADD TF@index TF@index int@1\n");
    dynamicBuffer_ADD_STRING(instruction,"SUB TF@tmp2 TF@tmp2 int@1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction,loopStart->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    // error
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS nil@nil\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN\n");
    // LOOP end
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,loopEnd->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@subString\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(loopStart);
    dynamicBufferFREE(loopEnd);
    dynamicBufferFREE(error);
    dynamicBufferFREE(instruction);
}

void generate_ord(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionOrd\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@argument1\n");
    // Add char with ascii value 0 at end of argument1, if argument1 is empty string, it returns 0 in the next line
    dynamicBuffer_ADD_STRING(instruction,"CONCAT TF@argument1 TF@argument1 string@\\000\n");
    dynamicBuffer_ADD_STRING(instruction,"STRI2INT TF@argument1 TF@argument1 int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(instruction);
}

void generate_chr(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionChr\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@returnedChar\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"INT2CHAR TF@returnedChar TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS TF@returnedChar\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(instruction);
}

void check_fn_arguments(tStack *defined_params, tStack *called_params, tDynamicBuffer *instruction, DLList *instruction_list){
    // PRINT STACK
    tStack print_stack;
    StackInit(&print_stack);
    // looping through whole stack of arguments
    while (!StackIsEmpty(defined_params)){
        tVar_TaV *stack_top_itm = ((tVar_TaV*) StackTop(defined_params));
        tDynamicBuffer *stack_top_itm_called = ((tDynamicBuffer *) StackTop(called_params));
        tDynamicBuffer *return_type_false = label_name_gen("return_type_false");
        tDynamicBuffer *TF_res = label_name_gen("TF@res_");
        tDynamicBuffer *TF_res_type = label_name_gen("TF@res_type_");
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "DEFVAR ");
        dynamicBuffer_ADD_STRING(instruction, TF_res->data);
        dynamicBuffer_ADD_STRING(instruction,"\n");
        dynamicBuffer_ADD_STRING(instruction, "DEFVAR ");
        dynamicBuffer_ADD_STRING(instruction, TF_res_type->data);
        dynamicBuffer_ADD_STRING(instruction,"\n");
        dynamicBuffer_ADD_STRING(instruction, stack_top_itm_called->data);
        dynamicBuffer_ADD_STRING(instruction, "\n");
        dynamicBuffer_ADD_STRING(instruction, "POPS ");
        dynamicBuffer_ADD_STRING(instruction, TF_res->data);
        dynamicBuffer_ADD_STRING(instruction,"\n");
        dynamicBuffer_ADD_STRING(instruction, "TYPE ");
        dynamicBuffer_ADD_STRING(instruction,TF_res_type->data);
        dynamicBuffer_ADD_STRING(instruction," ");
        dynamicBuffer_ADD_STRING(instruction,TF_res->data);
        dynamicBuffer_ADD_STRING(instruction,"\n");
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction,TF_res->data);
        dynamicBuffer_ADD_STRING(instruction,"\n");
        dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
        dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
        dynamicBuffer_ADD_STRING(instruction, " ");
        dynamicBuffer_ADD_STRING(instruction,TF_res_type->data);
        dynamicBuffer_ADD_STRING(instruction," string@");
        // checking parameter type
        switch(stack_top_itm->var_type){
            case T_STRING_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "string\n");
                break;
            case T_FLOAT_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "float\n");
                break;
            case T_INT_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "int\n");
                break;
            case T_STRING_N_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "string\n");
                dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
                dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
                dynamicBuffer_ADD_STRING(instruction, " ");
                dynamicBuffer_ADD_STRING(instruction,TF_res_type->data);
                dynamicBuffer_ADD_STRING(instruction," string@nil\n");
                break;
            case T_FLOAT_N_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "float\n");
                dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
                dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
                dynamicBuffer_ADD_STRING(instruction, " ");
                dynamicBuffer_ADD_STRING(instruction,TF_res_type->data);
                dynamicBuffer_ADD_STRING(instruction," string@nil\n");
                break;
            case T_INT_N_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "int\n");
                dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
                dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
                dynamicBuffer_ADD_STRING(instruction, " ");
                dynamicBuffer_ADD_STRING(instruction,TF_res_type->data);
                dynamicBuffer_ADD_STRING(instruction," string@nil\n");
                break;
            default:
                dynamicBuffer_ADD_STRING(instruction, "\n");
                dynamicBuffer_ADD_STRING(instruction, "JUMP ");
                dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
                dynamicBuffer_ADD_STRING(instruction, "\n");
                break;

        }
        dynamicBuffer_ADD_STRING(instruction, "EXIT int@4\n");
        dynamicBuffer_ADD_STRING(instruction, "LABEL ");
        dynamicBuffer_ADD_STRING(instruction, return_type_false->data);

        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        dynamicBufferFREE(return_type_false);
        dynamicBufferFREE(stack_top_itm_called);
        dynamicBufferFREE(TF_res);
        dynamicBufferFREE(TF_res_type);
        StackPush(&print_stack, stack_top_itm);
        StackPop(defined_params);
        StackPop(called_params);
    }
    while (!StackIsEmpty(&print_stack)){
        tVar_TaV *stack_top_itm = ((tVar_TaV*) StackTop(&print_stack));
        StackPush(defined_params, stack_top_itm);
        StackPop(&print_stack);
    }
}
void convert_into_bool(tDynamicBuffer *instruction, DLList *instruction_list,tDynamicBuffer *label_name){
    instruction = dynamicBuffer_INIT();

    tDynamicBuffer *expr_set_false = label_name_gen("expr_set_false");
    tDynamicBuffer *expr_set_true = label_name_gen("expr_set_true");
    tDynamicBuffer *expr_res_int2bool = label_name_gen("expr_res_int2bool");
    tDynamicBuffer *expr_res_float2bool = label_name_gen("expr_res_float2bool");
    tDynamicBuffer *expr_res_string2bool = label_name_gen("expr_res_string2bool");
    tDynamicBuffer *expr_res_eval = label_name_gen("expr_res_eval");
    
    dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res_type\n");
    dynamicBuffer_ADD_STRING(instruction, "POPS TF@res\n");
    dynamicBuffer_ADD_STRING(instruction, "TYPE TF@res_type TF@res\n");

    // IF RESULT IS NULL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@nil\n");

    // IF RESULT IS INT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_int2bool->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@int\n");

    // IF RESULT IS FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_float2bool->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@float\n");

    // IF RESULT IS STRING
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_string2bool->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@string\n");

    // IF RESULT IS BOOL
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@bool\n");

    // UNKNOWN TYPE
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");
    
    // INT TO BOOL
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_int2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res int@0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // FLOAT TO BOOL
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_float2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // STRING TO BOOL
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_string2bool->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res string@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res string@0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // SET RESULT AS FALSE
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@res bool@false\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    
    // SET RESULT AS TRUE
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@res bool@true\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    
    // EVALUATE RESULT
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS bool@true\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFNEQS false_");
    dynamicBuffer_ADD_STRING(instruction, label_name->data);
    // cond jump 

    DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
    dynamicBufferFREE(expr_set_false);
    dynamicBufferFREE(expr_set_true);
    dynamicBufferFREE(expr_res_int2bool);
    dynamicBufferFREE(expr_res_float2bool);
    dynamicBufferFREE(expr_res_string2bool);
    dynamicBufferFREE(expr_res_eval);
    dynamicBufferFREE(instruction);
}

void check_return_type(tDynamicBuffer *instruction, DLList *instruction_list){
    tDynamicBuffer *return_type_false = label_name_gen("return_type_false");
    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res\n");
    dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res_type\n");
    dynamicBuffer_ADD_STRING(instruction, "POPS TF@res\n");
    dynamicBuffer_ADD_STRING(instruction, "TYPE TF@res_type TF@res\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
    
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@");
    //deciding which return type has function
    switch(instruction_list->called_from->data.fun_data.return_type){
        case T_STRING_TYPE:
            dynamicBuffer_ADD_STRING(instruction, "string\n");
            break;
        case T_FLOAT_TYPE:
            dynamicBuffer_ADD_STRING(instruction, "float\n");
            break;
        case T_INT_TYPE:
            dynamicBuffer_ADD_STRING(instruction, "int\n");
            break;
        case T_STRING_N_TYPE:
            dynamicBuffer_ADD_STRING(instruction, "string\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@nil\n");
            break;
        case T_FLOAT_N_TYPE:
            dynamicBuffer_ADD_STRING(instruction, "float\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@nil\n");
            break;
        case T_INT_N_TYPE:
            dynamicBuffer_ADD_STRING(instruction, "int\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@nil\n");
            break;
        default:
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            break;

    }
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@4\n");
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "POPFRAME");
    DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
    dynamicBufferFREE(instruction);
    dynamicBufferFREE(return_type_false);
}

void print_stack(tStack *defined_param, tDynamicBuffer *instruction, DLList *instruction_list,char *code){
    // helping stack so we can keep popped parameters
    tStack print_stack;
    StackInit(&print_stack);
    while (!StackIsEmpty(defined_param)){
        tVar_TaV *stack_top_itm = ((tVar_TaV*) StackTop(defined_param));
        StackPush(&print_stack, stack_top_itm);
        StackPop(defined_param);
    }
    while (!StackIsEmpty(&print_stack)){
        tVar_TaV *stack_top_itm = ((tVar_TaV*) StackTop(&print_stack));
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, code);
        dynamicBuffer_ADD_STRING(instruction, stack_top_itm->var);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        StackPush(defined_param, stack_top_itm);
        StackPop(&print_stack);
    }
}

// EXPRESSION CODEGEN START
void save_create_tf(tDynamicBuffer *instruction){
    dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME\n");
}

void insert_instruction(DLList *instruction_list, tDynamicBuffer *instruction){
    if(!strcmp(instruction_list->called_from->key,"$$main")){ // Instruction should be inserted after main body instructions
        DLL_InsertAfter_main(instruction_list, instruction);
        if (instruction_list->active == instruction_list->main_body){ // Move both pointers if they are same
            DLL_Next(instruction_list);
        }
        DLL_Next_main(instruction_list);
    } else { // Instruction should be inserted after active element (currently generated function)
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

    // Generate labels for jumps
    tDynamicBuffer *mul_operand_1_float = label_name_gen("mul_operand_1_float");
    tDynamicBuffer *mul_operand_1_int = label_name_gen("mul_operand_1_int");
    tDynamicBuffer *mul_operand_1_null = label_name_gen("mul_operand_1_null");
    tDynamicBuffer *mul_operand_1_float_operand_2_null = label_name_gen("mul_operand_1_float_operand_2_null");
    tDynamicBuffer *mul_operand_1_int_operand_2_null = label_name_gen("mul_operand_1_int_operand_2_null");
    tDynamicBuffer *mul_operand_2_int2float = label_name_gen("mul_operand_2_int2float");
    tDynamicBuffer *mul_operand_1_int2float = label_name_gen("mul_operand_1_int2float");
    tDynamicBuffer *mul_calc = label_name_gen("mul_calc");

    // CHECK TYPE OF OPERAND 1
    // If operand 1 is float, jump to mul_operand_1_float label
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_1_TYPE\n");

    // If operand 1 is int, jump to mul_operand_1_int label
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_1_TYPE\n");

    // If operand 1 is null, jump to mul_operand_1_null label
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_1_TYPE\n");

    // Operand 1 is of invalid type
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CHECK TYPE OF OPERAND 2 IF FIRST WAS FLOAT
    // mul_operand_1_float label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // If FLOAT * FLOAT, jump to mul_calc and evaluate result
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // If FLOAT * INT, jump to mul_operand_2_int2float and convert second operand from INT to FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // If FLOAT * NULL, jump to mul_operand_1_float_operand_2_null, convert both operands to 0.0 as the result will be 0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // FLOAT * [STRING, BOOL] is invalid combination
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CHECK TYPE OF OPERAND 2 IF FIRST WAS INT
    // mul_operand_1_int label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // If INT * FLOAT, jump to mul_operand_1_int2float and convert first operand from INT to FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // If INT * INT, jump to mul_calc and evaluate result
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // If INT * NULL, jump to mul_operand_1_int_operand_2_null, convert both operands to 0 as the result will be 0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // INT * [STRING, BOOL] is invalid combination
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CHECK TYPE OF OPERAND 2 IF FIRST WAS NULL
    // mul_operand_1_null label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // If NULL * FLOAT, jump to mul_operand_1_float_operand_2_null, convert both operands to 0.0 as the result will be 0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // If NULL * INT, jump to mul_operand_1_int_operand_2_null, convert both operands to 0 as the result will be 0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // If NULL * NULL, jump to mul_operand_1_int_operand_2_null, convert both operands to 0 as the result will be 0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // NULL * [STRING, BOOL] is invalid combination
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CONVERT OPERAND 1 FROM INT TO FLOAT
    // mul_operand_1_int2float label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_1 TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // CONVERT OPERAND 2 FROM INT TO FLOAT
    // mul_operand_2_int2float label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_2 TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // CONVERT BOTH OPERANDS TO 0
    // mul_operand_1_int_operand_2_null label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // CONVERT BOTH OPERANDS TO 0.0
    // mul_operand_1_float_operand_2_null label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // EVALUATE RESULT
    // mul_calc label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, mul_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");

    dynamicBuffer_ADD_STRING(instruction, "MULS");

    // Free labels for jumps
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

    // Generate labels for jumps
    tDynamicBuffer *div_op_1_float = label_name_gen("div_op_1_float");
    tDynamicBuffer *div_op_1_int2float = label_name_gen("div_op_1_int2float");
    tDynamicBuffer *div_op_1_null2float = label_name_gen("div_op_1_null2float");
    tDynamicBuffer *div_op_2_float = label_name_gen("div_op_2_float");
    tDynamicBuffer *div_op_2_int2float = label_name_gen("div_op_2_int2float");
    tDynamicBuffer *div_op_2_null2float = label_name_gen("div_op_2_null2float");

    // CHECK TYPE OF OPERAND 1
    // If operand 1 is float, jump to div_op_1_float label
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@float\n");

    // If operand 1 is int, jump to div_op_1_int2float label and convert first operand from INT to FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@int\n");

    // If operand 1 is null, jump to div_op_1_null2float label and convert first operand from NULL to 0.0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_null2float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_1_TYPE string@nil\n");

    // If operand 1 is [STRING, BOOL], it is invalid and exit
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // OPERAND 1 INT TO FLOAT
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_1 TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, div_op_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // OPERAND 1 NULL TO 0.0
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

    // CHECK OPERAND 2 TYPE
    // If operand 2 is FLOAT jump to div_op_2_float label
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@float\n");

    // If operand 2 is INT jump to div_op_2_int2float label and convert second operand to FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@int\n");

    // If operand 2 is NULL jump to div_op_2_null2float label and convert second operand to 0.0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_null2float->data);
    dynamicBuffer_ADD_STRING(instruction, " TF@$TMP_2_TYPE string@nil\n");

    // IF OPERAND 2 IS [STRING, BOOL] it is invalid and exit
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

    // EVALUATE RESULT
    // OPERAND 2 WAS CONVERTED TO FLOAT (or was float already)
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, div_op_2_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");

    dynamicBuffer_ADD_STRING(instruction, "DIVS");

    // Free labels for jumps
    dynamicBufferFREE(div_op_1_float);
    dynamicBufferFREE(div_op_1_int2float);
    dynamicBufferFREE(div_op_1_null2float);
    dynamicBufferFREE(div_op_2_float);
    dynamicBufferFREE(div_op_2_int2float);
    dynamicBufferFREE(div_op_2_null2float);

    insert_instruction(instruction_list, instruction);
}

void gen_adds_subs(DLList *instruction_list, tDynamicBuffer *instruction, char *op_instruction){
    save_create_tf(instruction);
    def_tmp_get_type(instruction);

    // Generate labels for jumps
    tDynamicBuffer *add_sub_operand_1_float = label_name_gen("add_sub_operand_1_float");
    tDynamicBuffer *add_sub_operand_1_int = label_name_gen("add_sub_operand_1_int");
    tDynamicBuffer *add_sub_operand_1_null = label_name_gen("add_sub_operand_1_null");
    tDynamicBuffer *add_sub_operand_1_float_operand_2_null = label_name_gen("add_sub_operand_1_float_operand_2_null");
    tDynamicBuffer *add_sub_operand_1_int_operand_2_null = label_name_gen("add_sub_operand_1_int_operand_2_null");
    tDynamicBuffer *add_sub_operand_1_null_operand_2_null = label_name_gen("add_sub_operand_1_null_operand_2_null");
    tDynamicBuffer *add_sub_operand_2_int_operand_1_null = label_name_gen("add_sub_operand_2_int_operand_1_null");
    tDynamicBuffer *add_sub_operand_2_float_operand_1_null = label_name_gen("add_sub_operand_2_float_operand_1_null");
    tDynamicBuffer *add_sub_operand_2_int2float = label_name_gen("add_sub_operand_2_int2float");
    tDynamicBuffer *add_sub_operand_1_int2float = label_name_gen("add_sub_operand_1_int2float");
    tDynamicBuffer *add_sub_calc = label_name_gen("add_sub_calc");

    // CHECK OPERAND 1 TYPE
    // If operand 1 is FLOAT jump to add_sub_operand_1_float label
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_1_TYPE\n");

    // If operand 1 is INT jump to add_sub_operand_1_int label
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_1_TYPE\n");

    // If operand 1 is NULL jump to add_sub_operand_1_null label
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_1_TYPE\n");

    // OPERAND 1 IS INVALID
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CHECK OPERAND 2 TYPE IF OPERAND 1 WAS FLOAT
    // add_sub_operand_1_float label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // If FLOAT + FLOAT, jump to add_sub_calc and evaluate result
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // If FLOAT + INT, jump to add_sub_operand_2_int2float and convert second operand from INT to FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // If FLOAT + NULL, jump to add_sub_operand_1_float_operand_2_null and convert second operand from NULL to 0.0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // FLOAT + [STRING, BOOL] is invalid
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CHECK OPERAND 2 TYPE IF OPERAND 1 WAS INT
    // add_sub_operand_1_int label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_int->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // If INT + FLOAT, jump to add_sub_operand_1_int2float and convert first operand from INT to FLOAT
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // If INT + INT, jump to add_sub_calc and evaluate result
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // If INT + NULL, jump to add_sub_operand_1_int_operand_2_null and convert second operand from NULL to 0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // INT + [STRING, BOOL] is invalid
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CHECK OPERAND 2 TYPE IF OPERAND 1 WAS NULL
    // add_sub_operand_1_null label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // If NULL + FLOAT, jump to add_sub_operand_2_float_operand_1_null and convert first operand from NULL to 0.0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_2_float_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@float TF@$TMP_2_TYPE\n");

    // If NULL + INT, jump to add_sub_operand_2_int_operand_1_null and convert first operand from NULL to 0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_2_int_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@int TF@$TMP_2_TYPE\n");

    // If NULL + INT, jump to add_sub_operand_2_int_operand_1_null and convert both operands from NULL to 0
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_null_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // NULL + [STRING, BOOL] is invalid
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CONVERT FIRST OPERAND FROM INT TO FLOAT
    // add_sub_operand_1_int2float label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_1 TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // CONVERT SECOND OPERAND FROM INT TO FLOAT
    // add_sub_operand_2_int2float label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_2_int2float->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "INT2FLOAT TF@$TMP_2 TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // CONVERT SECOND OPERAND FROM NULL TO 0
    // add_sub_operand_1_int_operand_2_null label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_int_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // CONVERT SECOND OPERAND FROM NULL TO 0.0
    // add_sub_operand_1_float_operand_2_null LABEL
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_float_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // CONVERT FIRST OPERAND FROM NULL TO 0
    // add_sub_operand_2_int_operand_1_null label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_2_int_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // CONVERT FIRST OPERAND FROM NULL TO 0.0
    // add_sub_operand_2_float_operand_1_null label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_2_float_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 float@0x0p+0\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // CONVERT BOTH OPERANDS FROM NULL TO 0
    // add_sub_operand_1_null_operand_2_null label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_operand_1_null_operand_2_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 int@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // EVALUATE RESULT
    // add_sub_calc label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, add_sub_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");

    dynamicBuffer_ADD_STRING(instruction, op_instruction);

    // Free labels for jumps
    dynamicBufferFREE(add_sub_operand_1_float);
    dynamicBufferFREE(add_sub_operand_1_int);
    dynamicBufferFREE(add_sub_operand_1_null);
    dynamicBufferFREE(add_sub_operand_1_int2float);
    dynamicBufferFREE(add_sub_operand_1_float_operand_2_null);
    dynamicBufferFREE(add_sub_operand_1_int_operand_2_null);
    dynamicBufferFREE(add_sub_operand_2_int2float);
    dynamicBufferFREE(add_sub_operand_1_null_operand_2_null);
    dynamicBufferFREE(add_sub_operand_2_int_operand_1_null);
    dynamicBufferFREE(add_sub_operand_2_float_operand_1_null);
    dynamicBufferFREE(add_sub_calc);

    insert_instruction(instruction_list, instruction);
}

void gen_concat(DLList *instruction_list, tDynamicBuffer *instruction){
    save_create_tf(instruction);
    def_tmp_get_type(instruction);

    // Generate labels for jumps
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

    // [INT, FLOAT. BOOL] is invalid
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CHECK OPERAND 2 IF OPERAND 1 WAS STRING
    // concat_operand_1_str label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // If STRING . STRING, evaluate result
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, " string@string TF@$TMP_2_TYPE\n");

    // If STRING . NULL, convert second operand from NULL to ""
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_2_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // STRING . [INT, FLOAT. BOOL] is invalid
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // CHECK OPERAND 2 IF OPERAND 1 WAS NULL
    // concat_operand_1_null label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_null->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // If NULL . STRING, convert first operand from NULL to ""
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, " string@string TF@$TMP_2_TYPE\n");

    // If NULL . NULL, convert both operands from NULL to ""
    dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, concat_operands_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, " string@nil TF@$TMP_2_TYPE\n");

    // STRING . [INT, FLOAT. BOOL] is invalid
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");

    // Convert second operand from NULL to ""
    // concat_operand_2_null2str label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_2_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // Convert first operand from NULL to ""
    // concat_operand_1_null2str label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operand_1_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    //Convert both operands from NULL to ""
    // concat_operands_null2str label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_operands_null2str->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_1 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "MOVE TF@$TMP_2 string@\n");
    dynamicBuffer_ADD_STRING(instruction, "JUMP ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    // EVALUATE RESULT
    // concat_calc label
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, concat_calc->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");

    dynamicBuffer_ADD_STRING(instruction, "CONCAT TF@$TMP_1 TF@$TMP_1 TF@$TMP_2\n");
    dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@$TMP_1\n");
    dynamicBuffer_ADD_STRING(instruction, "POPFRAME");

    // Free labels for jumps
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

    // Generate labels for jumps
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

    // Free labels for jumps
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

    // Generate labels for jumps
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

    // Free labels for jumps
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
// EXPRESSION CODEGEN END

void print_instructions(DLList *instruction_list){
    DLL_instruction *tmp = instruction_list->first;
    //looping through whole list and printing instructions
    while(tmp != NULL){
        printf("%s\n", tmp->instruction->data);
        tmp = tmp->nextElement;
    }

}
