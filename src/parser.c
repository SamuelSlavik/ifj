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
#include "error.h"

#define ERROR_EXIT(flag,token,errcode) if (!(flag)){error_exit(token,errcode);}
tStack frames;
htab_t *local_sym;
extern htab_t *symtable;


void print_stack(tStack *expr_stack, tDynamicBuffer *instruction, DLList *instruction_list,char *code){
    // PRINT STACK
    tStack print_stack;
    StackInit(&print_stack);

    while (!StackIsEmpty(expr_stack)){
        tVar_TaV *stack_top_itm = ((tVar_TaV*) StackTop(expr_stack));
        dynamicBuffer_ADD_STRING(instruction, code);
        dynamicBuffer_ADD_STRING(instruction, stack_top_itm->var);
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        StackPush(&print_stack, stack_top_itm);
        StackPop(expr_stack);
    }
    while (!StackIsEmpty(&print_stack)){
        tVar_TaV *stack_top_itm = ((tVar_TaV*) StackTop(&print_stack));
        StackPush(expr_stack, stack_top_itm);
        StackPop(&print_stack);
    }
    // END PRINT STACK
}

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
        ERROR_EXIT(start,token,SYNTAX_ERROR);
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
        if (htab_find(symtable,token->data.STRINGval->data) == NULL ){
            dynamicBuffer_ADD_STRING(instruction, "DEFVAR ");
            dynamicBuffer_ADD_STRING(instruction, "LF@");
            dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
            DLL_InsertAfter(instruction_list,instruction);
            DLL_Next(instruction_list);
            instruction = dynamicBuffer_RESET(instruction);
        }
        instruction_list->first->curr_var=st_var_create(symtable, token->data.STRINGval->data);
        *token = get_token(1);
        
        if (token->type == T_ASSIGN){
            *token = get_token(1);
            body = f_body_var(token,instruction, instruction_list);
            ERROR_EXIT(body,token,SYNTAX_ERROR);
            dynamicBuffer_ADD_STRING(instruction, "POPS ");
            dynamicBuffer_ADD_STRING(instruction, "LF@");
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
            ERROR_EXIT(body,token,SYNTAX_ERROR);
        }
        break;
    case T_FUN_ID:
        //local_sym = st_fun_call(symtable,&frames,token->data.STRINGval->data);
        //opravit pre inbuild funkcie
        if (htab_find(symtable,token->data.STRINGval->data) == NULL ){
            //ERROR_EXIT(htab_find(symtable,token->data.STRINGval->data) == NULL,token,RE_DEF_ERROR); da sa to aj takto menit
            error_exit(token,RE_DEF_ERROR);
        }
        instruction_list->first->curr_fun=htab_find(symtable, token->data.STRINGval->data);
        dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME");
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        print_stack(instruction_list->first->curr_fun->data.fun_data.TaV,instruction,instruction_list, "DEFVAR TF@");
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            
            body = f_fn_call_l(token,instruction, instruction_list);
            ERROR_EXIT(body,token,SYNTAX_ERROR);
            if (token->type != T_SEMICOLON){
                exit(SYNTAX_ERROR);
            }
            dynamicBuffer_ADD_STRING(instruction, "CALL");
            DLL_InsertAfter(instruction_list,instruction);
            DLL_Next(instruction_list);
            instruction = dynamicBuffer_RESET(instruction);

            *token = get_token(1);
        }
        break;
    case T_RETURN:
        *token = get_token(1);
        body = f_body_ret(token,instruction, instruction_list);
        ERROR_EXIT(body,token,SYNTAX_ERROR);
        dynamicBuffer_ADD_STRING(instruction, "POPFRAME");
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        dynamicBuffer_ADD_STRING(instruction, "RETURN");
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
    break;
    case T_IF:
        *token = get_token(1);
        if(token->type == T_L_PAR){
            *token = get_token(1);
            end_token.type = T_R_PAR;
            body = check_expr_syntax(token, &end_token,instruction_list,NULL);
            ERROR_EXIT(body,token,SYNTAX_ERROR); //znova magia skontrolovat
            *token = get_token(1);
            if (token->type == T_L_BRAC){
                *token = get_token(1);
                body = f_in_body(token,instruction, instruction_list);
                ERROR_EXIT(body,token,SYNTAX_ERROR);
                if (token->type == T_ELSE){
                    *token = get_token(1);
                    if(token->type == T_L_BRAC){
                        *token = get_token(1);
                        body = f_in_body(token,instruction, instruction_list);
                        ERROR_EXIT(body,token,SYNTAX_ERROR);
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
            ERROR_EXIT(body,token,SYNTAX_ERROR);
            if (body == false) return body; //znova magia skontrolovat
            *token = get_token(1);
            if (token->type == T_L_BRAC){
                *token = get_token(1);
                body = f_in_body(token,instruction, instruction_list);
                ERROR_EXIT(body,token,SYNTAX_ERROR);
            }
        }
    break;
    default:
        end_token.type = T_SEMICOLON;
        body = check_expr_syntax(token, &end_token,instruction_list,NULL);
        ERROR_EXIT(body,token,SYNTAX_ERROR);
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
            ERROR_EXIT(body_var,token,SYNTAX_ERROR);
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
        ERROR_EXIT(body_var,token,SYNTAX_ERROR);
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
        ERROR_EXIT(body_ret,token,SYNTAX_ERROR);
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
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "string@");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_INT:
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "int@");
        dynamicBuffer_ADD_STRING(instruction, "5");
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_FLOAT:
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "string@");
        dynamicBuffer_ADD_STRING(instruction, "3.14");
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_VAR_ID:
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        // pridat radmec dynamicBuffer_ADD_STRING(instruction, "");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
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
        ERROR_EXIT(fn_call_bool2,token,SYNTAX_ERROR);
        break;
    case T_R_PAR:
        print_stack(instruction_list->first->curr_fun->data.fun_data.TaV,instruction,instruction_list, "POPS TF@");
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
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "string@");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_INT:
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "int@");
        dynamicBuffer_ADD_STRING(instruction, "10");
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_FLOAT:
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "float@");
        dynamicBuffer_ADD_STRING(instruction, "3.14");
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_VAR_ID:
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        // pridat radmec dynamicBuffer_ADD_STRING(instruction, "");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        DLL_InsertAfter(instruction_list,instruction);
        DLL_Next(instruction_list);
        instruction = dynamicBuffer_RESET(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
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
                dynamicBuffer_ADD_STRING(instruction, "LABEL ");
                label_name_gen(token->data.STRINGval->data);
                DLL_InsertAfter(instruction_list,instruction);
                DLL_Next(instruction_list);
                instruction = dynamicBuffer_RESET(instruction);
                dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME");
                DLL_InsertAfter(instruction_list,instruction);
                DLL_Next(instruction_list);
                instruction = dynamicBuffer_RESET(instruction);
            //TODO IFJCODE
            }
            instruction_list->first->curr_fun=st_fun_create(symtable, token->data.STRINGval->data);
            *token = get_token(1);
            if (token->type == T_L_PAR){
                *token = get_token(1);
                func = f_func_param(token,instruction, instruction_list);
                ERROR_EXIT(func,token,SYNTAX_ERROR);
                if(token->type == T_COLON){
                    *token = get_token(1);
                    func = f_func_type(token,instruction, instruction_list) && f_func_dedf(token,instruction, instruction_list);
                    ERROR_EXIT(func,token,SYNTAX_ERROR); 
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
        ERROR_EXIT(func_dedf,token,SYNTAX_ERROR);
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
    ERROR_EXIT(in_body,token,SYNTAX_ERROR);
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
        ERROR_EXIT(func_param,token,SYNTAX_ERROR);
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
        st_fun_param_name(instruction_list->first->curr_fun->data.fun_data.TaV,token->data.STRINGval->data);
        *token = get_token(1);
        func_param = f_func_dedf_param_var(token,instruction, instruction_list);
        ERROR_EXIT(func_param,token,SYNTAX_ERROR);
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
            ERROR_EXIT(func_param,token,SYNTAX_ERROR);
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
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME");
    DLL_InsertAfter(&instruction_list,instruction);
    DLL_Next(&instruction_list);
    instruction = dynamicBuffer_RESET(instruction);
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME");
    DLL_InsertAfter(&instruction_list,instruction);
    DLL_Next(&instruction_list);
    instruction = dynamicBuffer_RESET(instruction);
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME");
    DLL_InsertAfter(&instruction_list,instruction);
    DLL_Next(&instruction_list);
    instruction = dynamicBuffer_RESET(instruction);
    tToken token = get_token(0);
    if (f_start(&token,instruction,&instruction_list) == false){ //debug error print
        error_exit(&token,SYNTAX_ERROR);
        printf("PREBUBLALO TO AZ DO MAINU SKONTROLOVAT\n");
    }
    print_instructions(&instruction_list);
    return 0;
}
