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
    S_PROLOG,
    //
    S_COLON,                // :
    S_DOT,                  // .
    S_COMMA,                // ,
    S_SEMICOLON,            // ;
    S_R_BRAC,               // }
    S_L_BRAC,               // {
    S_R_PAR,                // )
    S_L_PAR,                // ( 
    //
    S_EQUAL,                // =
    S_EQEQ,                 // ==
    S_ASSIGN,               // ===
    S_GREATER_THAN,         // >
    S_G_T_,                 // >[?]
    S_LESS_THAN,            // <
    S_L_T_,                 // <[?]
    //
    S_ASTERISK,             // *
    S_SLASH,                // /
    S_COMMENT,              // /[?]
    S_LINE_COMMENT,         // //
    S_BLOCK_COMMENT,        // /*
    S_B_C_END,              // /* ... *
    S_PLUS,                 // +
    S_MINUS,                // -
    //
    S_QUOTE,                // "
    S_DOLLAR,               // $
    S_V_ID,                 // $_a-zA-Z 
    S_QUESTIONER,           // ?
    S_TYPE_ID,              // ?int | ?string | ?float
    S_EXCLAMATION,          // !
    S_LETTER,
    S_INT_NUM,
    S_FLOAT_NUM,
    S_EXP_NUM,

};


/**
 * @brief Possible Token types
 * 
 */
enum token_type {
    T_STRING_TYPE,          // string
    T_FLOAT_TYPE,           // float
    T_INT_TYPE,             // int
    T_NULL,                 // null
    T_STRING_N_TYPE,        // ?string
    T_FLOAT_N_TYPE,         // ?float
    T_INT_N_TYPE,           // ?int
    // T_NUM_EXP,
    T_VAR_ID,
    T_FUN_ID,
    T_STRING,
    T_NUM_INT,
    T_NUM_FLOAT,
    //
    T_COLON,                // :
    T_CONCATENATION,        // .
    T_COMMA,                // ,
    T_SEMICOLON,            // ;
    T_R_BRAC,               // }
    T_L_BRAC,               // {
    T_R_PAR,                // )
    T_L_PAR,                // (
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
    T_PROLOG,    
    T_EPILOG,               // ?>
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
    T_COMMENT_ERROR,        // incomplete block of comment
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

/**
 * @brief Function for changis STATE from S_STATE
 * 
 * @param c char which decided what state will be choosen
 * @return enum automat_state 
 */
enum automat_state change_state(char c);


/**
 * @brief Function for loading loading string to token
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_string(tToken* token, char c, unsigned long *init_count);


/**
 * @brief Function for loading variable name of token
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_var_id(tToken* token, char c, unsigned long *init_count);


/**
 * @brief Function for loading types of token
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_type_id(tToken* token, char c, unsigned long *init_count);


/**
 * @brief Function for loading numbers to token
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_num(tToken* token, char c, unsigned long *init_count);


/**
 * @brief Transform string to int or float
 * 
 * @param token with data that will be proccessed
 */
void string_to_num(tToken* token);


/**
 * @brief Function return token type of reserved ID or type T_FUN_ID
 * 
 * @param string string that well be compared with reserved IDs
 * @return enum token_type 
 */
enum token_type is_reserved_id(tDynamicBuffer* string);


/**
 * @brief Function for loading token while is in state S_LETTER
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_letter(tToken* token, char c, unsigned long *init_count);


/**
 * @brief Create the token object loaded from stdin
 * @param automat_state there are 3 possible automat states
 * 0 => WAITING ON PROLOG
 * 1 => PROCESS PROGRAM (normall token loading)
 * 2 => CHECKING CORRECT END (after epilog occures)
 * 
 * @return Loaded Token
 */
tToken get_token(short automat_state);

/*************** TESTING ***************/

    #ifdef TESTING

    #define ANSI_COLOR_YELLOW  "\x1b[33m"
    #define ANSI_COLOR_RED     "\x1b[31m"
    #define ANSI_COLOR_RESET   "\x1b[0m"

    void print_token_type(enum token_type type);

    #endif // TESTING

#endif // SCANNER_H