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
#include "error.h"


enum automat_state change_state(char c){
    
    if (isalpha(c) || (c == '_')){
        return S_LETTER;
    }
    else if (isdigit(c)){
        return S_INT_NUM;
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
    case '}':
        return S_R_BRAC;
        break;
    case '{':
        return S_L_BRAC;
        break;
    case ')':
        return S_R_PAR;
        break;
    case '(':
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


void load_string(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            error_handle();
            return;
        }
        *init_count = 1;
    }
    /* PROCESS skip first '"' */
    if (c != '"' || token->data.STRINGval->size > 0){
        bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            error_handle();
            return;
        }
    }
    token->type = T_ERROR;
}


void load_var_id(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            error_handle();
            return;
        }
        *init_count = 1;
    }
    /* PROCESS */
    bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
    if (err == false){
        error_handle();
        return;
    }
    token->type = T_VAR_ID;
}


void load_type_id(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            error_handle();
            return;
        }
        *init_count = 1;
    }
    /* PROCESS */
    bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
    if (err == false){
        error_handle();
        return;
    }
    /* SET TOKEN TYPE */
    if (((strcmp(token->data.STRINGval->data, "string")) == 0)){
        token->type = T_STRING_N_TYPE;
    }
    else if (((strcmp(token->data.STRINGval->data, "int")) == 0)){
        token->type = T_INT_N_TYPE;
    }
    else if (((strcmp(token->data.STRINGval->data, "float")) == 0)){
        token->type = T_FLOAT_N_TYPE;
    }
    else{
        token->type = T_ERROR;
    }
}


void load_num(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            error_handle();
            return;
        }
        *init_count = 1;
    }
    /* PROCESS */
    bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
    if (err == false){
        error_handle();
        return;
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

    /* check for missing decimal */
    if (token->data.STRINGval->data[token->data.STRINGval->size - 1] != '.'){

        tmp_double = strtod(token->data.STRINGval->data, &err);
    
        if (*err == '\0'){
            token->type = T_NUM_FLOAT;
            dynamicBufferFREE(token->data.STRINGval);
            token->data.FLOATval = tmp_double;
            return;
        }
    }

    /* ELSE */
    dynamicBufferFREE(token->data.STRINGval);
    token->type = T_ERROR;
}


enum token_type is_reserved_id(tDynamicBuffer* string){

    char *reserved_words[] =  {"string", "int", "float", "while", "if", "else", "function", "return", "void", "null"};
    enum token_type types[] = {T_STRING_TYPE, T_INT_TYPE, T_FLOAT_TYPE, T_WHILE, T_IF, T_ELSE, T_FUNCTION, T_RETURN, T_VOID, T_NULL};

    /* Loop through reserved words array which have 10 members */
    for (size_t i = 0; i < 10; i++){
        if ((strcmp(string->data, reserved_words[i])) == 0){
            return types[i];
        }
    }
    
    return T_FUN_ID;
}


void load_letter(tToken* token, char c, unsigned long *init_count){
    /* INIT BUFFER*/
    if (!(*init_count)){
        token->data.STRINGval = dynamicBuffer_INIT();
        if (token->data.STRINGval == NULL){
            error_handle();
            return;
        }
        *init_count = 1;
    }
    /* PROCESS */
    bool err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
    if (err == false){
        error_handle();
        return;
    }
}


