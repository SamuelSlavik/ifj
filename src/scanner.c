/**
 * @file scanner.c
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.1
 * @date 2022-10-12
 * 
 */


#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "scanner.h"
#include "dynamic_buffer.h"

/**
 * @brief Function for changis STATE from S_STATE
 * 
 * @param c char which decided what state will be choosen
 * @return enum automat_state 
 */
enum automat_state change_state(char c){
    
    if (isalpha(c) || (c == '_')){
        return S_LETTER;
    }
    else if (isdigit(c)){
        return S_NUM;
    }
    else if(isspace(c)){
        return S_START;
    }

    switch (c)
    {
    case ':':
        return S_COLON;
        break;
    case '.':
        return S_DOT;
        break;
    case ',':
        return S_COMMA;
        break;
    case ';':
        return S_SEMICOLON;
        break;
    case '{':
        return S_R_BRAC;
        break;
    case '}':
        return S_L_BRAC;
        break;
    case '(':
        return S_R_PAR;
        break;
    case ')':
        return S_L_PAR;
        break;
    case '=':
        return S_EQUAL;
        break;
    case '>':
        return S_GREATER_THAN;
        break;
    case '<':
        return S_LESS_THAN;
        break;
    case '*':
        return S_ASTERISK;
        break;
    case '/':
        return S_SLASH;
        break;
    case '+':
        return S_PLUS;
        break;
    case '-':
        return S_MINUS;
        break;
    case '"':
        return S_QUOTE;
        break;
    case '$':
        return S_DOLLAR;
        break;
    case '?':
        return S_QUESTIONER;
        break;
    case '!':
        return S_EXCLAMATION;
        break;

    default:
        return S_ERROR;
        break;
    }
}

/**
 * @brief Function for loading token while is in state S_EQUAL
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_equal(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if (c == '='){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
        /* SET TOKEN TYPE */
        if ((strcmp(token->data.STRINGval->data, "===")) == 0){
                token->type = T_EQUALS;
            }    
        else if ((strcmp(token->data.STRINGval->data, "=")) == 0){
                token->type = T_ASSIGN;
            }
        else {
                token->type = T_ERROR;
            }
    }
    else{
        ungetc(c, stdin);
        *init_count = 0;
        dynamicBufferFREE(token->data.STRINGval);
    }
}

/**
 * @brief Function for loading token while is in state S_GREATER_THAN
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_greater_than(tToken* token, char c, unsigned long *init_count){
    /* INIT_BUFFER */
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if (c == '>' || c == '='){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
        /* SET TOKEN TYPE */
        if ((strcmp(token->data.STRINGval->data, ">=")) == 0){
            token->type = T_GREATER_OR_EQUAL;
        }
        else if ((strcmp(token->data.STRINGval->data, ">")) == 0){
            token->type = T_GREATER_THAN;
        }
        else {
            token->type = T_ERROR;
        }
    }
    else {
        ungetc(c, stdin);
        *init_count = 0;
        dynamicBufferFREE(token->data.STRINGval);
    }
}

/**
 * @brief Function for loading token while is in state S_LESS_THAN
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_less_than(tToken* token, char c, unsigned long *init_count){
    /* INIT_BUFFER */
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if (c == '<' || c == '=' || c == '?' || c == 'p' || c == 'h'){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
        /* SET TOKEN TYPE */
        if ((strcmp(token->data.STRINGval->data, "<=")) == 0){
            token->type = T_LESS_OR_EQUAL;
        }
        else if ((strcmp(token->data.STRINGval->data, "<?php")) == 0){
            token->type = T_PROLOG_START;
        }
        else if ((strcmp(token->data.STRINGval->data, "<")) == 0){
            token->type = T_LESS_THAN;
        }
        else{
            token->type = T_ERROR;
        }
    }
    else {
        ungetc(c, stdin);
        *init_count = 0;
        dynamicBufferFREE(token->data.STRINGval);
    }
}

