/**
 * @file parser.c
 * @brief Implementation of parser
 * @version 0.02
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "scanner.h"


//<start> =>  [T_PROLOG] <prog>
bool f_start(tToken *token){
    bool start = false;
    if (token->type == T_PROLOG) {
        *token = get_token(1);
        start = f_prog(token);
    }
    return start;
}

bool f_prog(tToken *token){
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
    prog = (f_func(token) && f_prog(token)) || (f_body(token) && f_prog(token)); //<prog> => <fn> <prog>
    //<prog> => <body> <prog>
    return prog;
}
bool f_body(tToken *token){
    bool body = false;
    switch (token->type)
    {
    case T_VAR_ID:
        *token = get_token(1);
        body = f_body_as(token);
        break;
    case T_FUN_ID:
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            
            body = f_fn_call_l(token);
            if (token->type != T_SEMICOLON){
                body = false;
            }
            *token = get_token(1);
        }
        break;
    case T_RETURN:
        *token = get_token(1);
        body = f_body_ret(token);
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

bool f_body_as(tToken *token){
    bool body_as = false;
    if (token->type == T_ASSIGN){
        *token = get_token(1);
        body_as = f_body_var(token);
        
    }
    else if (token->type == T_SEMICOLON){
        *token = get_token(1);
        body_as = true;
    }
    return body_as;
}

bool f_body_var(tToken *token){
    bool body_var = false;
    switch (token->type)
    {
    case T_NUM_INT:
    case T_NUM_FLOAT:
    case T_STRING:
        *token = get_token(1);
        if (token->type == T_SEMICOLON){
            *token = get_token(1);
            body_var = true;
        }
        break;
    case T_FUN_ID:
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            body_var = f_fn_call_l(token);
            if (token->type != T_SEMICOLON){
                body_var = false;
            }
            *token = get_token(1);
        }
        break;
    //TODO expression
    default:
        break;
    }
    return body_var;
}

bool f_body_ret(tToken *token){
    bool body_ret = false;
    switch (token->type)
    {
    case T_STRING:
    case T_NUM_INT:
    case T_NUM_FLOAT:
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

bool f_fn_call_l(tToken *token){
    bool fn_call_bool = false;
    switch (token->type)
    {
    case T_STRING:
    case T_NUM_INT:
    case T_NUM_FLOAT:
    case T_VAR_ID:
        *token = get_token(1);
        fn_call_bool = f_fn_call_l(token);
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

bool f_func(tToken *token){
    bool func = false;
    if(token->type == T_FUNCTION){
        *token = get_token(1);
        if(token->type == T_FUN_ID){
            *token = get_token(1);
            if (token->type == T_L_PAR){
                *token = get_token(1);
                func = f_func_param(token);
                if(token->type == T_COLON && func != false){
                    *token = get_token(1);
                    func = f_func_type(token) && f_func_dedf(token); 
                }
                //TODO SKONTROLOVAT MAGIA
            }
        }
    }
    return func;
}
bool f_func_dedf(tToken *token){
    bool func_dedf = false;
    if(token->type == T_L_BRAC){
        *token = get_token(1);
        func_dedf = f_in_body(token);
    }
    else if (token->type == T_SEMICOLON){
        *token = get_token(1);
        func_dedf = true;
    }
    return func_dedf;
}

bool f_func_type(tToken *token){
    bool func_type = false;
    switch (token->type)
    {
    case T_VOID:
    case T_STRING_TYPE:
    case T_FLOAT_TYPE:
    case T_INT_TYPE:
    case T_STRING_N_TYPE:
    case T_FLOAT_N_TYPE:
    case T_INT_N_TYPE:
        *token = get_token(1);
        func_type = true;
        break;
    default:
        break;
    }
    return func_type;
}

bool f_in_body(tToken *token){
    bool in_body = false;
    if(token->type== T_R_BRAC){
        *token = get_token(1);
        in_body = true;
        return in_body;
    }
    in_body = f_body(token) && f_in_body(token);
    return in_body;
}

bool f_func_param(tToken *token){
    bool func_param = false;
    switch (token->type)
    {
    case T_STRING:
    case T_NUM_INT:
    case T_NUM_FLOAT:
        *token = get_token(1);
        func_param = f_func_mparam(token);
        break;
    case T_STRING_TYPE:
    case T_FLOAT_TYPE:
    case T_INT_TYPE:
    case T_STRING_N_TYPE:
    case T_FLOAT_N_TYPE:
    case T_INT_N_TYPE:
        *token = get_token(1);
        if(token->type == T_VAR_ID){
            *token = get_token(1);
            func_param = f_func_mparam(token);
        }
        break;
    case T_R_PAR:
        *token = get_token(1);
        func_param = true;
    default:
        break;
    }
    return func_param;
}

bool f_func_mparam(tToken * token){
    bool func_mparam = false;
    switch (token->type)
    {
    case T_STRING:
    case T_NUM_INT:
    case T_NUM_FLOAT:
        *token = get_token(1);
        func_mparam = f_func_mparam(token);
        break;
    case T_STRING_TYPE:
    case T_FLOAT_TYPE:
    case T_INT_TYPE:
    case T_STRING_N_TYPE:
    case T_FLOAT_N_TYPE:
    case T_INT_N_TYPE:
        *token = get_token(1);
        if(token->type == T_VAR_ID){
            *token = get_token(1);
            func_mparam = f_func_mparam(token);
        }
        break;
    case T_COMMA:
        *token = get_token(1);
        func_mparam = f_func_mparam(token);
        break;
    case T_R_PAR:
        *token = get_token(1);
        func_mparam = true;
    default:
        break;
    }
    return func_mparam;
}

int main(){
    tToken token = get_token(0);
    if (f_start(&token)){
        printf("PRINTF MAIN OK\n");
    }
    else{
        printf("PRINTF MAIN ERROR\n");
    }
}
