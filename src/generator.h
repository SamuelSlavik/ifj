/**
 * @file generator.c
 * @brief Implementation of built in functions and working with ifjcode22
 * @author Adam Pekný (xpekny00), Samuel Slávik (xslavi37), Jakub Kontrík (xkontr00)
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

// EXPRESSION CODEGEN START
/**
 * @brief Function for generating string with unique id
 *
 * @param name String to which the id should be added
 *
 * @return Dynamic buffer containing unique string as data
 * */
tDynamicBuffer *label_name_gen(char* name);

/**
 * @brief Function for generating IFJcode22 that checks whether variable has been initialized on runtime
 *
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 * @param var_id Dynamic buffer containing variable identificator as data
 *
 * */
void var_init_check(DLList *instruction_list, tDynamicBuffer *var_id);

/**
 * @brief Function for generating IFJcode22 that pushes TF to stack of LFs and creates new TF
 *
 * @param instruction Dynamic buffer to store those instructions as string
 *
 * */
void save_create_tf(tDynamicBuffer *instruction);

/**
 * @brief Function for inserting dynamic buffer with IFJcode22 instructions into instruction list and freeing the buffer
 *
 * @param instruction Dynamic buffer containing IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 *
 * */
void insert_instruction(DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Function for generating IFJcode22 that defines and inits temporary variables for operands on TF and gets their type
 *
 * @param instruction Dynamic buffer containing IFJcode22 instructions as string
 *
 * */
void def_tmp_get_type(tDynamicBuffer *instruction);

/**
 * @brief Function for generating IFJcode22 that evaluates multiplication of 2 operands
 *
 * @param instruction Dynamic buffer to store IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 *
 * */
void gen_muls(DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Function for generating IFJcode22 that evaluates division of 2 operands
 *
 * @param instruction Dynamic buffer to store IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 *
 * */
void gen_divs(DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Function for generating IFJcode22 that evaluates addition or subtraction of 2 operands
 *
 * @param instruction Dynamic buffer to store IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 * @param op_instruction String containing instruction of operator (ADDS for addition SUBS for subtraction)
 *
 * */
void gen_adds_subs(DLList *instruction_list, tDynamicBuffer *instruction, char *op_instruction);

/**
 * @brief Function for generating IFJcode22 that evaluates concatenation of 2 operands
 *
 * @param instruction Dynamic buffer to store IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 *
 * */
void gen_concat(DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Function for generating IFJcode22 that evaluates less than or greater than comparison of 2 operands
 *
 * @param instruction Dynamic buffer to store IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 * @param cmp_instruction String containing instruction of comparison (LTS for less than GTS for greater than)
 *
 * */
void gen_lts_gts(DLList *instruction_list, tDynamicBuffer *instruction, char *cmp_instruction);

/**
 * @brief Function for generating IFJcode22 that evaluates less than or equal comparison of 2 operands
 *
 * @param instruction Dynamic buffer to store IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 *
 * */
void gen_ltes(DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Function for generating IFJcode22 that evaluates greater than or equal comparison of 2 operands
 *
 * @param instruction Dynamic buffer to store IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 *
 * */
void gen_gtes(DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Function for generating IFJcode22 that evaluates equality comparison of 2 operands
 *
 * @param instruction Dynamic buffer to store IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 *
 * */
void gen_eqs(DLList *instruction_list, tDynamicBuffer *instruction);

/**
 * @brief Function for generating IFJcode22 that evaluates non-equality comparison of 2 operands
 *
 * @param instruction Dynamic buffer to store IFJcode22 instructions as string
 * @param instruction_list Double linked list used to store IFJcode22 instructions in correct order
 *
 * */
void gen_neqs(DLList *instruction_list, tDynamicBuffer *instruction);
// EXPRESSION CODEGEN END

#endif // GENERATOR_H
