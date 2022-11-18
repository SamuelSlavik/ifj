/**
 * @file parser.c
 * @brief Implementation of parser
 * @version 0.01
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "scanner.h"
bool fstart(tToken *);
bool fprog(tToken *);

//<start> =>  [T_PROLOG] <prog>
bool fstart(tToken *token){
    bool start = false;
    if (token->type == T_PROLOG) {
        *token = get_token(1);
        start = fprog(token);
    }
    return start;
}

bool fprog(tToken *token){
    bool prog = false;
    if (token->type == T_EOF){
        prog = true;
        return prog;
    }
    else if (token->type == T_EPILOG){
        *token = get_token(2);
        if (token->type == T_EOF){
        prog = true;
        return prog;
        }
    }
    // TODO func
    prog = fbody(token) && fprog(token); //<prog> => <body> <prog>
    return prog;
}
bool fbody(tToken *token){
    bool body = false;
    switch (token->type)
    {
    case T_VAR_ID:
        *token = get_token(1);
        body = fbody_as(token);
        break;
    case T_FUN_ID:
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            
            body = fn_call_l(token);
            if (token->type != T_SEMICOLON){
                body = false;
            }
            *token = get_token(1);
        }
        break;
    case T_RETURN:
        *token = get_token(1);
        body = fbody_ret(token);
    break;
    case T_IF:
        *token = get_token(1);
        //TODO
    break;
    case T_WHILE:
        *token = get_token(1);
        //TODO
    break;
    //todo expression
    default:
        break;
    }
    return body;
}

bool fbody_as(tToken *token){
    bool body_as = false;
    if (token->type == T_ASSIGN){
        *token = get_token(1);
        body_as = fbody_var(token);
        
    }
    if (token->type == T_SEMICOLON){
        body_as = true;
    }
    return body_as;
}

bool fbody_var(tToken *token){
    bool body_var;
    if (token->type == T_NUM_INT 
        || token->type == T_NUM_FLOAT 
        || token->type == T_STRING) {
            *token = get_token(1);
            if (token->type == T_SEMICOLON){
                body_var = true;
            }
    }
    if (token->type == T_FUN_ID){
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            body_var = fn_call_l(token);
            *token = get_token(1);
            if (token->type != T_SEMICOLON){
                body_var = false;
            }
        }
    }
    //TODO expression
    return body_var;
}

bool fbody_ret(tToken *token){
    bool body_ret = false;
    switch (token->type)
    {
    case T_STRING:
        *token = get_token(1);
        if (token->type == T_SEMICOLON){
            body_ret = true;
        }
        break;
    case T_NUM_INT:
        *token = get_token(1);
        if (token->type == T_SEMICOLON){
            body_ret = true;
        }
        break;
    case T_NUM_FLOAT:
        *token = get_token(1);
        if (token->type == T_SEMICOLON){
            body_ret = true;
        }
        break;
    
    case T_VAR_ID:
        *token = get_token(1);
        if (token->type == T_SEMICOLON){
            body_ret = true;
        }
        break;
    case T_SEMICOLON:
        body_ret = true;
        break;
    //TODO EXPRESSION
    default:
        break;
    }
    return body_ret;
    
}

bool fn_call_l(tToken *token){
    bool fn_call_bool = false;
    switch (token->type)
    {
    case T_STRING:
    case T_NUM_INT:
    case T_NUM_FLOAT:
    case T_VAR_ID:
        *token = get_token(1);
        fn_call_bool = fn_call_l(token);
        break;
    case T_R_PAR:
        *token = get_token(1);
        fn_call_bool = true;
        break;
    default:
        break;
    }
    return fn_call_bool;
}
int main(){
    tToken token = get_token(0);
    if (fstart(&token)){
        printf("PRINTF MAIN OK\n");
    }
    else{
        printf("PRINTF MAIN ERROR\n");
    }
}
