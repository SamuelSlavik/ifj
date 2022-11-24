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
#include "expression_parser.h"
#include "dynamic_buffer.h"
#include "dll_instruction_list.h"
#include "htab.h"

extern htab_t *symtable;


//<start> =>  [T_PROLOG] <prog>
bool f_start(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
    bool start = false;
    if (token->type == T_PROLOG) {
        *token = get_token(1);
        start = f_prog(token,instruction, instruction_list);
    }
    return start;
}

bool f_prog(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
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
    prog = (f_func(token,instruction, instruction_list) && f_prog(token,instruction, instruction_list)) || (f_body(token,instruction, instruction_list) && f_prog(token,instruction, instruction_list)); //<prog> => <fn> <prog>
    //<prog> => <body> <prog>
    return prog;
}
bool f_body(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
    bool body = false;
    tToken end_token;
    switch (token->type)
    {
    case T_VAR_ID:
        if (htab_find(symtable,token->data.STRINGval->data) == NULL ){
            dynamicBuffer_ADD_STRING(instruction, "DEFVAR ");
            dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
            DLL_InsertAfter(instruction_list,instruction);
        }
        instruction_list->first->curr_var=st_var_create(symtable, token->data.STRINGval->data);
        *token = get_token(1);
        printf("%s\n", instruction_list->first->curr_var->key);
        body = f_body_as(token,instruction, instruction_list);
        /*
        if (token->type == T_ASSIGN){
            body = f_body_as(token,instruction, instruction_list);
            dynamicBuffer_ADD_STRING(instruction, "POPS ");
            dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
            DLL_InsertAfter(instruction_list,instruction);
        }
        else{
            end_token.type = T_SEMICOLON;
            body = check_expr_syntax(token, &end_token);
        }*/
        break;
    case T_FUN_ID:
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            
            body = f_fn_call_l(token,instruction, instruction_list);
            if (token->type != T_SEMICOLON){
                body = false;
            }
            *token = get_token(1);
        }
        break;
    case T_RETURN:
        *token = get_token(1);
        body = f_body_ret(token,instruction, instruction_list);
    break;
    case T_IF:
        *token = get_token(1);
        if(token->type == T_L_PAR){
            *token = get_token(1);
            end_token.type = T_R_PAR;
            body = check_expr_syntax(token, &end_token);
            if (body == false) return body; //znova magia skontrolovat
            *token = get_token(1);
            if (token->type == T_L_BRAC){
                *token = get_token(1);
                body = f_in_body(token,instruction, instruction_list);
                if (body == false) return body;
                print_token_type(token->type);
                if (token->type == T_ELSE){
                    *token = get_token(1);
                    if(token->type == T_L_BRAC){
                        *token = get_token(1);
                        body = f_in_body(token,instruction, instruction_list);
                    }
                }
                else{
                    body = false;
                }
            }
        }
    break;
    case T_WHILE:
        *token = get_token(1);
        if(token->type == T_L_PAR){
            *token = get_token(1);
            end_token.type = T_R_PAR;
            body = check_expr_syntax(token, &end_token);
            if (body == false) return body; //znova magia skontrolovat
            *token = get_token(1);
            if (token->type == T_L_BRAC){
                *token = get_token(1);
                body = f_in_body(token,instruction, instruction_list);
            }
        }
    break;
    default:
        end_token.type = T_SEMICOLON;
        body = check_expr_syntax(token, &end_token);
        *token = get_token(1);
        break;
    }
    return body;
}

bool f_body_as(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
    bool body_as = false;
    if (token->type == T_ASSIGN){
        //add insctruction mov
        *token = get_token(1);
        body_as = f_body_var(token,instruction, instruction_list);
        
    }
    else if (token->type == T_SEMICOLON){
        *token = get_token(1);
        body_as = true;
    }
    return body_as;
}

