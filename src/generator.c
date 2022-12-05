#include <stdlib.h>
#include "dll_instruction_list.h"
#include "dynamic_buffer.h"

void generate_write(tDynamicBuffer *instruction, DLList *instruction_list){
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"LABEL functionWrite\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@term\n");
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR TF@numberOfArguments\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@numberOfArguments\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL loopStart\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMPIFEQ loopEnd TF@numberOfArguments int@0\n");
    dynamicBuffer_ADD_STRING(instruction,"POPS TF@term\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP loopWrite\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL loopWrite\n");
    dynamicBuffer_ADD_STRING(instruction,"WRITE TF@term\n");
    dynamicBuffer_ADD_STRING(instruction,"SUB TF@numberOfArguments TF@numberOfArguments int@1\n");
    dynamicBuffer_ADD_STRING(instruction,"JUMP loopStart\n");
    dynamicBuffer_ADD_STRING(instruction,"LABEL loopEnd\n");
    dynamicBuffer_ADD_STRING(instruction,"POPFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"RETURN");
    DLL_InsertAfter(instruction_list,instruction);
    DLL_Next(instruction_list);
    dynamicBufferFREE(instruction);
}