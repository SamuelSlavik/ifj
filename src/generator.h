/**
 * @file generator.c
 * @brief Implementation of built in functions and working with ifjcode22
 * @author Samuel Slávik (xslavi37), Jakub Kontrík (xkontr00)  
 */

#ifndef GENERATOR_H
#define GENERATOR_H
#include <stdlib.h>
#include "dll_instruction_list.h"
#include "dynamic_buffer.h"
#include "stack.h"

/**
 * @brief Built in function Write, generates code which writes terms on standard output
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_write(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Reads, generates code which reads string from standard input
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_reads(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Readi, generates code which reads an integer from standard input
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_readi(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Readf, generates code which reads a float from standard input
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_readf(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Intval, generates code which converts and return given term to integer
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_intval(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Floatval, generates code which converts and return given term to float
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_floatval(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Strval, generates code which converts and return given term to string
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_strval(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Strlen, generates code which returns length of given string
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_strlen(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Substr, generates code which return substring of given string according to starting and ending index
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_substr(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Chr, generates code which returns character with given ascii value
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_chr(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Ord, generates code which returns ordinal-value(ascii) of first letter from given string
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void generate_ord(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Function that generates ifjcode22 that checks if type of called arguments are matched with defined arguments
 * 
 * @param defined_params pointer to stack with stored function argument types and names of variables
 * @param called pointer to stack with stored parameters that was function called
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void check_fn_arguments(tStack *defined_params, tStack *called_params, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Function that generates ifjcode22 that converts value at top of the stack from expression parser into bool value
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 * @param label_name pointer to dynamic buffer that stores name of while or if label
 */
void convert_into_bool(tDynamicBuffer *instruction, DLList *instruction_list,tDynamicBuffer *label_name);

/**
 * @brief Function that generates ifjcode22 that checks if return type from function is matched with defined type
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void check_return_type(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Function that generates ifjcode22 that uses given code with each fuction argument from stack
 * 
 * @param defined_params pointer to stack with stored function argument types and names of variables
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 * @param code string that should be printed with each argument
 */
void print_stack(tStack *defined_params, tDynamicBuffer *instruction, DLList *instruction_list, char *code);

/**
 * @brief Function prints every stored instruction from DL list for valid ifjcode22
 * 
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
void print_instructions(DLList *instruction_list);

#endif // GENERATOR_H
