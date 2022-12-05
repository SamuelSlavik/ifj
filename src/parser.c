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
#include "expression_codegen.h"
#include "generator.h"

#define ERROR_EXIT(flag,token,errcode) if (!(flag)){error_exit(token,errcode);}
#define DETECT_MAIN(instruction_list,instruction,key) if(!strcmp(key,"$$main")){DLL_InsertAfter_main(instruction_list,instruction);\
if(instruction_list->active==instruction_list->main_body)\
{DLL_Next(instruction_list);}\
DLL_Next_main(instruction_list);}\
else{DLL_InsertAfter(instruction_list,instruction); DLL_Next(instruction_list);}
tStack buildinargs;
htab_t *local_sym;
extern htab_t *symtable;


void print_stack(tStack *expr_stack, tDynamicBuffer *instruction, DLList *instruction_list,char *code){
    // PRINT STACK
    tStack print_stack;
    StackInit(&print_stack);
    while (!StackIsEmpty(expr_stack)){
        tVar_TaV *stack_top_itm = ((tVar_TaV*) StackTop(expr_stack));
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, code);
        dynamicBuffer_ADD_STRING(instruction, stack_top_itm->var);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
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
        error_exit(token,LEX_ERROR); //todo error.c
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
        error_exit(token,LEX_ERROR);
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
    prog = (f_func(token,instruction, instruction_list) && f_prog(token,instruction, instruction_list)) || 
            (f_body(token,instruction, instruction_list) && f_prog(token,instruction, instruction_list)); //<prog> => <fn> <prog>
    //<prog> => <body> <prog>
    return prog;
}
bool f_body(tToken *token, tDynamicBuffer *instruction, DLList *instruction_list){
    bool body = false;
    if (token->type == T_ERROR){
        error_exit(token,LEX_ERROR);
    }
    tDynamicBuffer *labelname;
    tDynamicBuffer *labelnameif;
    tToken end_token;
    tToken tmp_token;
    switch (token->type)
    {
    case T_VAR_ID:
        if (htab_find(instruction_list->called_from->data.fun_data.localST,token->data.STRINGval->data) == NULL ){
            instruction = dynamicBuffer_INIT();
            dynamicBuffer_ADD_STRING(instruction, "DEFVAR ");
            dynamicBuffer_ADD_STRING(instruction, "LF@");
            dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
            if (instruction_list->if_while != NULL){
                DLL_InsertBefore_if_while(instruction_list,instruction);
            }
            else{
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            }
            dynamicBufferFREE(instruction);
        }
        instruction_list->curr_var=st_var_create(instruction_list->called_from->data.fun_data.localST, token->data.STRINGval->data);
        *token = get_token(1);
        
        if (token->type == T_ASSIGN){
            *token = get_token(1);
            body = f_body_var(token,instruction, instruction_list);
            ERROR_EXIT(body,token,SYNTAX_ERROR);
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
            tmp_token.data.STRINGval= dynamicBuffer_INIT();
            dynamicBuffer_ADD_STRING(tmp_token.data.STRINGval,instruction_list->curr_var->key);
            body = check_expr_syntax(token, &end_token,instruction_list, &tmp_token);
            dynamicBufferFREE(tmp_token.data.STRINGval);
            ERROR_EXIT(body,token,SYNTAX_ERROR);
            *token = get_token(1);
        }
        break;
    case T_FUN_ID:
        //local_sym = st_fun_call(symtable,&frames,token->data.STRINGval->data);
        //opravit pre inbuild funkcie
        if (htab_find(symtable,token->data.STRINGval->data) == NULL ){
            //ERROR_EXIT(htab_find(symtable,token->data.STRINGval->data) == NULL,token,RE_DEF_ERROR); da sa to aj takto menit
            error_exit(token,RE_DEF_ERROR);
        }
        instruction_list->curr_fun=htab_find(symtable, token->data.STRINGval->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            instruction = dynamicBuffer_INIT();
            dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME");
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "DEFVAR TF@");
        }
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            instruction_list->num_of_params_called=0;
            body = f_fn_call_l(token,instruction, instruction_list);
            ERROR_EXIT(body,token,SYNTAX_ERROR);

            ERROR_EXIT(token->type == T_SEMICOLON,token,SYNTAX_ERROR); // lepsia variantta
            
            /*if (token->type != T_SEMICOLON){
                ERROR_EXIT(0,token,SYNTAX_ERROR); // lol
            }*/
            instruction = dynamicBuffer_INIT();
            if (!strcmp(instruction_list->curr_fun->key,"substring")){
                while (!StackIsEmpty(&buildinargs)){
                    tDynamicBuffer *stack_top_itm = ((tDynamicBuffer *) StackTop(&buildinargs));
                    DETECT_MAIN(instruction_list,stack_top_itm,instruction_list->called_from->key);
                    StackPop(&buildinargs);
                    dynamicBufferFREE(stack_top_itm);
                }
                dynamicBuffer_ADD_STRING(instruction, "CALL ");
                dynamicBuffer_ADD_STRING(instruction, instruction_list->curr_fun->data.fun_data.label_name->data);
            }
            else if (!strcmp(instruction_list->curr_fun->key,"write")){
                while (!StackIsEmpty(&buildinargs)){
                    tDynamicBuffer *stack_top_itm = ((tDynamicBuffer *) StackTop(&buildinargs));
                    DETECT_MAIN(instruction_list,stack_top_itm,instruction_list->called_from->key);
                    StackPop(&buildinargs);
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
                dynamicBuffer_ADD_STRING(instruction, "CALL ");
                dynamicBuffer_ADD_STRING(instruction, instruction_list->curr_fun->data.fun_data.label_name->data);
            }
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
            *token = get_token(1);
        }
        break;
    case T_RETURN:
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
        ERROR_EXIT(body,token,SYNTAX_ERROR);
        tDynamicBuffer *return_type_false = label_name_gen("return_type_false");
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME\n");
        dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME\n");
        dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res\n");
        dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res_type\n");
        dynamicBuffer_ADD_STRING(instruction, "POPS TF@res\n");
        dynamicBuffer_ADD_STRING(instruction, "TYPE TF@res_type TF@res\n");
        dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
        
        dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
        dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
        dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@");
        switch(instruction_list->called_from->data.fun_data.return_type){
            case T_STRING_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "string\n");
                break;
            case T_FLOAT_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "float\n");
                break;
            case T_INT_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "int\n");
                break;
            case T_STRING_N_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "string\n");
                dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
                dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
                dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@nil\n");
                break;
            case T_FLOAT_N_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "float\n");
                dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
                dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
                dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@nil\n");
                break;
            case T_INT_N_TYPE:
                dynamicBuffer_ADD_STRING(instruction, "int\n");
                dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
                dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
                dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@nil\n");
                break;
            default:
                dynamicBuffer_ADD_STRING(instruction, "\n");
                dynamicBuffer_ADD_STRING(instruction, "JUMP ");
                dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
                dynamicBuffer_ADD_STRING(instruction, "\n");
                break;

        }
        dynamicBuffer_ADD_STRING(instruction, "EXIT int@4\n");
        dynamicBuffer_ADD_STRING(instruction, "LABEL ");
        dynamicBuffer_ADD_STRING(instruction, return_type_false->data);
        dynamicBuffer_ADD_STRING(instruction, "\n");

        dynamicBuffer_ADD_STRING(instruction, "POPFRAME");
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        dynamicBufferFREE(return_type_false);

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
        labelnameif = label_name_gen(token->data.STRINGval->data);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "LABEL if_");
        dynamicBuffer_ADD_STRING(instruction, labelnameif->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        if (instruction_list->if_while == NULL){
            instruction_list->label = labelnameif->data;
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
            ERROR_EXIT(body,token,SYNTAX_ERROR);
            instruction = dynamicBuffer_INIT();

            tDynamicBuffer *expr_set_false = label_name_gen("expr_set_false");
            tDynamicBuffer *expr_set_true = label_name_gen("expr_set_true");
            tDynamicBuffer *expr_res_int2bool = label_name_gen("expr_res_int2bool");
            tDynamicBuffer *expr_res_float2bool = label_name_gen("expr_res_float2bool");
            tDynamicBuffer *expr_res_string2bool = label_name_gen("expr_res_string2bool");
            tDynamicBuffer *expr_res_eval = label_name_gen("expr_res_eval");
            
            dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME\n");
            dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME\n");
            dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res_type\n");
            dynamicBuffer_ADD_STRING(instruction, "POPS TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "TYPE TF@res_type TF@res\n");

            // IF RESULT IS NULL
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@nil\n");

            // IF RESULT IS INT
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_int2bool->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@int\n");

            // IF RESULT IS FLOAT
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_float2bool->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@float\n");

            // IF RESULT IS STRING
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_string2bool->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@string\n");

            // IF RESULT IS BOOL
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@bool\n");

            // UNKNOWN TYPE
            dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");
            
            // INT TO BOOL
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_int2bool->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res int@0\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");

            // FLOAT TO BOOL
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_float2bool->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res float@0x0p+0\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");

            // STRING TO BOOL
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_string2bool->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res string@\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");

            // SET RESULT AS FALSE
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "MOVE TF@res bool@false\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            
            // SET RESULT AS TRUE
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "MOVE TF@res bool@true\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            
            // EVALUATE RESULT
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "PUSHS bool@true\n");
            dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFNEQS false_");
            dynamicBuffer_ADD_STRING(instruction, labelnameif->data);
            // cond jump 

            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(expr_set_false);
            dynamicBufferFREE(expr_set_true);
            dynamicBufferFREE(expr_res_int2bool);
            dynamicBufferFREE(expr_res_float2bool);
            dynamicBufferFREE(expr_res_string2bool);
            dynamicBufferFREE(expr_res_eval);
            dynamicBufferFREE(instruction);
            
            *token = get_token(1);
            if (token->type == T_L_BRAC){
                *token = get_token(1);
                body = f_in_body(token,instruction, instruction_list);
                ERROR_EXIT(body,token,SYNTAX_ERROR);            
                instruction = dynamicBuffer_INIT();
                dynamicBuffer_ADD_STRING(instruction, "JUMP end_");
                dynamicBuffer_ADD_STRING(instruction, labelnameif->data);
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                if (token->type == T_ELSE){
                    *token = get_token(1);
                    if(token->type == T_L_BRAC){
                        instruction = dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction, "LABEL false_");
                        dynamicBuffer_ADD_STRING(instruction, labelnameif->data);
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                        *token = get_token(1);
                        body = f_in_body(token,instruction, instruction_list);
                        ERROR_EXIT(body,token,SYNTAX_ERROR);
                        
                        instruction = dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction, "LABEL end_");
                        dynamicBuffer_ADD_STRING(instruction, labelnameif->data);
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                        if (!strcmp(instruction_list->label,labelnameif->data)){
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
        labelname = label_name_gen(token->data.STRINGval->data);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "LABEL while_");
        dynamicBuffer_ADD_STRING(instruction, labelname->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        if (instruction_list->if_while == NULL){
                instruction_list->label = labelname->data;
                if (!strcmp(instruction_list->called_from->key,"$$main")){
                    instruction_list->if_while=instruction_list->main_body;
                }
                else{
                    instruction_list->if_while=instruction_list->active;
                }
            }
        dynamicBufferFREE(instruction);
        *token = get_token(1);
        
        if(token->type == T_L_PAR){
            *token = get_token(1);
            end_token.type = T_R_PAR;
            body = check_expr_syntax(token, &end_token,instruction_list,NULL);
            ERROR_EXIT(body,token,SYNTAX_ERROR);
            instruction = dynamicBuffer_INIT();

            tDynamicBuffer *expr_set_false = label_name_gen("expr_set_false");
            tDynamicBuffer *expr_set_true = label_name_gen("expr_set_true");
            tDynamicBuffer *expr_res_int2bool = label_name_gen("expr_res_int2bool");
            tDynamicBuffer *expr_res_float2bool = label_name_gen("expr_res_float2bool");
            tDynamicBuffer *expr_res_string2bool = label_name_gen("expr_res_string2bool");
            tDynamicBuffer *expr_res_eval = label_name_gen("expr_res_eval");
            
            dynamicBuffer_ADD_STRING(instruction, "PUSHFRAME\n");
            dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME\n");
            dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "DEFVAR TF@res_type\n");
            dynamicBuffer_ADD_STRING(instruction, "POPS TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "TYPE TF@res_type TF@res\n");

            // IF RESULT IS NULL
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@nil\n");

            // IF RESULT IS INT
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_int2bool->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@int\n");

            // IF RESULT IS FLOAT
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_float2bool->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@float\n");

            // IF RESULT IS STRING
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_string2bool->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@string\n");

            // IF RESULT IS BOOL
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res_type string@bool\n");

            // UNKNOWN TYPE
            dynamicBuffer_ADD_STRING(instruction, "EXIT int@7\n");
            
            // INT TO BOOL
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_int2bool->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res int@0\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");

            // FLOAT TO BOOL
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_float2bool->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res float@0x0p+0\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");

            // STRING TO BOOL
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_string2bool->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFEQ ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, " TF@res string@\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");

            // SET RESULT AS FALSE
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_false->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "MOVE TF@res bool@false\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            
            // SET RESULT AS TRUE
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_set_true->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "MOVE TF@res bool@true\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMP ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            
            // EVALUATE RESULT
            dynamicBuffer_ADD_STRING(instruction, "LABEL ");
            dynamicBuffer_ADD_STRING(instruction, expr_res_eval->data);
            dynamicBuffer_ADD_STRING(instruction, "\n");
            dynamicBuffer_ADD_STRING(instruction, "PUSHS TF@res\n");
            dynamicBuffer_ADD_STRING(instruction, "PUSHS bool@true\n");
            dynamicBuffer_ADD_STRING(instruction, "POPFRAME\n");
            dynamicBuffer_ADD_STRING(instruction, "JUMPIFNEQS false_");
            dynamicBuffer_ADD_STRING(instruction, labelname->data);
            // cond jump 

            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(expr_set_false);
            dynamicBufferFREE(expr_set_true);
            dynamicBufferFREE(expr_res_int2bool);
            dynamicBufferFREE(expr_res_float2bool);
            dynamicBufferFREE(expr_res_string2bool);
            dynamicBufferFREE(expr_res_eval);
            dynamicBufferFREE(instruction);
            *token = get_token(1);
            if (token->type == T_L_BRAC){                
                *token = get_token(1);
                body = f_in_body(token,instruction, instruction_list);
                ERROR_EXIT(body,token,SYNTAX_ERROR);
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
                //poriesit pre vnorene loopy
                if (!strcmp(instruction_list->label,labelname->data)){
                    instruction_list->if_while=NULL; 
                }          
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
        error_exit(token,LEX_ERROR);
    }
    if(token->type == T_FUN_ID){
        //local_sym = st_fun_call(symtable,&frames,token->data.STRINGval->data);    
        //opravit pre inbuild funkcie
        if (htab_find(symtable,token->data.STRINGval->data) == NULL ){
            //ERROR_EXIT(htab_find(symtable,token->data.STRINGval->data) == NULL,token,RE_DEF_ERROR); da sa to aj takto menit
            error_exit(token,RE_DEF_ERROR);
        }
        instruction_list->curr_fun=htab_find(symtable, token->data.STRINGval->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            instruction = dynamicBuffer_INIT();
            dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME");
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "DEFVAR TF@");
        }
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            instruction_list->num_of_params_called=0;
            body_var = f_fn_call_l(token,instruction, instruction_list);
            ERROR_EXIT(body_var,token,SYNTAX_ERROR);

            ERROR_EXIT(token->type == T_SEMICOLON,token,SYNTAX_ERROR); // lepsia variantta
            
            /*if (token->type != T_SEMICOLON){
                ERROR_EXIT(0,token,SYNTAX_ERROR); // lol
            }*/
            instruction = dynamicBuffer_INIT();
            if (!strcmp(instruction_list->curr_fun->key,"substring")){
                while (!StackIsEmpty(&buildinargs)){
                    tDynamicBuffer *stack_top_itm = ((tDynamicBuffer *) StackTop(&buildinargs));
                    DETECT_MAIN(instruction_list,stack_top_itm,instruction_list->called_from->key);
                    StackPop(&buildinargs);
                    dynamicBufferFREE(stack_top_itm);
                }
                dynamicBuffer_ADD_STRING(instruction, "CALL ");
                dynamicBuffer_ADD_STRING(instruction, instruction_list->curr_fun->data.fun_data.label_name->data);
            }
            else if (!strcmp(instruction_list->curr_fun->key,"write")){
                while (!StackIsEmpty(&buildinargs)){
                    tDynamicBuffer *stack_top_itm = ((tDynamicBuffer *) StackTop(&buildinargs));
                    DETECT_MAIN(instruction_list,stack_top_itm,instruction_list->called_from->key);
                    StackPop(&buildinargs);
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
        //mov a token.data.int cislo
        body_var = check_expr_syntax(token, &end_token,instruction_list,NULL);
        ERROR_EXIT(body_var,token,SYNTAX_ERROR);
        *token = get_token(1);
        
    }
    return body_var;
}

bool f_body_ret(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    (void)instruction;
    bool body_ret = false;
    if (token->type == T_ERROR){
        error_exit(token,LEX_ERROR);
    }
    else if (token->type == T_SEMICOLON){
        if (instruction_list->called_from->data.fun_data.return_type != T_VOID){
            printf("%d",instruction_list->called_from->data.fun_data.return_type);
            error_exit(token,RETURN_ERROR);
        }
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
        ERROR_EXIT(body_ret,token,SYNTAX_ERROR);
        *token = get_token(1);
    }
    return body_ret;
    
}

bool f_fn_call_l(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool fn_call_bool = false;
    if (token->type == T_ERROR){
        error_exit(token,LEX_ERROR);
    }
    switch (token->type)
    {
    case T_STRING:
        instruction_list->num_of_params_called++;
        
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "string@");
        string_to_ifj_fmt(&token->data.STRINGval);
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
        }
        else{
            StackPush(&buildinargs,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_INT:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "int@");
        dynamicBuffer_ADD_STRING(instruction, long_2_string(token->data.INTval)->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
        }
        else{
            StackPush(&buildinargs,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_FLOAT:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "float@");
        dynamicBuffer_ADD_STRING(instruction, double_2_string(token->data.FLOATval)->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
        }
        else{
            StackPush(&buildinargs,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_VAR_ID:
        instruction_list->num_of_params_called++;
        if (htab_find(instruction_list->called_from->data.fun_data.localST,token->data.STRINGval->data)==NULL){
            error_exit(token,UN_DEF_VAR_ERROR);
        }
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS LF@");
        // pridat radmec dynamicBuffer_ADD_STRING(instruction, "");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
        }
        else{
            StackPush(&buildinargs,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_R_PAR:
    if(strcmp(instruction_list->curr_fun->key,"write")){
        if(instruction_list->num_of_params_called != instruction_list->curr_fun->data.fun_data.number_of_params){
            error_exit(token,PARAM_ERROR);
        }
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
    if (token->type == T_ERROR){
        error_exit(token,LEX_ERROR);
    }
    switch (token->type)
    {
    case T_COMMA:
        *token = get_token(1);
        fn_call_bool2 = f_fn_call_lparam(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool2,token,SYNTAX_ERROR);
        break;
    case T_R_PAR:
        if(strcmp(instruction_list->curr_fun->key,"write")){
        if(instruction_list->num_of_params_called != instruction_list->curr_fun->data.fun_data.number_of_params){
            error_exit(token,PARAM_ERROR);
        }
        print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "POPS TF@");
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
    if (token->type == T_ERROR){
        error_exit(token,LEX_ERROR);
    }
    switch (token->type)
    {
    case T_STRING:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "string@");
        string_to_ifj_fmt(&token->data.STRINGval);
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
        }
        else{
            StackPush(&buildinargs,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_INT:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "int@");
        dynamicBuffer_ADD_STRING(instruction, long_2_string(token->data.INTval)->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
        }
        else{
            StackPush(&buildinargs,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_FLOAT:
        instruction_list->num_of_params_called++;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "float@");
        dynamicBuffer_ADD_STRING(instruction, double_2_string(token->data.FLOATval)->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
        }
        else{
            StackPush(&buildinargs,instruction);
        }
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_VAR_ID:
        instruction_list->num_of_params_called++;
        if (htab_find(instruction_list->called_from->data.fun_data.localST,token->data.STRINGval->data)==NULL){
            error_exit(token,UN_DEF_VAR_ERROR);
        }
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS LF@");
        // pridat radmec dynamicBuffer_ADD_STRING(instruction, "");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        if (strcmp(instruction_list->curr_fun->key,"write") && strcmp(instruction_list->curr_fun->key,"substring")){
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
        }
        else{
            StackPush(&buildinargs,instruction);
        }
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
        error_exit(token,LEX_ERROR);
    }
    if(token->type == T_FUNCTION){
        *token = get_token(1);
        if(token->type == T_FUN_ID){        
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
            //TODO IFJCODE
            }
            else{
                error_exit(token,RE_DEF_ERROR);
            }
            *token = get_token(1);
            if (token->type == T_L_PAR){
                *token = get_token(1);
                func = f_func_param(token,instruction, instruction_list);
                ERROR_EXIT(func,token,SYNTAX_ERROR);
                if(token->type == T_COLON){
                    *token = get_token(1);
                    func = f_func_type(token,instruction, instruction_list) && f_func_dedf(token,instruction, instruction_list);
                    ERROR_EXIT(func,token,SYNTAX_ERROR); 
                    if (instruction_list->called_from->data.fun_data.has_ret == false && instruction_list->called_from->data.fun_data.return_type !=T_VOID){
                        error_exit(token,RETURN_ERROR);
                    }
                    else if(instruction_list->called_from->data.fun_data.has_ret == false && instruction_list->called_from->data.fun_data.return_type ==T_VOID){
                        instruction=dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction,"PUSHS nil@nil");
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);

                    }
                    instruction = dynamicBuffer_INIT();
                    dynamicBuffer_ADD_STRING(instruction, "POPFRAME");
                    DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                    dynamicBufferFREE(instruction);
                    instruction = dynamicBuffer_INIT();
                    dynamicBuffer_ADD_STRING(instruction, "RETURN");
                    DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                    dynamicBufferFREE(instruction);

                    instruction_list->called_from = htab_find(symtable, "$$main");
                }
                //TODO SKONTROLOVAT MAGIA
            }
        }
    }
    return func;
}
bool f_func_dedf(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    bool func_dedf = false;
    if (token->type == T_ERROR){
        error_exit(token,LEX_ERROR);
    }
    if(token->type == T_L_BRAC){
        *token = get_token(1);
        func_dedf = f_in_body(token,instruction, instruction_list);
        ERROR_EXIT(func_dedf,token,SYNTAX_ERROR);
    }
    return func_dedf;
}

bool f_func_type(tToken *token,tDynamicBuffer *instruction, DLList *instruction_list){
    (void)instruction;
    bool func_type = false;
    if (token->type == T_ERROR){
        error_exit(token,LEX_ERROR);
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
    if (token->type == T_ERROR){
        error_exit(token,LEX_ERROR);
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
        error_exit(token,LEX_ERROR);
    }
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
        st_fun_param_name(instruction_list->curr_fun->data.fun_data.TaV,token->data.STRINGval->data);
        st_var_create(instruction_list->curr_fun->data.fun_data.localST,token->data.STRINGval->data);
        instruction_list->curr_fun->data.fun_data.number_of_params++;
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
            st_fun_param_type(instruction_list->curr_fun->data.fun_data.TaV,token->type);
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
    StackInit(&buildinargs);
    dynamicBuffer_ADD_STRING(instruction,".IFJcode22");
    DLL_Init(&instruction_list);
    DLL_InsertFirst(&instruction_list, instruction);
    dynamicBufferFREE(instruction);
    DLL_First_main(&instruction_list);
    DLL_First(&instruction_list);
    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"$$main");
    instruction_list.curr_fun = st_fun_create(symtable,instruction->data);
    instruction_list.called_from = htab_find(symtable, "$$main");
    st_fun_retrun_type(instruction_list.curr_fun,T_VOID);
    st_fun_table_create(symtable,(char *)instruction_list.curr_fun->key);

    dynamicBufferFREE(instruction);
    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME");
    DLL_InsertAfter(&instruction_list,instruction);
    DLL_Next(&instruction_list);
    DLL_Next_main(&instruction_list);
    dynamicBufferFREE(instruction);
    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"PUSHFRAME");
    DLL_InsertAfter(&instruction_list,instruction);
    DLL_Next(&instruction_list);
    DLL_Next_main(&instruction_list);
    dynamicBufferFREE(instruction);
    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"CREATEFRAME");
    DLL_InsertAfter(&instruction_list,instruction);
    DLL_Next(&instruction_list);
    DLL_Next_main(&instruction_list);
    dynamicBufferFREE(instruction);
    tToken token = get_token(0);
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

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"ord"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=1;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionOrd"); 
    htab_find(symtable,"ord")->data.fun_data.label_name=instruction;

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"chr"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=1;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionChr"); 
    htab_find(symtable,"chr")->data.fun_data.label_name=instruction;

    instruction = dynamicBuffer_INIT();    
    dynamicBuffer_ADD_STRING(instruction,"substring"); 
    st_fun_create(symtable,instruction->data)->data.fun_data.number_of_params=3;
    dynamicBufferFREE(instruction);
    instruction=dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction,"functionSubStr"); 
    htab_find(symtable,"substring")->data.fun_data.label_name=instruction;

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
    
    if (f_start(&token,instruction,&instruction_list) == false){ //debug error print
        printf("PREBUBLALO TO AZ DO MAINU SKONTROLOVAT\n");
        exit(SYNTAX_ERROR);
    }
    instruction = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(instruction, "EXIT int@0");
    DLL_InsertAfter_main(&instruction_list,instruction);
    dynamicBufferFREE(instruction);
    print_instructions(&instruction_list);
    return 0;
}