tToken automat_state_prolog(tToken *token, char c, unsigned long *line_count){

    bool err = true;
    token->type = T_ERROR;

    /* initialize line count */
    *line_count = 1;

    /* initialize buffer */
    token->data.STRINGval = dynamicBuffer_INIT();
    if (token->data.STRINGval == NULL){
        error_handle();
    }

    /* LOAD FIRST PART <?php */
    for (size_t i = 0; i < strlen("<?php"); i++){
        err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
        if (err == false){
            error_handle();
        }
        c = getchar();
        if (c == EOF){
            return *token;
        }
    }

    /* Check first part of prolog */
    if ((strcmp(token->data.STRINGval->data, "<?php")) != 0){
        return *token;
    }

    /* LOAD SECOND PART declare(strict_types=1); */
    enum automat_state STATE = change_state(c);
    do
    {
        switch (STATE)
        {
        case S_START:
            if (c == '\n'){
                (*line_count)++;
            }
            STATE = change_state(c);
            if (STATE != S_START){
                ungetc(c, stdin);
            }
            else{
                err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
                if (err == false){
                    error_handle();
                }
            }
            break;
        case S_LETTER:
        case S_INT_NUM:
        case S_EQUAL:
        case S_R_PAR:
        case S_L_PAR:
            /* Add symbol */
            err = dynamicBuffer_ADD_CHAR(token->data.STRINGval, c);
            if (err == false){
                error_handle();
            }
            STATE = S_START;
            break;
        case S_SEMICOLON:
            /* Check if correct */
            if (isspace(token->data.STRINGval->data[5])){
                /* 29 is leng of prolog without white spaces + 1 */
                char tmp[29] = {'\0'};
                for (unsigned long i = 0, k = 0; i < token->data.STRINGval->size; i++){
                    if (!isspace(token->data.STRINGval->data[i]) && k < 28){
                        tmp[k] = token->data.STRINGval->data[i];
                        k++;
                    }
                    else{
                        if ((strcmp(tmp, "<?php")) && (strcmp(tmp, "<?phpdeclare")) && (strcmp(tmp, "<?phpdeclare(")) &&
                            (strcmp(tmp, "<?phpdeclare(strict_types")) && (strcmp(tmp, "<?phpdeclare(strict_types=")) &&
                            (strcmp(tmp, "<?phpdeclare(strict_types=1")) && (strcmp(tmp, "<?phpdeclare(strict_types=1)"))){
                                return *token;
                            }
                    }
                }
                if (!strcmp(tmp, "<?phpdeclare(strict_types=1)")){
                    token->type = T_PROLOG;
                }
            }
            return *token;
        case S_SLASH:
            STATE = S_COMMENT;
            break;
        case S_COMMENT:
            if (c == '*'){
                STATE = S_BLOCK_COMMENT;
            }
            else if (c == '/')
            {
                STATE = S_LINE_COMMENT;
            }
            else{
                return *token;
            }
            break;
        case S_BLOCK_COMMENT:
            if (c == '*'){
                STATE = S_B_C_END;
            }
            else if (c == '\n'){
                (*line_count)++;
            }
            break;
        case S_B_C_END:
            if (c == '/'){
                STATE = S_START;
            }
            else{
                if (c == '\n'){
                    (*line_count)++;
                }
                if (c != '*'){
                    STATE = S_BLOCK_COMMENT;
                }
            }
            break;
        case S_LINE_COMMENT:
            if (c == '\n'){
                (*line_count)++;
                ungetc(c, stdin);
                STATE = S_START;
            }
            break;
        default:
            return *token;
            break;
        }
    } while ((c = getchar()) != EOF);

    return *token;
}


tToken automat_state_epilog(tToken *token, char c, unsigned long *line_count){

    token->type = T_EOF;

    // One \n is accepted
    if (c != '\n'){
        token->type = T_ERROR;
    }

    *line_count += 1;
    token->line = *line_count;

    c = getchar();
    if (c != EOF){
        token->type = T_ERROR;
    }

    return *token;
}