/**
 * @brief Function for loading token while is in state S_SLASH
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_slash(tToken* token, char c, unsigned long *init_count){
    /* INIT_BUFFER */
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if ((c == '/' || c == '*') && 
        (strcmp(token->data.STRINGval->data, "/*")) &&  // is comment
        (strcmp(token->data.STRINGval->data, "//")) &&  // is comment
        (strcmp(token->data.STRINGval->data, "*"))){    // is end of comment

        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
    }
    else {
        if (((strcmp(token->data.STRINGval->data, "/*")) == 0) || ((strcmp(token->data.STRINGval->data, "*")) == 0)){

            if (((strcmp(token->data.STRINGval->data, "*")) == 0) && (c == '/')){  // end of comment
                token->data.STRINGval->data[1] = '/';    // "*" => "*/"
                return; // comment completed 
            }
            if (c == '*'){                                                         // possible end of comment (got '*')
                token->data.STRINGval->data[0] = c;      // 
                token->data.STRINGval->data[1] = '\0';   // "/*" => "*" 
            }
            else {                                                                 // keep starting state (no '/' after '*')
                token->data.STRINGval->data[0] = '/';    // 
                token->data.STRINGval->data[1] = '*';    // "*" => "/*" 
            }
            return; // incomplete comment
        }
        else if ((strcmp(token->data.STRINGval->data, "//")) == 0){
            return; // comment completed 
        }
        else if ((strcmp(token->data.STRINGval->data, "/")) == 0){
            token->type = T_DIV;
        }
        else{
            token->type = T_ERROR;
        }
        ungetc(c, stdin);
        *init_count = 0;
        dynamicBufferFREE(token->data.STRINGval);
        token->data.STRINGval = NULL;
    }
}

/**
 * @brief Function for loading token while is in state S_STRING
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_string(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if (c != '"'){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
    }
    else{
        token->type = T_STRING;
        return;
    }
}

/**
 * @brief Function for loading token while is in state S_DOLLAR
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_var_id(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if (c == '_' || isalpha(c) || isdigit(c)){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
    }
    else{
        token->type = T_VAR_ID;
    }
}

/**
 * @brief Function for loading token while is in state S_QUESTIONER
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_questioner(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if (isalpha(c) || isdigit(c) || c == '?' || (c == '>')){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
        /* SET TOKEN TYPE */
        if (((strcmp(token->data.STRINGval->data, "?>")) == 0)){
            token->type = T_PROLOG_END;
        }
        else if (((strcmp(token->data.STRINGval->data, "?string")) == 0)){
            token->type = T_STRING_TYPE;
        }
        else if (((strcmp(token->data.STRINGval->data, "?int")) == 0)){
            token->type = T_INT_TYPE;
        }
        else if (((strcmp(token->data.STRINGval->data, "?float")) == 0)){
            token->type = T_FLOAT_TYPE;
        }
        else{
            token->type = T_ERROR;
        }
    }
    else {
        ungetc(c, stdin);
        *init_count = 0;
        dynamicBufferFREE(token->data.STRINGval);
    }
}

/**
 * @brief Function for loading token while is in state S_EXCLAMATION
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_exclamation(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if (c == '=' || c == '!'){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
        /* SET TOKEN TYPE */
        if ((strcmp(token->data.STRINGval->data, "!==")) == 0){
                token->type = T_NOT_EQUALS;
            }    
        else {
                token->type = T_ERROR;
            }
    }
    else{
        ungetc(c, stdin);
        *init_count = 0;
        dynamicBufferFREE(token->data.STRINGval);
    }
}


void load_num(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if (isdigit(c) || c == 'e' || c == 'E'){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
    }
    else{
        ungetc(c, stdin);
        *init_count = 0;
    }
}


