/**
 * @file parser.c
 * @brief Implementation of top-down parser
 * @authors Jakub Kontrik (xkontr02), Adam Pekny(xpekny00), Samuel Slavik (xslavi37)
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
#include "expression_codegen.h"
#include "generator.h"

extern htab_t *symtable;


bool f_start(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    bool start = false;
    if (token->type == T_PROLOG) {
        *token = get_token(1);
        start = f_prog(token,instruction, instruction_list);
        ASSERT_ERROR(start,token,SYNTAX_ERROR);
    }
    return start;
}

bool f_prog(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
    bool prog = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
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
    //<prog> => <fn> <prog>
    //<prog> => <body> <prog>
    prog = (f_func(token,instruction, instruction_list) && f_prog(token,instruction, instruction_list))
            || (f_body(token,instruction, instruction_list) && f_prog(token,instruction, instruction_list));
    return prog;
}

bool f_body(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
    bool body = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    tDynamicBuffer *labelname;
    tDynamicBuffer *label_name_if;
    tToken end_token;
    tToken tmp_token;
    switch (token->type)
    {
    case T_VAR_ID:
        tmp_token.data.STRINGval= dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(tmp_token.data.STRINGval,token->data.STRINGval->data);
        tmp_token.line = token->line;
        *token = get_token(1);
        
        if (token->type == T_ASSIGN){
            //adding variable into function`s local symtable
            if (htab_find(instruction_list->called_from->data.fun_data.localST,tmp_token.data.STRINGval->data) == NULL ){
                instruction = dynamicBuffer_INIT();
                dynamicBuffer_ADD_STRING(instruction, "DEFVAR ");
                dynamicBuffer_ADD_STRING(instruction, "LF@");
                dynamicBuffer_ADD_STRING(instruction, tmp_token.data.STRINGval->data);
                //checking if in while loop or condition
                if (instruction_list->if_while != NULL){
                    DLL_InsertBefore_if_while(instruction_list,instruction);
                }
                else{
                    DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                }
                dynamicBufferFREE(instruction);
            }
            instruction_list->curr_var=st_var_create(instruction_list->called_from->data.fun_data.localST, tmp_token.data.STRINGval->data);

            *token = get_token(1);
            body = f_body_var(token,instruction, instruction_list);
            ASSERT_ERROR(body,token,SYNTAX_ERROR);
            instruction = dynamicBuffer_INIT();
            dynamicBuffer_ADD_STRING(instruction, "POPS ");
            dynamicBuffer_ADD_STRING(instruction, "LF@");
            dynamicBuffer_ADD_STRING(instruction, instruction_list->curr_var->key); //vec pozor
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
        }
        else{
            tmp_token.type = T_VAR_ID;
            end_token.type = T_SEMICOLON;
            body = check_expr_syntax(token, &end_token,instruction_list, &tmp_token);
            ASSERT_ERROR(body,token,SYNTAX_ERROR);
            *token = get_token(1);
        }
        break;
    case T_FUN_ID:
        //checking funtion is in symtable
        ASSERT_ERROR(htab_find(symtable,token->data.STRINGval->data) != NULL,token,RE_DEF_ERROR);
        //setting current function
        instruction_list->curr_fun=htab_find(symtable, token->data.STRINGval->data);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME");
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        //setting up variables with print_stack function if none of these functions
        if (strcmp(instruction_list->curr_fun->key,"write") 
            && strcmp(instruction_list->curr_fun->key,"substring")
            && strcmp(instruction_list->curr_fun->key,"ord")
            && strcmp(instruction_list->curr_fun->key,"chr")
            && strcmp(instruction_list->curr_fun->key,"strlen")){
            print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "DEFVAR TF@");
        }
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            instruction_list->num_of_params_called=0;
            body = f_fn_call_l(token,instruction, instruction_list);
            ASSERT_ERROR(body,token,SYNTAX_ERROR);
            ASSERT_ERROR(token->type == T_SEMICOLON,token,SYNTAX_ERROR);

            instruction = dynamicBuffer_INIT();
            if (!strcmp(instruction_list->curr_fun->key,"write")){
                //pushing args for write function
                while (!StackIsEmpty(&instruction_list->called_args)){
                    tDynamicBuffer *stack_top_itm = ((tDynamicBuffer *) StackTop(&instruction_list->called_args));
                    DETECT_MAIN(instruction_list,stack_top_itm,instruction_list->called_from->key);
                    StackPop(&instruction_list->called_args);
                    dynamicBufferFREE(stack_top_itm);
                }
                dynamicBuffer_ADD_STRING(instruction, "PUSHS int@");
                tDynamicBuffer *numof_param = dynamicBuffer_INIT();
                numof_param= long_2_string(instruction_list->num_of_params_called);
                dynamicBuffer_ADD_STRING(instruction, numof_param->data);
                dynamicBuffer_ADD_STRING(instruction, "\n");
                dynamicBuffer_ADD_STRING(instruction, "CALL ");
                dynamicBuffer_ADD_STRING(instruction,"functionWrite\n");
                dynamicBuffer_ADD_STRING(instruction,"PUSHS nil@nil");
                dynamicBufferFREE(numof_param);
            }
            else{
                //runtime type check of called arguments
                check_fn_arguments(instruction_list->curr_fun->data.fun_data.TaV,&instruction_list->called_args,instruction,instruction_list);
                //setting up variables with print_stack function if none of these functions
                if(strcmp(instruction_list->curr_fun->key,"chr")
                    &&strcmp(instruction_list->curr_fun->key,"ord")
                    &&strcmp(instruction_list->curr_fun->key,"strlen")
                    &&strcmp(instruction_list->curr_fun->key,"substring")){
                    print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "POPS TF@");
                }
                dynamicBuffer_ADD_STRING(instruction, "CALL ");
                dynamicBuffer_ADD_STRING(instruction, instruction_list->curr_fun->data.fun_data.label_name->data);
            }
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
            *token = get_token(1);
        }
        break;
    case T_RETURN:
        //checking if return is in mainbody
        if (!strcmp(instruction_list->called_from->key,"$$main")){
            instruction = dynamicBuffer_INIT();
            dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME\n");
            dynamicBuffer_ADD_STRING(instruction, "EXIT int@0");
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
        }
        instruction_list->called_from->data.fun_data.has_ret=true;
        *token = get_token(1);
        body = f_body_ret(token,instruction, instruction_list);
        ASSERT_ERROR(body,token,SYNTAX_ERROR);
        check_return_type(instruction,instruction_list);

        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "POPFRAME");
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "RETURN");
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        
    break;
    case T_IF:
        //generating unique label for if
        label_name_if = label_name_gen(token->data.STRINGval->data);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "LABEL if_");
        dynamicBuffer_ADD_STRING(instruction, label_name_if->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        //condition for nested ifs and whiles
        if (instruction_list->if_while == NULL){
            instruction_list->label = label_name_if->data;
            if (!strcmp(instruction_list->called_from->key,"$$main")){
                instruction_list->if_while=instruction_list->main_body;
            }
            else{
                instruction_list->if_while=instruction_list->active;
            }
        }

        *token = get_token(1);
        if(token->type == T_L_PAR){
            *token = get_token(1);
            end_token.type = T_R_PAR;
            body = check_expr_syntax(token, &end_token,instruction_list,NULL);
            ASSERT_ERROR(body,token,SYNTAX_ERROR);
            //converting value into bool type from expression parser
            convert_into_bool(instruction,instruction_list,label_name_if);
            *token = get_token(1);
            if (token->type == T_L_BRAC){
                //generating labels for jumping
                *token = get_token(1);
                body = f_in_body(token,instruction, instruction_list);
                ASSERT_ERROR(body,token,SYNTAX_ERROR);            
                instruction = dynamicBuffer_INIT();
                dynamicBuffer_ADD_STRING(instruction, "JUMP end_");
                dynamicBuffer_ADD_STRING(instruction, label_name_if->data);
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                if (token->type == T_ELSE){
                    *token = get_token(1);
                    if(token->type == T_L_BRAC){
                        instruction = dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction, "LABEL false_");
                        dynamicBuffer_ADD_STRING(instruction, label_name_if->data);
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                        *token = get_token(1);
                        body = f_in_body(token,instruction, instruction_list);
                        ASSERT_ERROR(body,token,SYNTAX_ERROR);
                        
                        instruction = dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction, "LABEL end_");
                        dynamicBuffer_ADD_STRING(instruction, label_name_if->data);
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                        //most outer if or while check
                        if (!strcmp(instruction_list->label,label_name_if->data)){
                            instruction_list->if_while=NULL; 
                        }
                    }
                }
                else{
                    error_exit(token,SYNTAX_ERROR);
                }
            }
        }
    break;
    case T_WHILE:
        //generating unique label for while
        labelname = label_name_gen(token->data.STRINGval->data);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "LABEL while_");
        dynamicBuffer_ADD_STRING(instruction, labelname->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        //condition for nested ifs and whiles
        if (instruction_list->if_while == NULL){
            instruction_list->label = labelname->data;
            if (!strcmp(instruction_list->called_from->key,"$$main")){
                instruction_list->if_while=instruction_list->main_body;
            }
            else{
                instruction_list->if_while=instruction_list->active;
            }
        }
        *token = get_token(1);
        
        if(token->type == T_L_PAR){
            *token = get_token(1);
            end_token.type = T_R_PAR;
            body = check_expr_syntax(token, &end_token,instruction_list,NULL);
            ASSERT_ERROR(body,token,SYNTAX_ERROR);
            //converting value into bool type from expression parser
            convert_into_bool(instruction,instruction_list,labelname);
            *token = get_token(1);
            if (token->type == T_L_BRAC){  
                //generating labels for jumping              
                *token = get_token(1);
                body = f_in_body(token,instruction, instruction_list);
                ASSERT_ERROR(body,token,SYNTAX_ERROR);
                instruction = dynamicBuffer_INIT();
                dynamicBuffer_ADD_STRING(instruction, "JUMP while_");
                dynamicBuffer_ADD_STRING(instruction, labelname->data);
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                instruction = dynamicBuffer_INIT();
                dynamicBuffer_ADD_STRING(instruction, "LABEL false_");
                dynamicBuffer_ADD_STRING(instruction, labelname->data);
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                //most outer if or while check
                if (!strcmp(instruction_list->label,labelname->data)){
                    instruction_list->if_while=NULL; 
                }          
            }
        }
    break;
    default:
        end_token.type = T_SEMICOLON;
        body = check_expr_syntax(token, &end_token,instruction_list,NULL);
        ASSERT_ERROR(body,token,SYNTAX_ERROR);
        *token = get_token(1);
        break;
    }
    return body;
}
bool f_body_var(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool body_var = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    if(token->type == T_FUN_ID){
        //checking funtion is in symtable
        ASSERT_ERROR(htab_find(symtable,token->data.STRINGval->data) != NULL,token,RE_DEF_ERROR);
        //setting current function
        instruction_list->curr_fun=htab_find(symtable, token->data.STRINGval->data);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME");
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        //setting up variables with print_stack function if none of these functions
        if (strcmp(instruction_list->curr_fun->key,"write") 
            && strcmp(instruction_list->curr_fun->key,"substring")
            && strcmp(instruction_list->curr_fun->key,"ord")
            && strcmp(instruction_list->curr_fun->key,"chr")
            && strcmp(instruction_list->curr_fun->key,"strlen")){
            print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "DEFVAR TF@");
        }
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            instruction_list->num_of_params_called=0;
            body_var = f_fn_call_l(token,instruction, instruction_list);
            ASSERT_ERROR(body_var,token,SYNTAX_ERROR);
            ASSERT_ERROR(token->type == T_SEMICOLON,token,SYNTAX_ERROR);
            
            instruction = dynamicBuffer_INIT();
            if (!strcmp(instruction_list->curr_fun->key,"write")){
                //pushing args for write function
                while (!StackIsEmpty(&instruction_list->called_args)){
                    tDynamicBuffer *stack_top_itm = ((tDynamicBuffer *) StackTop(&instruction_list->called_args));
                    DETECT_MAIN(instruction_list,stack_top_itm,instruction_list->called_from->key);
                    StackPop(&instruction_list->called_args);
                    dynamicBufferFREE(stack_top_itm);
                }
                dynamicBuffer_ADD_STRING(instruction, "PUSHS int@");
                tDynamicBuffer *numof_param = dynamicBuffer_INIT();
                numof_param= long_2_string(instruction_list->num_of_params_called);
                dynamicBuffer_ADD_STRING(instruction, numof_param->data);
                dynamicBuffer_ADD_STRING(instruction, "\n");
                dynamicBuffer_ADD_STRING(instruction, "CALL ");
                dynamicBuffer_ADD_STRING(instruction,"functionWrite\n");
                dynamicBuffer_ADD_STRING(instruction,"PUSHS nil@nil");
                dynamicBufferFREE(numof_param);
            }
            else{
                //runtime type check of called arguments
                check_fn_arguments(instruction_list->curr_fun->data.fun_data.TaV,&instruction_list->called_args,instruction,instruction_list);
                //setting up variables with print_stack function if none of these functions
                if(strcmp(instruction_list->curr_fun->key,"chr")
                    &&strcmp(instruction_list->curr_fun->key,"ord")
                    &&strcmp(instruction_list->curr_fun->key,"strlen")
                    &&strcmp(instruction_list->curr_fun->key,"substring")){
                    print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "POPS TF@");
                }
                dynamicBuffer_ADD_STRING(instruction, "CALL ");
                dynamicBuffer_ADD_STRING(instruction, instruction_list->curr_fun->data.fun_data.label_name->data);
            }
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            *token = get_token(1);
        }
    }
    else{
        tToken end_token={.type = T_SEMICOLON};
        body_var = check_expr_syntax(token, &end_token,instruction_list,NULL);
        ASSERT_ERROR(body_var,token,SYNTAX_ERROR);
        *token = get_token(1);
    }
    return body_var;
}

bool f_body_ret(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool body_ret = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    if (token->type == T_SEMICOLON){
        //return; can be only in VOID function
        ASSERT_ERROR(instruction_list->called_from->data.fun_data.return_type == T_VOID,token,RETURN_ERROR);
        instruction=dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction,"PUSHS nil@nil");
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        *token=get_token(1);
        body_ret = true;
    }
    else if (token->type == T_FUN_ID){
        if (instruction_list->called_from->data.fun_data.return_type == T_VOID && strcmp(instruction_list->called_from->key,"$$main")){
            error_exit(token,RETURN_ERROR);
        }
        body_ret = f_body_var(token,instruction, instruction_list);
    }
    else{
        if (instruction_list->called_from->data.fun_data.return_type == T_VOID && strcmp(instruction_list->called_from->key,"$$main")){
            error_exit(token,RETURN_ERROR);
        }
        tToken end_token={.type = T_SEMICOLON};
        body_ret = check_expr_syntax(token, &end_token,instruction_list,NULL);
        ASSERT_ERROR(body_ret,token,SYNTAX_ERROR);
        *token = get_token(1);
    }
    return body_ret;
    
}

bool f_fn_call_l(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool fn_call_bool = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    switch (token->type)
    {
    case T_STRING:
        instruction_list->num_of_params_called++;
        
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "string@");
        string_to_ifj_fmt(&token->data.STRINGval);
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        //if one of these there is no need of type control
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_INT:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "int@");
        dynamicBuffer_ADD_STRING(instruction, long_2_string(token->data.INTval)->data);
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_FLOAT:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "float@");
        dynamicBuffer_ADD_STRING(instruction, double_2_string(token->data.FLOATval)->data);
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_VAR_ID:
        instruction_list->num_of_params_called++;
        //check if variable is declared
        ASSERT_ERROR(htab_find(instruction_list->called_from->data.fun_data.localST,token->data.STRINGval->data)!=NULL,token,UN_DEF_VAR_ERROR);
        //check if variable is initialized
        var_init_check(instruction_list, token->data.STRINGval);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS LF@");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NULL:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "nil@nil");
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_R_PAR:
        //cecking number of called arguments except write that can have infinite args;
        if(strcmp(instruction_list->curr_fun->key,"write")){
            ASSERT_ERROR(instruction_list->num_of_params_called == instruction_list->curr_fun->data.fun_data.number_of_params,token,PARAM_ERROR);
        }
        *token = get_token(1);
        fn_call_bool = true;
    default:
        break;
    }
    return fn_call_bool;
}

bool f_fn_call_lc(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool fn_call_bool2 = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    switch (token->type)
    {
    case T_COMMA:
        *token = get_token(1);
        fn_call_bool2 = f_fn_call_lparam(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool2,token,SYNTAX_ERROR);
        break;
    case T_R_PAR:
        //cecking number of called arguments except write that can have infinite args;
        if(strcmp(instruction_list->curr_fun->key,"write")){
            ASSERT_ERROR(instruction_list->num_of_params_called == instruction_list->curr_fun->data.fun_data.number_of_params,token,PARAM_ERROR);
        }
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
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    switch (token->type)
    {
    case T_STRING:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "string@");
        string_to_ifj_fmt(&token->data.STRINGval);
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        //if one of these there is no need of type control
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_INT:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "int@");
        dynamicBuffer_ADD_STRING(instruction, long_2_string(token->data.INTval)->data);
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_FLOAT:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "float@");
        dynamicBuffer_ADD_STRING(instruction, double_2_string(token->data.FLOATval)->data);
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_VAR_ID:
        instruction_list->num_of_params_called++;
        //check is variable is declared
        ASSERT_ERROR(htab_find(instruction_list->called_from->data.fun_data.localST,token->data.STRINGval->data)!=NULL,token,UN_DEF_VAR_ERROR);
        //check if variable is initialized
        var_init_check(instruction_list, token->data.STRINGval);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS LF@");
        // pridat radmec dynamicBuffer_ADD_STRING(instruction, "");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NULL:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "nil@nil");
        if (!strcmp(instruction_list->curr_fun->key,"strval") 
            || !strcmp(instruction_list->curr_fun->key,"intval") 
            || !strcmp(instruction_list->curr_fun->key,"floatval")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&instruction_list->called_args,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ASSERT_ERROR(fn_call_bool,token,SYNTAX_ERROR);
        break;
    default:
        break;
    }
    return fn_call_bool;

}

bool f_func(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    if(token->type == T_FUNCTION){
        *token = get_token(1);
        if(token->type == T_FUN_ID){    
            //generating if code for new function and checking if it is defined    
            if (htab_find(symtable,token->data.STRINGval->data) == NULL ){
                instruction_list->curr_fun=st_fun_create(symtable, token->data.STRINGval->data);
                instruction_list->called_from=st_fun_create(symtable, token->data.STRINGval->data);
                instruction = dynamicBuffer_INIT();
                dynamicBuffer_ADD_STRING(instruction, "LABEL ");
                instruction_list->curr_fun->data.fun_data.label_name = label_name_gen(token->data.STRINGval->data);
                dynamicBuffer_ADD_STRING(instruction, instruction_list->curr_fun->data.fun_data.label_name->data);
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                instruction = dynamicBuffer_INIT();
                dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME");
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                instruction = dynamicBuffer_INIT();
                dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME");
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                st_fun_table_create(symtable,(char *)instruction_list->curr_fun->key);
            }
            else{
                error_exit(token,RE_DEF_ERROR);
            }
            *token = get_token(1);
            if (token->type == T_L_PAR){
                *token = get_token(1);
                func = f_func_param(token,instruction, instruction_list);
                ASSERT_ERROR(func,token,SYNTAX_ERROR);
                if(token->type == T_COLON){
                    *token = get_token(1);
                    func = f_func_type(token, instruction_list) && f_func_dedf(token,instruction, instruction_list);
                    ASSERT_ERROR(func,token,SYNTAX_ERROR);
                    //if non-void function has no return;
                    if (instruction_list->called_from->data.fun_data.has_ret == false 
                        && instruction_list->called_from->data.fun_data.return_type !=T_VOID){
                        error_exit(token,PARAM_ERROR);
                    }
                    if(instruction_list->called_from->data.fun_data.return_type == T_VOID){
                        instruction=dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction,"PUSHS nil@nil");
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                        instruction = dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction, "POPFRAME");
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                        instruction = dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction, "RETURN");
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                    }
                    else{
                        //if non void function has return but it is in if 
                        instruction = dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction, "EXIT int@4");
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                    }
                    //setting calling function "main"
                    instruction_list->called_from = htab_find(symtable, "$$main");
                }
            }
        }
    }
    return func;
}
bool f_func_dedf(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func_dedf = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    //end of func definition
    if(token->type == T_L_BRAC){
        *token = get_token(1);
        func_dedf = f_in_body(token,instruction, instruction_list);
        ASSERT_ERROR(func_dedf,token,SYNTAX_ERROR);
    }
    return func_dedf;
}

bool f_func_type(tToken *token, DLList *instruction_list){
    bool func_type = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    switch (token->type)
    {
    case T_VOID:
    case T_STRING_TYPE:
    case T_FLOAT_TYPE:
    case T_INT_TYPE:
    case T_STRING_N_TYPE:
    case T_FLOAT_N_TYPE:
    case T_INT_N_TYPE:
        st_fun_retrun_type(instruction_list->curr_fun,token->type);
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
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    if(token->type== T_R_BRAC){
        *token = get_token(1);
        in_body = true;
        return in_body;
    }
    in_body = f_body(token,instruction, instruction_list) && f_in_body(token,instruction, instruction_list);
    ASSERT_ERROR(in_body,token,SYNTAX_ERROR);
    return in_body;
}

bool f_func_param(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func_param = false;
    ASSERT_ERROR(token->type != T_ERROR,token,LEX_ERROR);
    switch (token->type)
    {
    case T_STRING_TYPE:
    case T_FLOAT_TYPE:
    case T_INT_TYPE:
    case T_STRING_N_TYPE:
    case T_FLOAT_N_TYPE:
    case T_INT_N_TYPE:
        st_fun_param_type(instruction_list->curr_fun->data.fun_data.TaV,token->type);
        *token = get_token(1);
        func_param = f_func_dedf_param_type(token,instruction, instruction_list);
        ASSERT_ERROR(func_param,token,SYNTAX_ERROR);
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
    if(token->type == T_VAR_ID){
        st_fun_param_name(instruction_list->curr_fun->data.fun_data.TaV,token->data.STRINGval->data);
        //checking if parameter was defined or no, error if 2 or more parameters has same name
        if (htab_find(instruction_list->curr_fun->data.fun_data.localST,token->data.STRINGval->data) == NULL){
            st_var_create(instruction_list->curr_fun->data.fun_data.localST,token->data.STRINGval->data);
        }
        else{
            error_exit(token,OTHER_ERROR);
        }
        instruction_list->curr_fun->data.fun_data.number_of_params++;
        *token = get_token(1);
        func_param = f_func_dedf_param_var(token,instruction, instruction_list);
        ASSERT_ERROR(func_param,token,SYNTAX_ERROR);
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
            st_fun_param_type(instruction_list->curr_fun->data.fun_data.TaV,token->type);
            *token = get_token(1);
            func_param = f_func_dedf_param_type(token,instruction, instruction_list);
            ASSERT_ERROR(func_param,token,SYNTAX_ERROR);
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
    //initialization of structures and needed codegen
    tDynamicBuffer *instruction = dynamicBuffer_INIT();
    symtable = htab_init(HTAB_SIZE);
    DLList instruction_list;
    dynamicBuffer_ADD_STRING(instruction,".IFJcode22");
    DLL_Init(&instruction_list);
    DLL_InsertFirst(&instruction_list, instruction);
    dynamicBufferFREE(instruction);
    StackInit(&instruction_list.called_args);
    DLL_First_main(&instruction_list);
    DLL_First(&instruction_list);
    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME\n");
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME");
    DLL_InsertAfter(&instruction_list,instruction);
    DLL_Next(&instruction_list);
    DLL_Next_main(&instruction_list);
    dynamicBufferFREE(instruction);
    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"DEFVAR GF@expr_var_type");
    DLL_InsertAfter(&instruction_list,instruction);
    DLL_Next(&instruction_list);
    DLL_Next_main(&instruction_list);
    dynamicBufferFREE(instruction);
    tToken token = get_token(0);
    //adding inbuild fuctions and "main" function into symtable
    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"$$main");
    instruction_list.curr_fun = st_fun_create(symtable,instruction->data);
    instruction_list.called_from = htab_find(symtable, "$$main");
    st_fun_retrun_type(instruction_list.curr_fun,T_VOID);
    st_fun_table_create(symtable,(char *)instruction_list.curr_fun->key);
    dynamicBufferFREE(instruction);

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"write"); 
    st_fun_create(symtable,instruction->data);
    dynamicBufferFREE(instruction);

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"reads"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=0;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionReads"); 
    htab_find(symtable,"reads")->data.fun_data.label_name=instruction;

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"readi"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=0;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionReadi"); 
    htab_find(symtable,"readi")->data.fun_data.label_name=instruction;

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"readf"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=0;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionReadf"); 
    htab_find(symtable,"readf")->data.fun_data.label_name=instruction;

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"floatval"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=1;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionFloatVal"); 
    htab_find(symtable,"floatval")->data.fun_data.label_name=instruction;

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"intval"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=1;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionIntVal"); 
    htab_find(symtable,"intval")->data.fun_data.label_name=instruction;

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"strval"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=1;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionStrVal"); 
    htab_find(symtable,"strval")->data.fun_data.label_name=instruction;

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"strlen"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=1;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionStrlen"); 
    htab_find(symtable,"strlen")->data.fun_data.label_name=instruction;
    st_fun_param_type(htab_find(symtable,"strlen")->data.fun_data.TaV,T_STRING_TYPE);
    st_fun_param_name(htab_find(symtable,"strlen")->data.fun_data.TaV,"$$strlen");

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"ord"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=1;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionOrd"); 
    htab_find(symtable,"ord")->data.fun_data.label_name=instruction;
    st_fun_param_type(htab_find(symtable,"ord")->data.fun_data.TaV,T_STRING_TYPE);
    st_fun_param_name(htab_find(symtable,"ord")->data.fun_data.TaV,"$$ord");

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"chr"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=1;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionChr"); 
    htab_find(symtable,"chr")->data.fun_data.label_name=instruction;
    st_fun_param_type(htab_find(symtable,"chr")->data.fun_data.TaV,T_INT_TYPE);
    st_fun_param_name(htab_find(symtable,"chr")->data.fun_data.TaV,"$$chr");

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"substring"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=3;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionSubStr"); 
    htab_find(symtable,"substring")->data.fun_data.label_name=instruction;
    st_fun_param_type(htab_find(symtable,"substring")->data.fun_data.TaV,T_STRING_TYPE);
    st_fun_param_name(htab_find(symtable,"substring")->data.fun_data.TaV,"$$substr");
    st_fun_param_type(htab_find(symtable,"substring")->data.fun_data.TaV,T_INT_TYPE);
    st_fun_param_name(htab_find(symtable,"substring")->data.fun_data.TaV,"$$substrint1");
    st_fun_param_type(htab_find(symtable,"substring")->data.fun_data.TaV,T_INT_TYPE);
    st_fun_param_name(htab_find(symtable,"substring")->data.fun_data.TaV,"$$substrint2");
    //generation of inbuild functions
    generate_write(instruction,&instruction_list);
    generate_readi(instruction,&instruction_list);
    generate_reads(instruction,&instruction_list);
    generate_readf(instruction,&instruction_list);
    generate_floatval(instruction,&instruction_list);
    generate_intval(instruction,&instruction_list);
    generate_strval(instruction,&instruction_list);
    generate_strlen(instruction,&instruction_list);
    generate_ord(instruction,&instruction_list);
    generate_chr(instruction,&instruction_list);
    generate_substr(instruction,&instruction_list);
    //calling first rule
    if (f_start(&token,instruction,&instruction_list) == false){
        exit(SYNTAX_ERROR);
    }
    //adding exit 0 to ifjcode
    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@0");
    DLL_InsertAfter_main(&instruction_list,instruction);
    dynamicBufferFREE(instruction);
    //dispose and free structures
    print_instructions(&instruction_list);
    DLL_Dispose(&instruction_list);
    return 0;
}
