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


enum automat_state {
    S_ERROR,
    S_START,
    //
    S_COLON,                // :
    S_DOT,                  // .
    S_COMMA,                // ,
    S_SEMICOLON,            // ;
    S_R_BRAC,               // {
    S_L_BRAC,               // }
    S_R_PAR,                // (
    S_L_PAR,                // ) 
    //
    S_EQUAL,                // =
    S_GREATER_THAN,         // >
    S_LESS_THAN,            // <
    //
    S_ASTERISK,             // *
    S_SLASH,                // /
    S_PLUS,                 // +
    S_MINUS,                // -
    //
    S_QUOTE,                // "
    S_DOLLAR,               // $
    S_QUESTIONER,           // ?
    S_EXCLAMATION,          // !
    S_LETTER,
    S_NUM,

};


/**
 * @brief Possible Token types
 * 
 */
enum token_type {
    T_STRING_TYPE,
    T_FLOAT_TYPE,
    T_INT_TYPE,
    // T_NUM_EXP,
    T_VAR_ID,
    T_FUN_ID,
    T_STRING,
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
    T_NOT_EQUALS,           // !==
    T_GREATER_THAN,         // >
    T_GREATER_OR_EQUAL,     // >=
    T_LESS_THAN,            // <
    T_LESS_OR_EQUAL,        // <=
    //
    T_MUL,                  // *
    T_DIV,                  // /
    T_ADD,                  // +
    T_SUB,                  // -
    T_PROLOG_START,         // <?php    
    T_PROLOG_END,           // ?>
    //
    T_WHILE,
    T_IF,
    T_ELSE,
    T_FUNCTION,
    T_RETURN,
    T_VOID,
    //
    T_EOF,
    T_UNKNOW,
    T_COMMENT_ERROR,
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
    enum token_type type;
    tToken_value data;
} tToken;


/*************** TESTING ***************/

    #ifdef TESTING

    #define ANSI_COLOR_YELLOW  "\x1b[33m"
    #define ANSI_COLOR_RED     "\x1b[31m"
    #define ANSI_COLOR_RESET   "\x1b[0m"

    void print_token_type(enum token_type type);

    #endif // TESTING

#endif // SCANNER_H