void string_to_num(tToken* token){

    char* err;

    long tmp_int;
    double tmp_double;

    /* IF INT */

    tmp_int = strtol(token->data.STRINGval->data, &err, 10);

    if (*err == '\0') {
        token->type = T_NUM_INT;
        dynamicBufferFREE(token->data.STRINGval);
        token->data.INTval = tmp_int;
        return;
    }
    
    /* IF FLOAT */

    tmp_double = strtod(token->data.STRINGval->data, &err);
    
    if (*err == '\0'){
        token->type = T_NUM_FLOAT;
        dynamicBufferFREE(token->data.STRINGval);
        token->data.FLOATval = tmp_double;
        return;
    }

    /* ELSE */
    dynamicBufferFREE(token->data.STRINGval);
    token->type = T_ERROR;
}


/**
 * @brief Function return token type of reserved ID or type T_FUN_ID
 * 
 * @param string string that well be compared with reserved IDs
 * @return enum token_type 
 */
enum token_type is_reserved_id(tDynamicBuffer* string){
    
    if ((strcmp(string->data, "string")) == 0){
        return T_STRING_TYPE;
    }
    else if ((strcmp(string->data, "int")) == 0){
        return T_INT_TYPE;
    }
    else if ((strcmp(string->data, "float")) == 0){
        return T_FLOAT_TYPE;
    }
    else if ((strcmp(string->data, "while")) == 0){
        return T_WHILE;
    }
    else if ((strcmp(string->data, "if")) == 0){
        return T_IF;
    }
    else if ((strcmp(string->data, "else")) == 0){
        return T_ELSE;
    }
    else if ((strcmp(string->data, "function")) == 0){
        return T_FUNCTION;
    }
    else if ((strcmp(string->data, "return")) == 0){
        return T_RETURN;
    }
    else if ((strcmp(string->data, "void")) == 0){
        return T_VOID;
    }
    else {
        return T_FUN_ID;
    }
}

/**
 * @brief Function for loading token while is in state S_LETTER
 * 
 * @param token token where chars will be loaded
 * @param c char that will be loaded
 * @param init_count control parameter for initialization
 */
void load_letter(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            /* err handle */
        }
        *init_count = 1;
    }
    /* PROCESS */
    if ((isalpha(c)) || (isdigit(c)) || (c == '_')){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            /*err handle*/
        }
        /* SET TOKEN TYPE */
        token->type = T_FUN_ID;
    }
    else{
        ungetc(c, stdin);
        *init_count = 0;
    }
}

/**
 * @brief Create the token object loaded from stdin
 * 
 * @return Loaded Token
 */
