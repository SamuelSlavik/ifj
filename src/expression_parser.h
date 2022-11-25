/**
 * @file expression_parser.h
 * @author Adam Pekný (xpekny00)
 * @brief
 * @version 0.1
 * @date 2022-11-18
 *
 */


#ifndef IFJ_EXPRESSION_PARSER_H
#define IFJ_EXPRESSION_PARSER_H

#include "scanner.h"
#include "stack.h"
#include "dll_instruction_list.h"


#define PRECED_TAB_SIZE 15

#define PRECED_TAB const char preced_tab[PRECED_TAB_SIZE][PRECED_TAB_SIZE] = { \
    { '>' , '>' , '<' , '<' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '>' , '>' , '<' , '<' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '>' , '>' , '<' , '<' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '<' , '<' , '<' , '<' , '<' , '>' , '>' , '>' , '>' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '<' , '<' , '<' , '<' , '<' , '>' , '>' , '>' , '>' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '<' , '<' , '<' , '<' , '<' , '>' , '>' , '>' , '>' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '<' , '<' , '<' , '<' , '<' , '>' , '>' , '>' , '>' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '>' , '>' , '<'  , '>'  , '<'  , '>' },\
    { '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<'  , '='  , '<'  , '\0'},\
    { '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '\0' , '>'  , '\0' , '>' },\
    { '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '>' , '\0' , '>'  , '\0' , '>' },\
    { '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<' , '<'  , '\0' , '<'  , '\0'}\
}

enum expr_item_type{
    T_ADD_EXPR,
    T_SUB_EXPR,
    T_MUL_EXPR,
    T_DIV_EXPR,
    T_CONCAT_EXPR,
    T_LT_EXPR,
    T_GT_EXPR,
    T_LTE_EXPR,
    T_GTE_EXPR,
    T_EQ_EXPR,
    T_NEQ_EXPR,
    T_LPAR_EXPR,
    T_RPAR_EXPR,
    T_OPERAND_EXPR,
    T_END_EXPR,
    T_UNKNOW_EXPR,
};

typedef struct expr_item{
    tToken *token;
    enum expr_item_type type;
    bool is_terminal;
    bool handle;
}tExprItem;

tDynamicBuffer *label_name_gen(char* name);

tDynamicBuffer *double_2_string(double num);
tDynamicBuffer *long_2_string(long int num);

bool check_expr_syntax(tToken *start_token, tToken *end_token, DLList *instruction_list, tToken *extra_token);

bool is_expr_end_token(tToken *current_token, tToken *end_token, size_t *par_level);

bool is_stack_end_state(tStack *expr_stack, tToken *end_token);

tExprItem *get_stack_top_terminal(tStack *expr_stack);

enum expr_item_type token_to_preced_idx(enum token_type token_type, bool is_end);

void clean_expr_stack(tStack *expr_stack);

//void print_stack(tStack *expr_stack);

#endif //IFJ_EXPRESSION_PARSER_H