bool f_body_var(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool body_var = false;
    if(token->type == T_FUN_ID){
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            body_var = f_fn_call_l(token,instruction, instruction_list);
            if (token->type != T_SEMICOLON){
                body_var = false;
            }
            *token = get_token(1);
        }
    }
    else{
        tToken end_token={.type = T_SEMICOLON};
        //mov a token.data.int cislo
        body_var = check_expr_syntax(token, &end_token);
        *token = get_token(1);
        
    }
    return body_var;
}

bool f_body_ret(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool body_ret = false;
    if (token->type == T_SEMICOLON){
        body_ret = true;
    }
    else{
        tToken end_token={.type = T_SEMICOLON};
        body_ret = check_expr_syntax(token, &end_token);
        *token = get_token(1);
    }
    return body_ret;
    
}

bool f_fn_call_l(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool fn_call_bool = false;
    switch (token->type)
    {
    case T_STRING:
    case T_NUM_INT:
    case T_NUM_FLOAT:
    case T_VAR_ID:
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        break;
    default:
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list); //??????????????????
        break;
    }
    return fn_call_bool;
}

bool f_fn_call_lc(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool fn_call_bool2 = false;
    switch (token->type)
    {
    case T_COMMA:
        *token = get_token(1);
        fn_call_bool2 = f_fn_call_l(token,instruction, instruction_list);
        break;
    case T_R_PAR:
        *token = get_token(1);
        fn_call_bool2 = true;
        break;
    default:
        break;
    }
    return fn_call_bool2;
}

bool f_func(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func = false;
    if(token->type == T_FUNCTION){
        *token = get_token(1);
        if(token->type == T_FUN_ID){
            *token = get_token(1);
            if (token->type == T_L_PAR){
                *token = get_token(1);
                func = f_func_param(token,instruction, instruction_list);
                if(token->type == T_COLON && func != false){
                    *token = get_token(1);
                    func = f_func_type(token,instruction, instruction_list) && f_func_dedf(token,instruction, instruction_list); 
                }
                //TODO SKONTROLOVAT MAGIA
            }
        }
    }
    return func;
}
bool f_func_dedf(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func_dedf = false;
    if(token->type == T_L_BRAC){
        *token = get_token(1);
        func_dedf = f_in_body(token,instruction, instruction_list);
    }
    else if (token->type == T_SEMICOLON){
        *token = get_token(1);
        func_dedf = true;
    }
    return func_dedf;
}

bool f_func_type(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
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

bool f_in_body(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool in_body = false;
    if(token->type== T_R_BRAC){
        *token = get_token(1);
        in_body = true;
        return in_body;
    }
    in_body = f_body(token,instruction, instruction_list) && f_in_body(token,instruction, instruction_list);
    return in_body;
}

bool f_func_param(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func_param = false;
    switch (token->type)
    {
    case T_STRING:
    case T_NUM_INT:
    case T_NUM_FLOAT:
        *token = get_token(1);
        func_param = f_func_mparam(token,instruction, instruction_list);
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
            func_param = f_func_mparam(token,instruction, instruction_list);
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

bool f_func_mparam(tToken * token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func_mparam = false;
    switch (token->type)
    {
    case T_STRING:
    case T_NUM_INT:
    case T_NUM_FLOAT:
        *token = get_token(1);
        func_mparam = f_func_mparam(token,instruction, instruction_list);
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
            func_mparam = f_func_mparam(token,instruction, instruction_list);
        }
        break;
    case T_COMMA:
        *token = get_token(1);
        func_mparam = f_func_mparam(token,instruction, instruction_list);
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
    tDynamicBuffer *instruction = dynamicBuffer_INIT();
    symtable = htab_init(HTAB_SIZE);
    DLList instruction_list;
    DLL_Init(&instruction_list);
    DLL_InsertFirst(&instruction_list, instruction);
    DLL_First(&instruction_list);
    
    tToken token = get_token(0);
    if (f_start(&token,instruction,&instruction_list)){
        printf("%s\n", instruction_list.last->instruction->data);
        printf("SYNTAX OK\n");
        return 0;
    }
    else{
        printf("SYNTAX ERROR\n");
        return 1;
    }
}