tToken get_token(){

    tToken token;
    static unsigned long init_count;
    int c = getchar();

    if (c == EOF){
        token.type = T_EOF;
        return token;
    }

    enum automat_state STATE = change_state(c);

    do{
        switch (STATE)
        {
        case S_START:
            STATE = change_state(c);
            if (STATE != S_START){
                ungetc(c, stdin);
            }
            break;
        case S_COLON:
            token.type = T_COLON;
            return token;
        case S_DOT:
            token.type = T_CONCATENATION;
            return token;
        case S_COMMA:
            token.type = T_COMMA;
            return token;
        case S_SEMICOLON:
            token.type = T_SEMICOLON;
            return token;
        case S_R_BRAC:
            token.type = T_R_BRAC;
            return token;
        case S_L_BRAC:
            token.type = T_L_BRAC;
            return token;
        case S_R_PAR:
            token.type = T_R_PAR;
            return token;
        case S_L_PAR:
            token.type = T_L_PAR;
            return token;
        case S_EQUAL:
            load_equal(&token, c, &init_count);
            if (c != '='){
                return token;
            }
            break;
        case S_GREATER_THAN:
            load_greater_than(&token, c, &init_count);
            if (c != '>' && c != '='){
                return token;
            }
            break;
        case S_LESS_THAN:
            load_less_than(&token, c, &init_count);
            if (c != '<' && c != '=' && c != '?' && c != 'p' && c != 'h'){
                return token;
            }
            break;
        case S_ASTERISK:
            token.type = T_MUL;
            return token;
        case S_SLASH:
            load_slash(&token, c, &init_count);
            /* IF COMMENT */
            if (token.data.STRINGval != NULL){
                if (((strcmp(token.data.STRINGval->data, "//")) == 0) && (c == '\n')) {
                    init_count = 0;
                    dynamicBufferFREE(token.data.STRINGval);
                    token.data.STRINGval = NULL;
                    token.type = T_ERROR; // fill token.type to something else than T_DIV
                }
                else if (((strcmp(token.data.STRINGval->data, "*/")) == 0)){
                    init_count = 0;
                    dynamicBufferFREE(token.data.STRINGval);
                    token.data.STRINGval = NULL;
                    token.type = T_ERROR; // fill token.type to something else than T_DIV
                }
            }
            /* RETURN DIV */
            if ((token.data.STRINGval == NULL) && (token.type == T_DIV)){
                return token;
            }
            /* RETURN COMMENT */
            else if ((token.data.STRINGval == NULL) && (token.type != T_DIV)){
                STATE = S_START;
            }
            break;
        case S_PLUS:
            token.type = T_ADD;
            return token;
        case S_MINUS:
            token.type = T_SUB;
            return token;
        case S_QUOTE:
            if ((init_count) && c == '"'){
                init_count = 0;
                return token;
            }
            load_string(&token, c, &init_count);
            break;
        case S_DOLLAR:
            if (init_count && c != '_' && (!isalpha(c)) && (!isdigit(c))){
                if (token.data.STRINGval->data[0] != '_' && (!isalpha(token.data.STRINGval->data[0]))){
                    dynamicBufferFREE(token.data.STRINGval);
                    token.type = T_ERROR;
                }
                init_count = 0;
                ungetc(c, stdin);
                return token;
            } 
            load_var_id(&token, c, &init_count);
            break;
        case S_QUESTIONER:
            load_questioner(&token, c, &init_count);
            if (!isalpha(c) && !isdigit(c) && (c != '?') && (c != '>')){
                return token;
            }
            break;
        case S_EXCLAMATION:
            load_exclamation(&token, c, &init_count);
            if ((c != '=') && (c != '!')){
                return token;
            }
            break;
        case S_LETTER:
            load_letter(&token, c, &init_count);
            if ((!isalpha(c)) && (!isdigit(c)) && (c != '_')){
                token.type = is_reserved_id(token.data.STRINGval);
                return token;
            }
            break;
        case S_NUM:
            load_num(&token, c, &init_count);
            if (isspace(c)){
                string_to_num(&token);
                return token;
            }
            break;
        case S_ERROR:
            token.type = T_ERROR;
            return token;
        default:
            token.type = T_UNKNOW;
            return token;
        }
    } while ((c = getchar()) != EOF);

    /* UNCOMPLETE BOLCK OF COMMENT */
    if (STATE == S_SLASH){
        if (token.data.STRINGval != NULL){
            if (((strcmp(token.data.STRINGval->data, "//")) == 0)){ // line comment and EOF
                token.type = T_EOF;
            }
            else{
                token.type = T_COMMENT_ERROR;
            }
            dynamicBufferFREE(token.data.STRINGval);
            return token;
        }
    }
    /* EOF */
    if (token.type == T_FUN_ID){ // CHECK FOR RESERVED IDs
        token.type =  is_reserved_id(token.data.STRINGval);
    }

    if (token.type == T_VAR_ID){
        if (token.data.STRINGval->data[0] != '_' && (!isalpha(token.data.STRINGval->data[0]))){
            dynamicBufferFREE(token.data.STRINGval);
            token.type = T_ERROR;
            return token;
        }
    }
    
    if (token.type == T_VAR_ID || token.type == T_FUN_ID){ // RETURN WITHOUT FREE BUFFER
        init_count = 0;
        return token;
    }
    else if (token.type == T_STRING){ // UNCMOPLETE STRING
        dynamicBufferFREE(token.data.STRINGval);
        init_count = 0;
        token.type = T_ERROR;
        return token;
    }
    else if (STATE == S_NUM){
        string_to_num(&token);
        init_count = 0;
        return token;
    }
    
    if (init_count){ // FREE AFTER EOF
        init_count = 0;
        dynamicBufferFREE(token.data.STRINGval);
    }
    /* COMPLETE BLOCK OF COMMENT AND EOF */
    else {
        token.type = T_EOF;
    }
    return token;
}



