/**
 * @file expression_parser.c
 * @author Adam Pekný (xpekny00)
 * @brief
 * @version 0.1
 * @date 2022-11-18
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "expression_parser.h"
#include "stack.h"
#include "scanner.h"
#include "htab.h"
#include "dll_instruction_list.h"
#include "expression_codegen.h"
#include "error.h"

extern htab_t *symtable;

void string_to_ifj_fmt(tDynamicBuffer **string){
    tDynamicBuffer *tmp_copy = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(tmp_copy, (*string)->data);
    *string = dynamicBuffer_RESET(*string);

    size_t len = tmp_copy->size + 1;
    int current_char;

    for (size_t i = 0; i < len; ++i) {
        current_char = (unsigned char) tmp_copy->data[i];

        if (current_char == ' '){
            dynamicBuffer_ADD_STRING(*string, "\\032");
        } else if (current_char == '#'){
            dynamicBuffer_ADD_STRING(*string, "\\035");
        } else if (current_char == '$'){
            // TODO add token to error exit somehow
            exit(1);
        } else if (current_char == '\\'){
            current_char = (unsigned char) tmp_copy->data[++i];

            if (current_char == '\\'){
                dynamicBuffer_ADD_STRING(*string, "\\092");
            } else if(current_char == 't'){
                dynamicBuffer_ADD_STRING(*string, "\\009");
            } else if(current_char == 'n'){
                dynamicBuffer_ADD_STRING(*string, "\\010");
            } else if(current_char == '"'){
                dynamicBuffer_ADD_STRING(*string, "\\034");
            } else if(current_char == '$'){
                dynamicBuffer_ADD_STRING(*string, "\\036");
            } else if(current_char == 'x'){
                tDynamicBuffer *hex_str = dynamicBuffer_INIT();
                dynamicBuffer_ADD_CHAR(hex_str, tmp_copy->data[i+1]);
                dynamicBuffer_ADD_CHAR(hex_str, tmp_copy->data[i+2]);

                char *hex_end_ptr = NULL;
                long hex_int_val = strtol(hex_str->data, &hex_end_ptr, 16);
                if (*hex_end_ptr != '\0'){
                    dynamicBuffer_ADD_STRING(*string, "\\092x");
                } else {
                    tDynamicBuffer *int_val_str = long_2_string(hex_int_val);
                    if (int_val_str->size < 3){
                        tDynamicBuffer *int_val_str_tmp = int_val_str;
                        int_val_str = dynamicBuffer_INIT();
                        for (size_t j = 0; j < 3 - int_val_str_tmp->size; ++j) {
                            dynamicBuffer_ADD_CHAR(int_val_str, '0');
                        }
                        dynamicBuffer_ADD_STRING(int_val_str, int_val_str_tmp->data);
                        dynamicBufferFREE(int_val_str_tmp);
                    }
                    dynamicBuffer_ADD_STRING(*string, "\\");
                    dynamicBuffer_ADD_STRING(*string, int_val_str->data);
                    dynamicBufferFREE(int_val_str);
                    i += 2;
                }
                dynamicBufferFREE(hex_str);
            } else if(current_char >= '0' && current_char <= '3'){
                tDynamicBuffer *oct_str = dynamicBuffer_INIT();
                dynamicBuffer_ADD_CHAR(oct_str, (char) current_char);
                dynamicBuffer_ADD_CHAR(oct_str, tmp_copy->data[i+1]);
                dynamicBuffer_ADD_CHAR(oct_str, tmp_copy->data[i+2]);

                char *oct_end_ptr = NULL;
                long oct_int_val = strtol(oct_str->data, &oct_end_ptr, 8);
                if (*oct_end_ptr != '\0'){
                    dynamicBuffer_ADD_STRING(*string, "\\092");
                    dynamicBuffer_ADD_CHAR(*string, (char) current_char);
                } else {
                    tDynamicBuffer *int_val_str = long_2_string(oct_int_val);
                    if (int_val_str->size < 3){
                        tDynamicBuffer *int_val_str_tmp = int_val_str;
                        int_val_str = dynamicBuffer_INIT();
                        for (size_t j = 0; j < 3 - int_val_str_tmp->size; ++j) {
                            dynamicBuffer_ADD_CHAR(int_val_str, '0');
                        }
                        dynamicBuffer_ADD_STRING(int_val_str, int_val_str_tmp->data);
                        dynamicBufferFREE(int_val_str_tmp);
                    }
                    dynamicBuffer_ADD_STRING(*string, "\\");
                    dynamicBuffer_ADD_STRING(*string, int_val_str->data);
                    dynamicBufferFREE(int_val_str);
                    i += 2;
                }
                dynamicBufferFREE(oct_str);
            } else {
                dynamicBuffer_ADD_STRING(*string, "\\092");
                dynamicBuffer_ADD_CHAR(*string, (char) current_char);
            }
        } else {
            dynamicBuffer_ADD_CHAR(*string, (char) current_char);
        }
    }

    dynamicBufferFREE(tmp_copy);
}

void print_stack_1(tStack *expr_stack){
    // PRINT STACK
    tStack print_stack;
    StackInit(&print_stack);

    while (!StackIsEmpty(expr_stack)){
        tExprItem *stack_top_itm = ((tExprItem*) StackTop(expr_stack));
        StackPush(&print_stack, stack_top_itm);
        StackPop(expr_stack);
    }
    while (!StackIsEmpty(&print_stack)){
        tExprItem *stack_top_itm = ((tExprItem*) StackTop(&print_stack));
        StackPush(expr_stack, stack_top_itm);
        StackPop(&print_stack);
    }
    // END PRINT STACK
}

tDynamicBuffer *double_2_string(double num){
    tDynamicBuffer *float_str = dynamicBuffer_INIT();
    char buffer[50];
    sprintf(buffer, "%a", num);
    dynamicBuffer_ADD_STRING(float_str, buffer);
    return float_str;
}

tDynamicBuffer *long_2_string(long int num){
    tDynamicBuffer *long_str = dynamicBuffer_INIT();
    long long int digit_count = 1;
    long int number = num;
    while (number != 0){
        number /= 10;
        digit_count++;
    }
    char *buffer = calloc(sizeof(char), digit_count + 1);
    if (buffer == NULL){
        exit(99);
    }
    sprintf(buffer, "%ld", num);
    dynamicBuffer_ADD_STRING(long_str, buffer);
    free(buffer);
    return long_str;
}

bool check_expr_syntax(tToken *start_token, tToken *end_token, DLList *instruction_list, tToken *extra_token){
    PRECED_TAB;
    tStack expr_stack;
    StackInit(&expr_stack);
    tToken *end_token_alloc = (tToken*) malloc(sizeof(tToken));
    if (end_token_alloc == NULL){
        exit(99);
    }
    end_token_alloc->type = end_token->type;
    tExprItem *end_item = (tExprItem*) malloc(sizeof(tExprItem));
    if (end_item == NULL){
        exit(99);
    }
    end_item->token = end_token_alloc;
    end_item->type = T_END_EXPR;
    end_item->is_terminal = true;
    end_item->handle = false;
    StackPush(&expr_stack, end_item);

    tToken *current_token = (tToken*) malloc(sizeof(tToken));
    if (current_token == NULL){
        exit(99);
    }

    tToken tmp_token;
    if (extra_token == NULL){
        tmp_token.type = start_token->type;
        tmp_token.data = start_token->data;
        tmp_token.line = start_token->line;
        start_token = NULL;
    } else {
        tmp_token.type = extra_token->type;
        tmp_token.data = extra_token->data;
        tmp_token.line = extra_token->line;
        extra_token = NULL;
    }
    current_token->type = tmp_token.type;
    current_token->data = tmp_token.data;
    current_token->line = tmp_token.line;
    size_t par_level = 0;

    while (!is_expr_end_token(current_token, end_token, &par_level) || !is_stack_end_state(&expr_stack, end_token)){

        if (current_token->type == T_ERROR){
            error_exit(current_token, 1);
        } else if (current_token->type == T_VAR_ID){
            if (!htab_find(instruction_list->called_from->data.fun_data.localST, current_token->data.STRINGval->data)){
                error_exit(current_token, 5);
            }
        } else if (current_token->type == T_L_PAR){
            par_level++;
        }

        // Get topmost terminal on stack and precedence table indexes
        tExprItem *top_terminal = get_stack_top_terminal(&expr_stack);
        size_t top_terminal_idx = top_terminal->type;
        size_t input_token_idx = token_to_preced_idx(current_token->type, is_expr_end_token(current_token, end_token, &par_level) && top_terminal_idx != T_LPAR_EXPR);
        if (input_token_idx == T_UNKNOW_EXPR){
            printf("err4\n");
            return false;
        }

        if (preced_tab[top_terminal_idx][input_token_idx] == '='){
            // Create new stack item and push it
            tExprItem *new_expr_item = (tExprItem*) malloc(sizeof(tExprItem));
            if (new_expr_item == NULL){
                exit(99);
            }
            new_expr_item->token = current_token;
            new_expr_item->type = token_to_preced_idx(current_token->type, is_expr_end_token(current_token, end_token, &par_level) && top_terminal_idx != T_LPAR_EXPR);
            new_expr_item->is_terminal = true;
            new_expr_item->handle = false;

            StackPush(&expr_stack, new_expr_item);

            // Allocate space for next token and save values
            current_token = (tToken*) malloc(sizeof(tToken));
            if (current_token == NULL){
                exit(99);
            }
            if (start_token == NULL){
                tmp_token = get_token(1);
            } else {
                tmp_token.type = start_token->type;
                tmp_token.data = start_token->data;
                tmp_token.line = start_token->line;
                start_token = NULL;
            }
            current_token->type = tmp_token.type;
            current_token->data = tmp_token.data;
            current_token->line = tmp_token.line;
        } else if (preced_tab[top_terminal_idx][input_token_idx] == '<'){
            top_terminal->handle = true;

            // Create new stack item and push it
            tExprItem *new_expr_item = (tExprItem*) malloc(sizeof(tExprItem));
            if (new_expr_item == NULL){
                exit(99);
            }
            new_expr_item->token = current_token;
            new_expr_item->type = token_to_preced_idx(current_token->type, is_expr_end_token(current_token, end_token, &par_level) && top_terminal_idx != T_LPAR_EXPR);
            new_expr_item->is_terminal = true;
            new_expr_item->handle = false;

            StackPush(&expr_stack, new_expr_item);

            // Allocate space for next token and save values
            current_token = (tToken*) malloc(sizeof(tToken));
            if (current_token == NULL){
                exit(99);
            }
            if (start_token == NULL){
                tmp_token = get_token(1);
            } else {
                tmp_token.type = start_token->type;
                tmp_token.data = start_token->data;
                tmp_token.line = start_token->line;
                start_token = NULL;
            }
            current_token->type = tmp_token.type;
            current_token->data = tmp_token.data;
            current_token->line = tmp_token.line;

        } else if (preced_tab[top_terminal_idx][input_token_idx] == '>'){
            tStack help_stack;
            StackInit(&help_stack);

            // CODE GENERATION START
            tDynamicBuffer *instruction = dynamicBuffer_INIT();
            tDynamicBuffer *num_str;
            switch (top_terminal->type) {
                case T_OPERAND_EXPR:
                    dynamicBuffer_ADD_STRING(instruction, "PUSHS ");
                    switch (top_terminal->token->type) {
                        case T_VAR_ID:
                            var_init_check(instruction_list, top_terminal->token->data.STRINGval);
                            dynamicBuffer_ADD_STRING(instruction, "LF@");
                            dynamicBuffer_ADD_STRING(instruction, top_terminal->token->data.STRINGval->data);
                            break;
                        case T_STRING:
                            string_to_ifj_fmt(&top_terminal->token->data.STRINGval);
                            dynamicBuffer_ADD_STRING(instruction, "string@");
                            dynamicBuffer_ADD_STRING(instruction, top_terminal->token->data.STRINGval->data);
                            break;
                        case T_NUM_INT:
                            num_str = long_2_string(top_terminal->token->data.INTval);
                            dynamicBuffer_ADD_STRING(instruction, "int@");
                            dynamicBuffer_ADD_STRING(instruction, num_str->data);
                            dynamicBufferFREE(num_str);
                            break;
                        case T_NUM_FLOAT:
                            num_str = double_2_string(top_terminal->token->data.FLOATval);
                            dynamicBuffer_ADD_STRING(instruction, "float@");
                            dynamicBuffer_ADD_STRING(instruction, num_str->data);
                            dynamicBufferFREE(num_str);
                            break;
                        case T_NULL:
                            dynamicBuffer_ADD_STRING(instruction, "nil@nil");
                            break;
                        default:
                            break;
                    }

                    insert_instruction(instruction_list, instruction);
                    break;
                case T_MUL_EXPR:
                    gen_muls(instruction_list, instruction);
                    break;
                case T_DIV_EXPR:
                    gen_divs(instruction_list, instruction);
                    break;
                case T_ADD_EXPR:
                    gen_adds(instruction_list, instruction);
                    break;
                case T_SUB_EXPR:
                    gen_subs(instruction_list, instruction);
                    break;
                case T_CONCAT_EXPR:
                    gen_concat(instruction_list, instruction);
                    break;
                case T_LT_EXPR:
                    gen_lts_gts(instruction_list, instruction, "LTS");
                    break;
                case T_GT_EXPR:
                    gen_lts_gts(instruction_list, instruction, "GTS");
                    break;
                case T_LTE_EXPR:
                    gen_ltes(instruction_list, instruction);
                    break;
                case T_GTE_EXPR:
                    gen_gtes(instruction_list, instruction);
                    break;
                case T_EQ_EXPR:
                    gen_eqs(instruction_list, instruction);
                    break;
                case T_NEQ_EXPR:
                    gen_neqs(instruction_list, instruction);
                    break;
                default:
                    break;
            }
            // CODE GENERATION END

            tExprItem *tmp_stack_item = (tExprItem*) StackTop(&expr_stack);

            while (!tmp_stack_item->handle){
                StackPush(&help_stack, tmp_stack_item);
                StackPop(&expr_stack);
                tmp_stack_item = StackTop(&expr_stack);
            }

            tmp_stack_item->handle = false;

            tmp_stack_item = StackTop(&help_stack);

            if (tmp_stack_item->is_terminal == true){
                if (tmp_stack_item->type == T_OPERAND_EXPR){
                    // Create new stack item and push it
                    tExprItem *new_expr_item = (tExprItem*) malloc(sizeof(tExprItem));
                    if (new_expr_item == NULL){
                        exit(99);
                    }
                    new_expr_item->token = NULL;
                    new_expr_item->type = T_OPERAND_EXPR;
                    new_expr_item->is_terminal = false;
                    new_expr_item->handle = false;

                    StackPush(&expr_stack, new_expr_item);

                } else if (tmp_stack_item->type == T_LPAR_EXPR){
                    free_expr_token(&tmp_stack_item->token);
                    free(tmp_stack_item);
                    StackPop(&help_stack);
                    tmp_stack_item = (tExprItem*) StackTop(&help_stack);

                    if (tmp_stack_item != NULL && tmp_stack_item->type == T_OPERAND_EXPR){
                        free_expr_token(&tmp_stack_item->token);
                        free(tmp_stack_item);
                        StackPop(&help_stack);
                        tmp_stack_item = (tExprItem*) StackTop(&help_stack);

                        if (tmp_stack_item != NULL && tmp_stack_item->type == T_RPAR_EXPR){
                            // Create new stack item and push it
                            tExprItem *new_expr_item = (tExprItem*) malloc(sizeof(tExprItem));
                            if (new_expr_item == NULL){
                                exit(99);
                            }
                            new_expr_item->token = NULL;
                            new_expr_item->type = T_OPERAND_EXPR;
                            new_expr_item->is_terminal = false;
                            new_expr_item->handle = false;

                            StackPush(&expr_stack, new_expr_item);
                        } else {
                            if (tmp_stack_item->token != NULL){
                                free(tmp_stack_item->token);
                                tmp_stack_item->token = NULL;
                            }
                            free(tmp_stack_item);
                            printf("err1\n");
                            return false;
                        }
                    } else {
                        if (tmp_stack_item->token != NULL){
                            free(tmp_stack_item->token);
                            tmp_stack_item->token = NULL;
                        }
                        free(tmp_stack_item);
                        printf("err2\n");
                        return false;
                    }
                } else {
                    if (tmp_stack_item->token != NULL){
                        free(tmp_stack_item->token);
                        tmp_stack_item->token = NULL;
                    }
                    free(tmp_stack_item);
                    printf("err3\n");
                    return false;
                }
            } else if (tmp_stack_item->type == T_OPERAND_EXPR){
                free_expr_token(&tmp_stack_item->token);
                free(tmp_stack_item);
                StackPop(&help_stack);
                tmp_stack_item = (tExprItem*) StackTop(&help_stack);
                tExprItem *new_expr_item = (tExprItem*) malloc(sizeof(tExprItem));
                if (new_expr_item == NULL){
                    exit(99);
                }
                switch (tmp_stack_item->type) {
                    case T_ADD_EXPR:
                    case T_SUB_EXPR:
                    case T_MUL_EXPR:
                    case T_DIV_EXPR:
                    case T_CONCAT_EXPR:
                    case T_LT_EXPR:
                    case T_GT_EXPR:
                    case T_LTE_EXPR:
                    case T_GTE_EXPR:
                    case T_EQ_EXPR:
                    case T_NEQ_EXPR:
                        free_expr_token(&tmp_stack_item->token);
                        free(tmp_stack_item);
                        StackPop(&help_stack);
                        tmp_stack_item = (tExprItem*) StackTop(&help_stack);

                        if (tmp_stack_item != NULL && tmp_stack_item->type == T_OPERAND_EXPR){
                            // Create new stack item and push it
                            new_expr_item->token = NULL;
                            new_expr_item->type = T_OPERAND_EXPR;
                            new_expr_item->is_terminal = false;
                            new_expr_item->handle = false;

                            StackPush(&expr_stack, new_expr_item);

                        } else {
                            free(current_token);
                            free(new_expr_item);
                            printf("err8\n");
                            return false;
                        }
                        break;
                    default:
                        free(current_token);
                        free(new_expr_item);
                        printf("err7\n");
                        return false;
                }
            } else {
                printf("err6\n");
                return false;
            }

            clean_expr_stack(&help_stack);

        } else if (preced_tab[top_terminal_idx][input_token_idx] == '\0'){
            printf("err5\n");
            return false;
        }
    }
    free_expr_token(&current_token);
    clean_expr_stack(&expr_stack);
    return true;
}

void free_expr_token(tToken **token){
    if ((*token) != NULL){
        // varid funid string
        switch ((*token)->type) {
            case T_VAR_ID:
            case T_FUN_ID:
            case T_STRING:
                dynamicBufferFREE((*token)->data.STRINGval);
                break;
            default:
                break;
        }
        free((*token));
        (*token) = NULL;
    }
}

void clean_expr_stack(tStack *expr_stack){
    while (!StackIsEmpty(expr_stack)){
        tExprItem *free_item = (tExprItem*) StackTop(expr_stack);

        free_expr_token(&free_item->token);

        free(free_item);
        StackPop(expr_stack);
    }
    StackInit(expr_stack);
}

bool is_expr_end_token(tToken *current_token, tToken *end_token, size_t *par_level){
    if (current_token->type == end_token->type && current_token->type == T_R_PAR){
        if (*par_level == 0){
            return true;
        }

        (*par_level)--;
        return false;
    }

    return current_token->type == end_token->type;
}

tExprItem *get_stack_top_terminal(tStack *expr_stack){
    tStack help_stack;
    StackInit(&help_stack);

    tExprItem *current_item = (tExprItem*) StackTop(expr_stack);
    while (current_item->is_terminal == false){
        StackPop(expr_stack);
        StackPush(&help_stack, current_item);
        current_item = (tExprItem*) StackTop(expr_stack);
    }

    while (!StackIsEmpty(&help_stack)){
        StackPush(expr_stack, (tExprItem*) StackTop(&help_stack));
        StackPop(&help_stack);
    }

    return current_item;
}

bool is_stack_end_state(tStack *expr_stack, tToken *end_token){
    tExprItem *stack_top_item = (tExprItem*) StackTop(expr_stack);
    bool top_is_expr = !stack_top_item->is_terminal;
    StackPop(expr_stack);

    tExprItem *expr_start_item = (tExprItem*) StackTop(expr_stack);
    StackPush(expr_stack, stack_top_item);
    if (expr_start_item != NULL){
        bool next_is_end_token = expr_start_item->is_terminal == true && expr_start_item->token->type == end_token->type;
        return top_is_expr && next_is_end_token;
    }
    return false;
}

enum expr_item_type token_to_preced_idx(enum token_type token_type, bool is_end){
    switch (token_type) {
        case T_VAR_ID:
        case T_STRING:
        case T_NUM_INT:
        case T_NUM_FLOAT:
            return T_OPERAND_EXPR;
        case T_CONCATENATION:
            return T_CONCAT_EXPR;
        case T_L_PAR:
            return T_LPAR_EXPR;
        case T_R_PAR:
            return is_end ? T_END_EXPR : T_RPAR_EXPR;
        case T_EQUALS:
            return T_EQ_EXPR;
        case T_NOT_EQUALS:
            return T_NEQ_EXPR;
        case T_GREATER_THAN:
            return T_GT_EXPR;
        case T_GREATER_OR_EQUAL:
            return T_GTE_EXPR;
        case T_LESS_THAN:
            return T_LT_EXPR;
        case T_LESS_OR_EQUAL:
            return T_LTE_EXPR;
        case T_MUL:
            return T_MUL_EXPR;
        case T_DIV:
            return T_DIV_EXPR;
        case T_ADD:
            return T_ADD_EXPR;
        case T_SUB:
            return T_SUB_EXPR;
        case T_SEMICOLON:
            return T_END_EXPR;
        case T_NULL:
            return T_OPERAND_EXPR;
        default:
            return T_UNKNOW_EXPR;
    }
}

#ifdef TESTING

/**
 * @brief print token type
 *
 * @param unum token_type member
 */
