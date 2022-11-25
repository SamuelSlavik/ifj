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
tStack frames;
htab_t *local_sym;
extern htab_t *symtable;
void print_instructions(DLList *instruction_list){
    DLL_instruction *tmp = instruction_list->first;
    while(tmp != NULL){
        printf("%s\n", tmp->instruction->data);
        tmp = tmp->nextElement;
    }

}

//<start> =>  [T_PROLOG] <prog>
bool f_start(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
    if (token->type == T_ERROR){
        exit(1); //todo error.c
    }
    bool start = false;
    if (token->type == T_PROLOG) {
        *token = get_token(1);
        start = f_prog(token,instruction, instruction_list);
    }
    return start;
}

bool f_prog(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
    bool prog = false;
    if (token->type == T_ERROR){
        exit(1);
    }
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
    if (token->type == T_ERROR){
        exit(1);
    }
    tToken end_token;
    tToken tmp_token;
    switch (token->type)
    {
    case T_VAR_ID:
        if (htab_find(local_sym,token->data.STRINGval->data) == NULL ){
            dynamicBuffer_ADD_STRING(instruction, "DEFVAR ");
            dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
            DLL_InsertAfter(instruction_list,instruction);
            DLL_Next(instruction_list);
            instruction = dynamicBuffer_RESET(instruction);
        }
        instruction_list->first->curr_var=st_var_create(local_sym, token->data.STRINGval->data);
        *token = get_token(1);
        
        if (token->type == T_ASSIGN){
            *token = get_token(1);
            body = f_body_var(token,instruction, instruction_list);
            dynamicBuffer_ADD_STRING(instruction, "POPS ");
            dynamicBuffer_ADD_STRING(instruction, instruction_list->first->curr_var->key); //vec pozor
            DLL_InsertAfter(instruction_list,instruction);
            DLL_Next(instruction_list);
            instruction = dynamicBuffer_RESET(instruction);
        }
        else{
            tmp_token.type = T_VAR_ID;
            end_token.type = T_SEMICOLON;
            tmp_token.data.STRINGval= dynamicBuffer_INIT();
            dynamicBuffer_ADD_STRING(tmp_token.data.STRINGval,instruction_list->first->curr_var->key);
            body = check_expr_syntax(token, &end_token,instruction_list, &tmp_token);
        }
        break;
    case T_FUN_ID:
        //local_sym = st_fun_call(symtable,&frames,token->data.STRINGval->data);
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
            body = check_expr_syntax(token, &end_token,instruction_list,NULL);
            if (body == false) return body; //znova magia skontrolovat
            *token = get_token(1);
            if (token->type == T_L_BRAC){
                *token = get_token(1);
                body = f_in_body(token,instruction, instruction_list);
                if (body == false) return body;
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
            body = check_expr_syntax(token, &end_token,instruction_list,NULL);
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
        body = check_expr_syntax(token, &end_token,instruction_list,NULL);
        *token = get_token(1);
        break;
    }
    return body;
}
bool f_body_var(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool body_var = false;
    if (token->type == T_ERROR){
        exit(1);
    }
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
        body_var = check_expr_syntax(token, &end_token,instruction_list,NULL);
        *token = get_token(1);
        
    }
    return body_var;
}

bool f_body_ret(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool body_ret = false;
    if (token->type == T_ERROR){
        exit(1);
    }
    if (token->type == T_SEMICOLON){
        *token=get_token(1);
        body_ret = true;
    }
    else{
        tToken end_token={.type = T_SEMICOLON};
        body_ret = check_expr_syntax(token, &end_token,instruction_list,NULL);
        *token = get_token(1);
    }
    return body_ret;
    
}

bool f_fn_call_l(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool fn_call_bool = false;
    if (token->type == T_ERROR){
        exit(1);
    }
    switch (token->type)
    {
    case T_STRING:
    case T_NUM_INT:
    case T_NUM_FLOAT:
    case T_VAR_ID:
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        break;
    case T_R_PAR:
        *token = get_token(1);
        fn_call_bool = true;
    default:
        break;
    }
    return fn_call_bool;
}

