/**
 * @file scanner.h
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.1
 * @date 2022-10-12
 * 
 */

#include <stdio.h>
#include "dynamic_buffer.h"


#ifndef SCANNER_H
#define SCANNER_H


/**
 * @brief Possible Token types
 * 
 */
enum token_type {
    T_STRING,
    T_NUM_FLOAT,
    T_NUM_INT,
    // T_NUM_EXP,
    T_VAR_ID,
    T_TYPE_ID,
    T_FUN_ID,
    //
    T_COLON,                // :
    T_CONCATENATION,        // .
    T_COMMA,                // ,
    T_SEMICOLON,            // ;
    T_R_BRAC,               // {
    T_L_BRAC,               // }
    T_R_PAR,                // (
    T_L_PAR,                // )
    //
    T_ASSIGN,               // =
    T_EQUALS,               // === 
    T_NOT_EQUALS,           // !===
    T_MORE_THAN,            // >
    T_MORE_OR_EQUAL,        // >=
    T_LESS_THAN,            // <
    T_LESS_OR_EQUAL,        // <=
    // * | / | + | -
    T_MUL,
    T_DIV,
    T_ADD,
    T_SUB,
    // PROLOG_START,
    // PROLOG_END,
    //
    T_WHILE,
    T_IF,
    T_ELSE,
    T_FUNCTION,
    T_RETURN,
    T_VOID,
    T_STRING,
    T_INT,
    T_FLOAT,
    //
    T_UNKNOW,
    T_ERROR,
};


/**
 * @brief Value types for Token data
 * 
 */
typedef union token_value
{
    tDynamicBuffer* STRINGval;
    long INTval;
    double FLOATval;
} tToken_value;


/**
 * @brief Token structure
 * 
 */
typedef struct Token
{
    enum token_type;
    tToken_value data;
} tToken;


#endif // SCANNER_H