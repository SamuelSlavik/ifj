/**
 * @file expression_parser.h
 * @author Adam Pekný (xpekny00)
 * @brief Header file for expression_parser.c
 *
 */


#ifndef IFJ_EXPRESSION_PARSER_H
#define IFJ_EXPRESSION_PARSER_H

#include "stack.h"
#include "scanner.h"
#include "htab.h"
#include "dll_instruction_list.h"
#include "generator.h"
#include "error.h"

/**
 * @brief Macro for number of rows and columns in precedence table
 *
 */
#define PRECED_TAB_SIZE 15

/**
 * @brief Macro for definition and initialization of precedence table.
 * @brief Columns (top-down) and rows(left-right) are in order: + - * / . < > <= >= === !== ( ) i $
 */
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

/**
 * @brief Types of expression items that can occur in expression
 *
 */
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

/**
 * @brief Structure for items in expression stack, to keep important metadata
 *
 */
typedef struct expr_item{
    tToken *token; // Token representing item if it is terminal
    enum expr_item_type type; // Expression item type (also index in precedence table)
    bool is_terminal;
    bool handle; // Mark telling that after this item a reduction should be done
}tExprItem;

/**
 * @brief Function for filling decadic IFJcode22 escape sequence with 0s at beginning if needed
 *
 * @param int_val decadic value of character in escape sequence
 * @param string dynamic buffer where the escape sequence should be stored
 */
void format_esc_sq(long int_val, tDynamicBuffer *string);

/**
 * @brief Function for converting string from php format to IFJcode22 format
 *
 * @param string dynamic buffer containing string to be converted (it is both source and destination)
 */
void string_to_ifj_fmt(tDynamicBuffer **string);

/**
 * @brief Function for converting double to string
 *
 * @param num numeric value of type double to be converted
 *
 * @return dynamic buffer containing converted string as data
 */
tDynamicBuffer *double_2_string(double num);

/**
 * @brief Function for converting double to string
 *
 * @param num numeric value of type long int to be converted
 *
 * @return dynamic buffer containing converted string as data
 */
tDynamicBuffer *long_2_string(long int num);

/**
 * @brief Function for checking syntax, performing semantic checks and generating code for expressions
 *
 * @param start_token pointer to the first token of expression (will be the second token if extra_token is not NULL)
 * @param end_token pointer to token of type that should end expression
 * @param instruction_list double linked list to store all generated IFJcode22 instructions in correct order
 * @param extra_token pointer to first token of expression if 2 tokens were needed to decide if code is expression (pass NULL if not needed)
 *
 * @return bool value telling if stack is in correct end state
 */
bool parse_expression(tToken *start_token, tToken *end_token, DLList *instruction_list, tToken *extra_token);

/**
 * @brief Function for deciding whether token is the token that should end expression
 *
 * @param current_token token to be checked
 * @param end_token pointer to token of type that should end expression
 * @param par_level pointer to number of level of nested parenthesis
 *
 * @return bool value telling if stack is in correct end state
 */
bool is_expr_end_token(tToken *current_token, tToken *end_token, size_t *par_level);

/**
 * @brief Function for deciding whether expression stack is in correct end state (ENDINGTOKEN EXPRESSIONITEM)
 *
 * @param expr_stack stack which state to analyze
 * @param end_token pointer to token of type that should end expression
 *
 * @return bool value telling if stack is in correct end state
 */
bool is_stack_end_state(tStack *expr_stack, tToken *end_token);

/**
 * @brief Function for getting the topmost terminal in expression stack
 *
 * @param expr_stack stack to look in for the topmost terminal
 *
 * @return pointer to topmost terminal in expression stack
 */
tExprItem *get_stack_top_terminal(tStack *expr_stack);

/**
 * @brief Function for converting token type to expression token type (it is also index in precedence table)
 *
 * @param token_type type of token to convert
 * @param is_end value telling function if the token type is also token type ending expression
 *
 * @return enum value of expression token type (also precedence table index)
 */
enum expr_item_type token_to_preced_idx(enum token_type token_type, bool is_end);

/**
 * @brief Function for freeing all memory allocated to token in expression parser
 *
 * @param token token to be freed
 */
void free_expr_token(tToken **token);

/**
 * @brief Function for cleaning expression stack and freeing all its allocated memory
 *
 * @param expr_stack expression stack to be cleaned
 */
void clean_expr_stack(tStack *expr_stack);

#endif //IFJ_EXPRESSION_PARSER_H