tToken get_token(short automat_state){

    tToken token;
    token.type = T_UNKNOW;
    bool escapeSeq = false;
    static unsigned long init_count;
    static unsigned long line_count;
    token.line = line_count;
    int c = getchar();

    if (c == EOF){
        token.type = T_EOF;
        return token;
    }

    /* AUTOMAT STATE 2 => CHECKING CORRECT END */

    if (automat_state == 2){
        return automat_state_epilog(&token, c, &line_count);
    }

    enum automat_state STATE = change_state(c);


    /* AUTOMAT STATE 0 => WAITING ON PROLOG */

    if (automat_state == 0){
        token = automat_state_prolog(&token, c, &line_count);
        dynamicBufferFREE(token.data.STRINGval);
        token.line = line_count;
        return token;
    }

    /* AUTOMAT STATE 1 => PROCESS PROGRAM */

    do{
        switch (STATE)
        {
        case S_START:
            if (c == '\n'){
                line_count++;
                token.line = line_count;
            }
            STATE = change_state(c);
            if (STATE != S_START){
                ungetc(c, stdin);
            }
            break;
        /* : */
        case S_COLON:
            token.type = T_COLON;
            return token;
        /* . */
        case S_DOT:
            token.type = T_CONCATENATION;
            return token;
        /* , */
        case S_COMMA:
            token.type = T_COMMA;
            return token;
        /* ; */
        case S_SEMICOLON:
            token.type = T_SEMICOLON;
            return token;
        /* } */
        case S_R_BRAC:
            token.type = T_R_BRAC;
            return token;
        /* { */
        case S_L_BRAC:
            token.type = T_L_BRAC;
            return token;
        /* ) */
        case S_R_PAR:
            token.type = T_R_PAR;
            return token;
        /* ( */
        case S_L_PAR:
            token.type = T_L_PAR;
            return token;
        /* = */
        case S_EQUAL:
            token.type = T_ASSIGN;
            STATE = S_EQEQ;
            break;
        /* == */
        case S_EQEQ:
            if (c == '='){
                STATE = S_ASSIGN;
                token.type = T_ERROR;
            }
            else{
                ungetc(c, stdin);
                return token;
            }
            break;
        /* === */
        case S_ASSIGN:
            if (c == '='){
                token.type = T_EQUALS;
            }
            else{
                token.type = T_ERROR;
                ungetc(c, stdin);
            }
            return token;
        /* > */
        case S_GREATER_THAN:
            token.type = T_GREATER_THAN;
            STATE = S_G_T_;
            break;
        /* >[?] */
        case S_G_T_:
            if (c == '='){
                token.type = T_GREATER_OR_EQUAL;
            }
            else{
                ungetc(c, stdin);
            }
            return token;
        /* < */
        case S_LESS_THAN:
            token.type = T_LESS_THAN;
            STATE = S_L_T_;
            break;
        /* <[?] */
        case S_L_T_:
            if (c == '='){
                token.type = T_LESS_OR_EQUAL;
            }
            else{
                ungetc(c, stdin);
            }
            return token;
        /* * */
        case S_ASTERISK:
            token.type = T_MUL;
            return token;
        /* / */
        case S_SLASH:
            token.type = T_DIV;
            STATE = S_COMMENT;
            break;
        /* /[?] */
        case S_COMMENT:
            if (c == '/'){
                STATE = S_LINE_COMMENT;
            }
            else if (c == '*'){
                STATE = S_BLOCK_COMMENT;
            }
            else{
                ungetc(c, stdin);
                return token;
            }
            break;
        /* // */
        case S_LINE_COMMENT:
            if (c == '\n'){
                token.type = T_UNKNOW;
                line_count++;
                token.line = line_count;
                STATE = S_START;
            }
            break;
        /* BLOCK COMMENT */            
        case S_BLOCK_COMMENT:
            token.type = T_ERROR;
            if (c == '*'){
                STATE = S_B_C_END;
            }
            else if(c == '\n'){
                line_count++;
                token.line = line_count;
            }
            break;
        /* BLOCK COMMENT END */
        case S_B_C_END:
            if (c == '/'){
                token.type = T_UNKNOW;
                STATE = S_START;
            }
            else {
                if (c == '\n'){
                    line_count++;
                    token.line = line_count;
                }
                if (c != '*'){
                    STATE = S_BLOCK_COMMENT;
                }
            }
            break;
        /* + */
        case S_PLUS:
            token.type = T_ADD;
            return token;
        /* - */
        case S_MINUS:
            token.type = T_SUB;
            return token;
        /* "..." */
        case S_QUOTE: 
            if (c == '$' && !escapeSeq){
                dynamicBufferFREE(token.data.STRINGval);
                init_count = 0;
                return token;
            }
            if (c == '\\' && !escapeSeq){
                escapeSeq = true;
            }
            else if (c != '"'){
                escapeSeq = false;
            }
            if ((init_count) && c == '"' && !escapeSeq){
                /* remove last '"' */
                token.data.STRINGval->data[token.data.STRINGval->size] = '\0';
                init_count = 0;
                token.type = T_STRING;
                return token;
            }
            if (c == '"' && escapeSeq){
                escapeSeq = false;
            }
            load_string(&token, c, &init_count);     
            break;
        /* $ */
        case S_DOLLAR:
            if ((c == '_') || (isalpha(c))){
                init_count = 0;
                load_var_id(&token, c, &init_count);
                STATE = S_V_ID;
            }
            else if ((!init_count) && (c == '$')){ // skip first $
                init_count = 1;
                token.type = T_ERROR;
                STATE = S_DOLLAR;
            }
            else {
                token.type = T_ERROR;
                ungetc(c, stdin);
                init_count = 0;
                return token;
            }
            break;
        /* $_a-zA-Z  */
        case S_V_ID:
            if((c == '_') || (isalpha(c)) || (isdigit(c))){
                load_var_id(&token, c, &init_count);
            }
            else {
                init_count = 0;
                ungetc(c, stdin);
                return token;
            }
            break;
        /* ? */
        case S_QUESTIONER:
            if (c == '>'){
                init_count = 0;
                token.type = T_EPILOG;
                return token;
            }
            else if (isalpha(c)){
                ungetc(c, stdin);
                init_count = 0;
                STATE = S_TYPE_ID;
            }
            else if ((init_count != 2) && (c == '?')){
                init_count = 2;
            }
            else{
                init_count = 0;
                ungetc(c, stdin);
                token.type = T_ERROR;
                return token;
            }
            break;
        /* ?int | ?string | ?float */
        case S_TYPE_ID:
            if (isalpha(c)){
                load_type_id(&token, c, &init_count);
            }
            else{
                dynamicBufferFREE(token.data.STRINGval);
                ungetc(c, stdin);
                init_count = 0;
                return token;
            }
            break;
        /* ! */
        case S_EXCLAMATION:
            if ((init_count == 0) && (c == '!')){       // !
                init_count = 2;
                token.type = T_ERROR; 
            }
            else if ((init_count == 2) && (c == '=')){  // !=
                init_count = 3;
                token.type = T_ERROR;
            }
            else if ((init_count == 3) && (c == '=')){  // !==
                init_count = 0;
                token.type = T_NOT_EQUALS;
                return token;
            }
            else {
                init_count = 0;
                token.type = T_ERROR;
                return token;
            }
            break;
        /* _a-zA-Z */
        case S_LETTER:
            if ((!isalpha(c)) && (!isdigit(c)) && (c != '_')){
                /* check for reserved words */
                token.type = is_reserved_id(token.data.STRINGval);
                ungetc(c, stdin);
                init_count = 0;
                return token;
            }
            token.type = T_FUN_ID;
            load_letter(&token, c, &init_count);
            break;
        /* 0-9 */
        case S_INT_NUM:
            if (isdigit(c)){
                load_num(&token, c, &init_count);
            }
            else if (c == '.'){
                load_num(&token, c, &init_count);
                STATE = S_FLOAT_NUM;
            }
            else if ((c == 'e') || (c == 'E')){
                load_num(&token, c, &init_count);
                STATE = S_EXP_NUM;
            }
            else {
                string_to_num(&token);
                ungetc(c, stdin);
                init_count = 0;
                return token;
            }
            break;
        case S_FLOAT_NUM:
            if (isdigit(c)){
                load_num(&token, c, &init_count);
            }
            else if ((c == 'e') || (c == 'E')){
                load_num(&token, c, &init_count);
                STATE = S_EXP_NUM;
            }
            else {
                string_to_num(&token);
                ungetc(c, stdin);
                init_count = 0;
                return token;
            }
            break;
        case S_EXP_NUM:
            if (isdigit(c)){
                load_num(&token, c, &init_count);
                STATE = S_FLOAT_NUM;
            }
            else if ((c == '+') || (c == '-')){
                load_num(&token, c, &init_count);
                STATE = S_FLOAT_NUM;
            }
            else {
                init_count = 0;
                token.type = T_ERROR;
                dynamicBufferFREE(token.data.STRINGval);
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

    /* EOF */
    
    if ((STATE == S_INT_NUM) || (STATE == S_FLOAT_NUM) || (STATE == S_EXP_NUM)){
        string_to_num(&token);
        init_count = 0;
    }

    if (STATE == S_LETTER){
        /* check for reserved words */
        token.type = is_reserved_id(token.data.STRINGval);
    }
    
    if ((init_count == 1) && 
        (token.type != T_VAR_ID) && 
        (token.type != T_FUN_ID)){ // FREE AFTER EOF
        dynamicBufferFREE(token.data.STRINGval);
    }

    if ((token.type == T_UNKNOW) || (STATE == S_LINE_COMMENT)){
        token.type = T_EOF;
    }
    /* Uncomplete block comment */
    else if ((STATE == S_BLOCK_COMMENT) || (STATE == S_B_C_END)){
        token.type = T_ERROR;
    }

    init_count = 0;
    return token;
}



/************************************************/
/*********** MAIN FOR TESTING PURPOSES **********/
/************************************************/

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
    case T_NULL:
        printf(ANSI_COLOR_YELLOW "[T_NULL]" ANSI_COLOR_RESET "\n");
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
    case T_STRING_N_TYPE:
        printf(ANSI_COLOR_YELLOW "[T_STRING_N_TYPE]" ANSI_COLOR_RESET "\n");
        break;
    case T_FLOAT_N_TYPE:
        printf(ANSI_COLOR_YELLOW "[T_FLOAT_N_TYPE]" ANSI_COLOR_RESET "\n");
        break;
    case T_INT_N_TYPE:
        printf(ANSI_COLOR_YELLOW "[T_INT_N_TYPE]" ANSI_COLOR_RESET "\n");
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
    case T_PROLOG:
        printf(ANSI_COLOR_YELLOW "[T_PROLOG]" ANSI_COLOR_RESET "\n");
        break;
    case T_EPILOG:
        printf(ANSI_COLOR_YELLOW "[T_EPILOG]" ANSI_COLOR_RESET "\n");
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
    case T_ERROR:
        printf(ANSI_COLOR_RED "[T_ERROR]" ANSI_COLOR_RESET "\n");
        break;
    default:
        break;
    }
}

int main(){

    tToken token;

    token = get_token(0);
    print_token_type(token.type);
    printf("line: %ld\n", token.line);


    while ((token = get_token(1)).type != T_EOF)
    {

        if (token.type == T_EPILOG){
            print_token_type(token.type);
            printf("line: %ld\n", token.line);
            break;
        }
        print_token_type(token.type);
        printf("line: %ld\n", token.line);

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

    while ((token = get_token(2)).type != T_EOF){
        print_token_type(token.type);
        printf("line: %ld\n", token.line);
    }
    print_token_type(token.type);
    printf("line: %ld\n", token.line);


    return 0;
}

#endif // TESTING