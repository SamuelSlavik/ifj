/**
 * @file generator.c
 * @brief Implementation of built in functions and working with ifjcode22
 * @author Samuel Slávik (xslavi37), Jakub Kontrík (xkontr02)  
 */

#include <stdlib.h>
#include "dll_instruction_list.h"
#include "dynamic_buffer.h"
#include "expression_codegen.h"
#include "parser.h"
#include "generator.h"

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

void print_instructions(DLList *instruction_list){
    DLL_instruction *tmp = instruction_list->first;
    //looping through whole list and printing instructions
    while(tmp != NULL){
        printf("%s\n", tmp->instruction->data);
        tmp = tmp->nextElement;
    }

}
