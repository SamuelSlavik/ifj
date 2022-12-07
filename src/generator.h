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
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_write(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Reads, generates code which reads string from standard input
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_reads(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Readi, generates code which reads an integer from standard input
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_readi(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Readf, generates code which reads a float from standard input
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_readf(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Intval, generates code which converts and return given term to integer
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_intval(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Floatval, generates code which converts and return given term to float
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_floatval(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Strval, generates code which converts and return given term to string
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_strval(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Strlen, generates code which returns length of given string
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_strlen(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Substr, generates code which return substring of given string according to starting and ending index
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_substr(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Chr, generates code which returns character with given ascii value
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_chr(tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Built in function Ord, generates code which returns ordinal-value(ascii) of first letter from given string
 * 
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list pointer to double linked list to store instruction buffer at correct location 
 */
void generate_ord(tDynamicBuffer *instruction, DLList *instruction_list);


void check_fn_arguments(tStack *defined_params, tStack *called, tDynamicBuffer *instruction, DLList *instruction_list);
void convert_into_bool(tDynamicBuffer *instruction, DLList *instruction_list,tDynamicBuffer *labelname);
void check_return_type(tDynamicBuffer *instruction, DLList *instruction_list);
void print_stack(tStack *expr_stack, tDynamicBuffer *instruction, DLList *instruction_list,char *code);
void print_instructions(DLList *instruction_list);

#endif // GENERATOR_H
