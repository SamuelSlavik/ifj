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
#define DETECT_MAIN(instruction_list,instruction,key) if(!strcmp(key,"$$main")){DLL_InsertAfter_main(instruction_list,instruction);\
if(instruction_list->active==instruction_list->main_body)\
{DLL_Next(instruction_list);}\
DLL_Next_main(instruction_list);}\
else{DLL_InsertAfter(instruction_list,instruction); DLL_Next(instruction_list);}
tStack frames;
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
    prog = (f_func(token,instruction, instruction_list) && f_prog(token,instruction, instruction_list)) || 
            (f_body(token,instruction, instruction_list) && f_prog(token,instruction, instruction_list)); //<prog> => <fn> <prog>
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
        instruction_list->curr_var=st_var_create(symtable, token->data.STRINGval->data);
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
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME");
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "DEFVAR TF@");
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            
            body = f_fn_call_l(token,instruction, instruction_list);
            ERROR_EXIT(body,token,SYNTAX_ERROR);

            ERROR_EXIT(token->type == T_SEMICOLON,token,SYNTAX_ERROR); // lepsia variantta
            
            /*if (token->type != T_SEMICOLON){
                ERROR_EXIT(0,token,SYNTAX_ERROR); // lol
            }*/
            instruction = dynamicBuffer_INIT();
            dynamicBuffer_ADD_STRING(instruction, "CALL ");
            dynamicBuffer_ADD_STRING(instruction, instruction_list->curr_fun->data.fun_data.label_name->data);
            DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
            dynamicBufferFREE(instruction);
            
            *token = get_token(1);
        }
        break;
    case T_RETURN:
        *token = get_token(1);
        body = f_body_ret(token,instruction, instruction_list);
        ERROR_EXIT(body,token,SYNTAX_ERROR);
    break;
    case T_IF:
        char *labelnameif = label_name_gen(token->data.STRINGval->data)->data;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "LABEL if_");
        dynamicBuffer_ADD_STRING(instruction, labelnameif);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        if (instruction_list->if_while == NULL){
            instruction_list->label = labelnameif;
        DLL_Set_if_while(instruction_list);
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
            dynamicBuffer_ADD_STRING(instruction, labelnameif);
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
                dynamicBuffer_ADD_STRING(instruction, labelnameif);
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                if (token->type == T_ELSE){
                    *token = get_token(1);
                    if(token->type == T_L_BRAC){
                        instruction = dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction, "LABEL false_");
                        dynamicBuffer_ADD_STRING(instruction, labelnameif);
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                        *token = get_token(1);
                        body = f_in_body(token,instruction, instruction_list);
                        ERROR_EXIT(body,token,SYNTAX_ERROR);
                        
                        instruction = dynamicBuffer_INIT();
                        dynamicBuffer_ADD_STRING(instruction, "LABEL end_");
                        dynamicBuffer_ADD_STRING(instruction, labelnameif);
                        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                        dynamicBufferFREE(instruction);
                        if (!strcmp(instruction_list->label,labelnameif)){
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
        char *labelname = label_name_gen(token->data.STRINGval->data)->data;
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "LABEL while_");
        dynamicBuffer_ADD_STRING(instruction, labelname);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        if (instruction_list->if_while == NULL){
            instruction_list->label = labelname;
        DLL_Set_if_while(instruction_list);
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
            dynamicBuffer_ADD_STRING(instruction, labelnameif);
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
                dynamicBuffer_ADD_STRING(instruction, labelname);
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                instruction = dynamicBuffer_INIT();
                dynamicBuffer_ADD_STRING(instruction, "LABEL false_");
                dynamicBuffer_ADD_STRING(instruction, labelname);
                DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
                dynamicBufferFREE(instruction);
                //poriesit pre vnorene loopy
                if (!strcmp(instruction_list->label,labelname)){
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
        exit(1);
    }
    if(token->type == T_FUN_ID){
        //local_sym = st_fun_call(symtable,&frames,token->data.STRINGval->data);    
        //opravit pre inbuild funkcie
        if (htab_find(symtable,token->data.STRINGval->data) == NULL ){
            //ERROR_EXIT(htab_find(symtable,token->data.STRINGval->data) == NULL,token,RE_DEF_ERROR); da sa to aj takto menit
            error_exit(token,RE_DEF_ERROR);
        }
        instruction_list->curr_fun=htab_find(symtable, token->data.STRINGval->data);
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "CREATEFRAME");
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "DEFVAR TF@");
        *token = get_token(1);
        if (token->type == T_L_PAR){
            *token = get_token(1);
            
            body_var = f_fn_call_l(token,instruction, instruction_list);
            ERROR_EXIT(body_var,token,SYNTAX_ERROR);

            ERROR_EXIT(token->type == T_SEMICOLON,token,SYNTAX_ERROR); // lepsia variantta
            
            /*if (token->type != T_SEMICOLON){
                ERROR_EXIT(0,token,SYNTAX_ERROR); // lol
            }*/
            instruction = dynamicBuffer_INIT();
            dynamicBuffer_ADD_STRING(instruction, "CALL ");
            dynamicBuffer_ADD_STRING(instruction, instruction_list->curr_fun->data.fun_data.label_name->data);
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
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "string@");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_INT:
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "int@");
        dynamicBuffer_ADD_STRING(instruction, long_2_string(token->data.INTval)->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_FLOAT:
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "float@");
        dynamicBuffer_ADD_STRING(instruction, double_2_string(token->data.FLOATval)->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_VAR_ID:
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        // pridat radmec dynamicBuffer_ADD_STRING(instruction, "");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
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
        print_stack(instruction_list->curr_fun->data.fun_data.TaV,instruction,instruction_list, "POPS TF@");
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
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "string@");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_INT:
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "int@");
        dynamicBuffer_ADD_STRING(instruction, long_2_string(token->data.INTval)->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_NUM_FLOAT:
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        dynamicBuffer_ADD_STRING(instruction, "float@");
        dynamicBuffer_ADD_STRING(instruction, double_2_string(token->data.FLOATval)->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
        *token = get_token(1);
        fn_call_bool = f_fn_call_lc(token,instruction, instruction_list);
        ERROR_EXIT(fn_call_bool,token,SYNTAX_ERROR);
        break;
    case T_VAR_ID:
        instruction = dynamicBuffer_INIT();
        dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
        // pridat radmec dynamicBuffer_ADD_STRING(instruction, "");
        dynamicBuffer_ADD_STRING(instruction, token->data.STRINGval->data);
        DETECT_MAIN(instruction_list,instruction,instruction_list->called_from->key);
        dynamicBufferFREE(instruction);
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
            //TODO IFJCODE
            }
            instruction_list->curr_fun=st_fun_create(symtable, token->data.STRINGval->data);
            instruction_list->called_from=st_fun_create(symtable, token->data.STRINGval->data);
            *token = get_token(1);
            if (token->type == T_L_PAR){
                *token = get_token(1);
                func = f_func_param(token,instruction, instruction_list);
                ERROR_EXIT(func,token,SYNTAX_ERROR);
                if(token->type == T_COLON){
                    *token = get_token(1);
                    func = f_func_type(token,instruction, instruction_list) && f_func_dedf(token,instruction, instruction_list);
                    ERROR_EXIT(func,token,SYNTAX_ERROR); 
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
    local_sym = htab_init(HTAB_SIZE);
    StackPush(&frames,(void*)local_sym);
    
    st_fun_definition(instruction_list->curr_fun);
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
    (void)instruction;
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
    StackInit(&frames);
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
    if (f_start(&token,instruction,&instruction_list) == false){ //debug error print
        printf("PREBUBLALO TO AZ DO MAINU SKONTROLOVAT\n");
        exit(SYNTAX_ERROR);
    }
    print_instructions(&instruction_list);
    return 0;
}
