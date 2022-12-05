#ifndef GENERATOR_H
#define GENERATOR_H
#include <stdlib.h>
#include "dll_instruction_list.h"
#include "dynamic_buffer.h"

void generate_write(tDynamicBuffer *instruction, DLList *instruction_list);

#endif