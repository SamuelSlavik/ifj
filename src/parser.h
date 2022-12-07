/**
 * @file parser.c
 * @brief Implementation of recursive top-down parser
 * @authors Jakub Kontrik (xkontr02), Adam Pekny(xpekny00), Samuel Slavik (xslavi37)
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"
#include "dll_instruction_list.h"

//macro for checking conditions and then exiting with error code
#define ASSERT_ERROR(flag,token,errcode) if (!(flag)){error_exit(token,errcode);}
//macro that inserts instruction at right position checks if instruction is called in function or in main
#define DETECT_MAIN(instruction_list,instruction,key) \
if(!strcmp(key,"$$main")){\
DLL_InsertAfter_main(instruction_list,instruction);\
if(instruction_list->active==instruction_list->main_body)\
{DLL_Next(instruction_list);}\
DLL_Next_main(instruction_list);}\
else{\
DLL_InsertAfter(instruction_list,instruction); DLL_Next(instruction_list);\
}

/**
 * @brief Implementation of grammar rule for <start>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_start(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rules for <prog>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_prog(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rules for <body>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_body(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rules for <body_var> 
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_body_var(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rules for <body_ret>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_body_ret(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rules for <fn_call_l>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_fn_call_l(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rules for <fn_call_lc>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_fn_call_lc(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rule for <fn_call_lparam>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_fn_call_lparam(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rule for <fn>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_func(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rule for <fn_dedf>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_func_dedf(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rule for <fn_type> 
 * 
 * @param token pointer to current stored token
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_func_type(tToken *token, DLList *instruction_list);

/**
 * @brief Implementation of grammar rules for <in_body>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_in_body(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rules for <fn_param>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_func_param(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rule for <fn_dedf_param_type>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_func_dedf_param_type(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

/**
 * @brief Implementation of grammar rules for <fn_dedf_param_var>
 * 
 * @param token pointer to current stored token
 * @param instruction pointer to buffer to store string with instructions
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 */
bool f_func_dedf_param_var(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list);

#endif //PARSER_H
