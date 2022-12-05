#ifndef GENERATOR_H
#define GENERATOR_H
#include <stdlib.h>
#include "dll_instruction_list.h"
#include "dynamic_buffer.h"

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
#endif