/*
void print_token_type(enum token_type type){
    switch (type)
    {
    case T_ADD_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_ADD_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_SUB_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_SUB_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_MUL_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_MUL_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_DIV_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_DIV_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_CONCAT_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_CONCAT_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_LT_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_LT_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_GT_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_GT_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_LTE_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_LTE_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_GTE_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_GTE_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_EQ_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_EQ_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_NEQ_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_NEQ_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_LPAR_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_LPAR_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_RPAR_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_RPAR_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_OPERAND_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_OPERAND_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_END_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_END_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    case T_UNKNOW_EXPR:
        printf(ANSI_COLOR_YELLOW "[T_UNKNOW_EXPR]" ANSI_COLOR_RESET "\n");
        break;
    default:
        break;
    }
}*/

int main(){
    printf("TESTING EXPRESSION PARSER!\n");
    get_token(0);
    tToken start_tok = get_token(1);
    tStack stack;
    StackInit(&stack);

    tToken end_tok_semicolon = {.type=T_SEMICOLON};
    // tToken end_tok_r_par = {.type=T_R_PAR};

    DLList instruction_list;
    DLL_Init(&instruction_list);

    if (check_expr_syntax(&start_tok, &end_tok_semicolon, &instruction_list, NULL))
        printf("Expression is syntactically OK\n");
    else
        printf("Expression is syntactically WRONG\n");

    return 0;
}

#endif