bool f_fn_call_lc(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool fn_call_bool2 = false;
    if (token->type == T_ERROR){
        exit(1);
    }
    switch (token->type)
    {
    case T_COMMA:
        *token = get_token(1);
        fn_call_bool2 = f_fn_call_lparam(token,instruction, instruction_list);
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

bool f_fn_call_lparam(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool fn_call_bool = false;
    if (token->type == T_ERROR){
        exit(1);
    }
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
        break;
    }
    return fn_call_bool;

}

bool f_func(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func = false;
    if (token->type == T_ERROR){
        exit(1);
    }
    if(token->type == T_FUNCTION){
        *token = get_token(1);
        if(token->type == T_FUN_ID){
            if (htab_find(symtable,token->data.STRINGval->data) == NULL ){
                label_name_gen(token->data.STRINGval->data);
            //TODO IFJCODE
            }
            instruction_list->first->curr_fun=st_fun_create(symtable, token->data.STRINGval->data);
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
    local_sym = htab_init(HTAB_SIZE);
    StackPush(&frames,(void*)local_sym);
    
    st_fun_definition(instruction_list->first->curr_fun);
    if (token->type == T_ERROR){
        exit(1);
    }
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
    if (token->type == T_ERROR){
        exit(1);
    }
    switch (token->type)
    {
    case T_VOID:
    case T_STRING_TYPE:
    case T_FLOAT_TYPE:
    case T_INT_TYPE:
    case T_STRING_N_TYPE:
    case T_FLOAT_N_TYPE:
    case T_INT_N_TYPE:
        st_fun_retrun_type(instruction_list->first->curr_fun,token->type);
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
    if (token->type == T_ERROR){
        exit(1);
    }
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
    if (token->type == T_ERROR){
        exit(1);
    }
    switch (token->type)
    {
    case T_STRING_TYPE:
    case T_FLOAT_TYPE:
    case T_INT_TYPE:
    case T_STRING_N_TYPE:
    case T_FLOAT_N_TYPE:
    case T_INT_N_TYPE:
        st_fun_param_type(instruction_list->first->curr_fun->data.fun_data.TaV,token->type);
        *token = get_token(1);
        func_param = f_func_dedf_param_type(token,instruction, instruction_list);
        break;
    case T_R_PAR:
        *token = get_token(1);
        func_param = true;
    default:
        break;
    }
    return func_param;
}

bool f_func_dedf_param_type(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func_param = false;
    if(token->type == T_VAR_ID){         //skontrolovat first
        st_fun_param_name(instruction_list->first->curr_fun->data.fun_data.TaV,token->data.STRINGval);
        *token = get_token(1);
        func_param = f_func_dedf_param_var(token,instruction, instruction_list);
    }
    return func_param;
}

bool f_func_dedf_param_var(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func_param = false;
    if(token->type == T_COMMA){
        *token = get_token(1);
        switch (token->type)
        {
        case T_STRING_TYPE:
        case T_FLOAT_TYPE:
        case T_INT_TYPE:
        case T_STRING_N_TYPE:
        case T_FLOAT_N_TYPE:
        case T_INT_N_TYPE:
            st_fun_param_type(instruction_list->first->curr_fun->data.fun_data.TaV,token->type);
            *token = get_token(1);
            func_param = f_func_dedf_param_type(token,instruction, instruction_list);
            break;
        default:
            break;
        }
    }
    else if(token->type == T_R_PAR){
        *token = get_token(1);
        func_param = true;
    }
    return func_param;
}

int main(){
    tDynamicBuffer *instruction = dynamicBuffer_INIT();
    symtable = htab_init(HTAB_SIZE);
    DLList instruction_list;
    StackInit(&frames);
    dynamicBuffer_ADD_STRING(instruction,".IFJcode22");
    DLL_Init(&instruction_list);
    DLL_InsertFirst(&instruction_list, instruction);
    dynamicBuffer_RESET(instruction);
    DLL_First(&instruction_list);
    tToken token = get_token(0);
    if (f_start(&token,instruction,&instruction_list)){
        printf("SYNTAX OK\n");
    }
    else{
        printf("SYNTAX ERROR\n");
    }
    print_instructions(&instruction_list);
}