#ifdef TESTING

/**
 * @brief print token type
 * 
 * @param unum token_type member
 */
void print_token_type(enum token_type type){
    switch (type)
    {
    case T_STRING:
        printf(ANSI_COLOR_YELLOW "[T_STRING]" ANSI_COLOR_RESET "\n");
        break;
    case T_NUM_INT:
        printf(ANSI_COLOR_YELLOW "[T_NUM_INT]" ANSI_COLOR_RESET "\n");
        break;
    case T_NUM_FLOAT:
        printf(ANSI_COLOR_YELLOW "[T_NUM_FLOAT]" ANSI_COLOR_RESET "\n");
        break;
    case T_STRING_TYPE:
        printf(ANSI_COLOR_YELLOW "[T_STRING_TYPE]" ANSI_COLOR_RESET "\n");
        break;
    case T_FLOAT_TYPE:
        printf(ANSI_COLOR_YELLOW "[T_FLOAT_TYPE]" ANSI_COLOR_RESET "\n");
        break;
    case T_INT_TYPE:
        printf(ANSI_COLOR_YELLOW "[T_INT_TYPE]" ANSI_COLOR_RESET "\n");
        break;
    case T_VAR_ID:
        printf(ANSI_COLOR_YELLOW "[T_VAR_ID]" ANSI_COLOR_RESET "\n");
        break;
    case T_FUN_ID:
        printf(ANSI_COLOR_YELLOW "[T_FUN_ID]" ANSI_COLOR_RESET "\n");
        break;
    case T_COLON:
        printf(ANSI_COLOR_YELLOW "[T_COLON]" ANSI_COLOR_RESET "\n");
        break;
    case T_CONCATENATION:
        printf(ANSI_COLOR_YELLOW "[T_CONCATENATION]" ANSI_COLOR_RESET "\n");
        break;
    case T_COMMA:
        printf(ANSI_COLOR_YELLOW "[T_COMMA]" ANSI_COLOR_RESET "\n");
        break;
    case T_SEMICOLON:
        printf(ANSI_COLOR_YELLOW "[T_SEMICOLON]" ANSI_COLOR_RESET "\n");
        break;
    case T_R_BRAC:
        printf(ANSI_COLOR_YELLOW "[T_R_BRAC]" ANSI_COLOR_RESET "\n");
        break;
    case T_L_BRAC:
        printf(ANSI_COLOR_YELLOW "[T_L_BRAC]" ANSI_COLOR_RESET "\n");
        break;
    case T_R_PAR:
        printf(ANSI_COLOR_YELLOW "[T_R_PAR]" ANSI_COLOR_RESET "\n");
        break;
    case T_L_PAR:
        printf(ANSI_COLOR_YELLOW "[T_L_PAR]" ANSI_COLOR_RESET "\n");
        break;
    case T_ASSIGN:
        printf(ANSI_COLOR_YELLOW "[T_ASSIGN]" ANSI_COLOR_RESET "\n");
        break;
    case T_EQUALS:
        printf(ANSI_COLOR_YELLOW "[T_EQUALS]" ANSI_COLOR_RESET "\n");
        break;
    case T_NOT_EQUALS:
        printf(ANSI_COLOR_YELLOW "[T_NOT_EQUALS]" ANSI_COLOR_RESET "\n");
        break;
    case T_GREATER_THAN:
        printf(ANSI_COLOR_YELLOW "[T_GREATER_THAN]" ANSI_COLOR_RESET "\n");
        break;
    case T_GREATER_OR_EQUAL:
        printf(ANSI_COLOR_YELLOW "[T_GREATER_OR_EQUAL]" ANSI_COLOR_RESET "\n");
        break;
    case T_LESS_THAN:
        printf(ANSI_COLOR_YELLOW "[T_LESS_THAN]" ANSI_COLOR_RESET "\n");
        break;
    case T_LESS_OR_EQUAL:
        printf(ANSI_COLOR_YELLOW "[T_LESS_OR_EQUAL]" ANSI_COLOR_RESET "\n");
        break;
    case T_MUL:
        printf(ANSI_COLOR_YELLOW "[T_MUL]" ANSI_COLOR_RESET "\n");
        break;
    case T_DIV:
        printf(ANSI_COLOR_YELLOW "[T_DIV]" ANSI_COLOR_RESET "\n");
        break;
    case T_ADD:
        printf(ANSI_COLOR_YELLOW "[T_ADD]" ANSI_COLOR_RESET "\n");
        break;
    case T_SUB:
        printf(ANSI_COLOR_YELLOW "[T_SUB]" ANSI_COLOR_RESET "\n");
        break;
    case T_PROLOG_START:
        printf(ANSI_COLOR_YELLOW "[T_PROLOG_START]" ANSI_COLOR_RESET "\n");
        break;
    case T_PROLOG_END:
        printf(ANSI_COLOR_YELLOW "[T_PROLOG_END]" ANSI_COLOR_RESET "\n");
        break;
    case T_WHILE:
        printf(ANSI_COLOR_YELLOW "[T_WHILE]" ANSI_COLOR_RESET "\n");
        break;
    case T_IF:
        printf(ANSI_COLOR_YELLOW "[T_IF]" ANSI_COLOR_RESET "\n");
        break;
    case T_ELSE:
        printf(ANSI_COLOR_YELLOW "[T_ELSE]" ANSI_COLOR_RESET "\n");
        break;
    case T_FUNCTION:
        printf(ANSI_COLOR_YELLOW "[T_FUNCTION]" ANSI_COLOR_RESET "\n");
        break;
    case T_RETURN:
        printf(ANSI_COLOR_YELLOW "[T_RETURN]" ANSI_COLOR_RESET "\n");
        break;
    case T_VOID:
        printf(ANSI_COLOR_YELLOW "[T_VOID]" ANSI_COLOR_RESET "\n");
        break;
    case T_EOF:
        printf(ANSI_COLOR_YELLOW "[T_EOF]" ANSI_COLOR_RESET "\n");
        break;
    case T_UNKNOW:
        printf(ANSI_COLOR_YELLOW "[T_UNKNOW]" ANSI_COLOR_RESET "\n");
        break;
    case T_COMMENT_ERROR:
        printf(ANSI_COLOR_RED "[T_COMMENT_ERROR]" ANSI_COLOR_RESET "\n");
        break;
    case T_ERROR:
        printf(ANSI_COLOR_RED "[T_ERROR]" ANSI_COLOR_RESET "\n");
        break;
    default:
        break;
    }
}

int main(){

    tToken token;
    while ((token = get_token()).type != T_EOF)
    {
        if (token.type == T_COMMENT_ERROR){
            print_token_type(token.type);
            /* ! there is incomplete bolck of comment ! */
            break;
        }
        print_token_type(token.type);

        if (token.type == T_STRING || token.type == T_VAR_ID || token.type == T_FUN_ID){
            printf("TOKEN VALUE: %s\n", token.data.STRINGval->data);
        }
        else if (token.type == T_NUM_INT){
            printf("TOKEN VALUE: %ld\n", token.data.INTval);
        }
        else if (token.type == T_NUM_FLOAT){
            printf("TOKEN VALUE: %f\n", token.data.FLOATval);
        }
    }
    return 0;
}

#endif // TESTING