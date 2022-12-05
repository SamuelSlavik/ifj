#include <stdlib.h>
#include "dll_instruction_list.h"
#include "dynamic_buffer.h"
#include "expression_codegen.h"

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
    tDynamicBuffer *loopStart = label_name_gen("loopStart");
    tDynamicBuffer *loopWrite = label_name_gen("loopWrite");
    tDynamicBuffer *loopEnd = label_name_gen("loopEnd");

    dynamicBuffer_ADD_STRING(instruction,"LABEL functionWrite\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@term\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@numberOfArguments\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@numberOfArguments\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction, loopStart->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction, loopEnd->data);
    dynamicBuffer_ADD_STRING(instruction," TF@numberOfArguments int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@term\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction,loopWrite->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction, loopWrite->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"WRITE TF@term\n");
    dynamicBuffer_ADD_STRING(instruction,"SUB TF@numberOfArguments TF@numberOfArguments int@1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction, loopStart->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
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
    tDynamicBuffer *argumentIsNil = label_name_gen("argumentIsNil");
    tDynamicBuffer *ending = label_name_gen("ending");
    tDynamicBuffer *isUnsupported = label_name_gen("isUnsupported");

    dynamicBuffer_ADD_STRING(instruction,"LABEL functionFloatVal\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argumentType\n");
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
    dynamicBuffer_ADD_STRING(instruction,"INT2FLOAT TF@argument1 TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction, ending->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction, "LABEL ");
    dynamicBuffer_ADD_STRING(instruction, argumentIsNil->data);
    dynamicBuffer_ADD_STRING(instruction, "\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS float@0x0.0p+0\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
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
    tDynamicBuffer *argumentIsNil = label_name_gen("argumentIsNil");
    tDynamicBuffer *ending = label_name_gen("ending");
    tDynamicBuffer *isUnsupported = label_name_gen("isUnsupported");
    
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionIntVal\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argumentType\n");
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
    dynamicBuffer_ADD_STRING(instruction,"FLOAT2INT TF@argument1 TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction,ending->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction,argumentIsNil->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN\n");
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
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction, isNil->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHS string@\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP ");
    dynamicBuffer_ADD_STRING(instruction,ending->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL ");
    dynamicBuffer_ADD_STRING(instruction, isUnsupported->data);
    dynamicBuffer_ADD_STRING(instruction,"\n");
    dynamicBuffer_ADD_STRING(instruction,"EXIT int@4\n");
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

// ////////////////////////////////////////////////////////
// TO DO SUBSTRING
// ///////////////////////////////////////////////////////
void generate_substr(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
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

    dynamicBuffer_ADD_STRING(instruction,"GT TF@tmp TF@i TF@j\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");

    dynamicBuffer_ADD_STRING(instruction,"LT TF@tmp TF@i int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");

    dynamicBuffer_ADD_STRING(instruction,"LT TF@tmp TF@j int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");

    dynamicBuffer_ADD_STRING(instruction,"STRLEN TF@length TF@string\n");
    dynamicBuffer_ADD_STRING(instruction,"GT TF@tmp TF@i TF@length\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");
    dynamicBuffer_ADD_STRING(instruction,"GT TF@tmp TF@j TF@length\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ ");
    dynamicBuffer_ADD_STRING(instruction,error->data);
    dynamicBuffer_ADD_STRING(instruction," TF@tmp bool@true\n");
    
    // ///////////////////////////////// HERE YA GO
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
// /////////////////////////////////////////////////////////////////////////////////////////////

void generate_ord(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionOrd\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@argument1\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@argument1\n");
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