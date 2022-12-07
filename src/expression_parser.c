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

// Global symbol table initialized in symtable.c
extern htab_t *symtable;

void format_esc_sq(long int_val, tDynamicBuffer *string){
    tDynamicBuffer *int_val_str = long_2_string(int_val);
    // If value of character is less than 3 digits, fill 0s at beginning
    if (int_val_str->size < 3){
        tDynamicBuffer *int_val_str_tmp = int_val_str;
        int_val_str = dynamicBuffer_INIT();
        for (size_t j = 0; j < 3 - int_val_str_tmp->size; ++j) {
            dynamicBuffer_ADD_CHAR(int_val_str, '0');
        }
        dynamicBuffer_ADD_STRING(int_val_str, int_val_str_tmp->data);
        dynamicBufferFREE(int_val_str_tmp);
    }
    // Store escape sequence to destination string
    dynamicBuffer_ADD_STRING(string, "\\");
    dynamicBuffer_ADD_STRING(string, int_val_str->data);
    dynamicBufferFREE(int_val_str);
}

void string_to_ifj_fmt(tDynamicBuffer **string){
    // Create temporary copy of php string
    tDynamicBuffer *tmp_copy = dynamicBuffer_INIT();
    dynamicBuffer_ADD_STRING(tmp_copy, (*string)->data);
    // Return desired buffer to its initial state
    *string = dynamicBuffer_RESET(*string);

    size_t len = tmp_copy->size + 1;
    int current_char;

    for (size_t i = 0; i < len; ++i) {
        current_char = (unsigned char) tmp_copy->data[i];

        // Convert special characters and php escape sequences to correct escape sequence
        if (current_char == ' '){
            dynamicBuffer_ADD_STRING(*string, "\\032");
        } else if (current_char == '#'){
            dynamicBuffer_ADD_STRING(*string, "\\035");
        } else if (current_char == '$'){ // Character $ is forbidden outside escape sequence in string
            exit(1);
        } else if (current_char == '\\'){ // Found escape sequence beginning
            current_char = (unsigned char) tmp_copy->data[++i];

            // Convert php escape sequence to IFJcode22 escape sequence based on type
            if (current_char == '\\'){ // Convert php \\ to \ in IFJcode22
                dynamicBuffer_ADD_STRING(*string, "\\092");
            } else if(current_char == 't'){ // Convert \t to tab in IFJcode22
                dynamicBuffer_ADD_STRING(*string, "\\009");
            } else if(current_char == 'n'){ // Convert \n to newline in IFJcode22
                dynamicBuffer_ADD_STRING(*string, "\\010");
            } else if(current_char == '"'){ // Convert \" to " in IFJcode 22
                dynamicBuffer_ADD_STRING(*string, "\\034");
            } else if(current_char == '$'){ // Convert \$ to $ in IFJcode 22
                dynamicBuffer_ADD_STRING(*string, "\\036");
            } else if(current_char == 'x'){ // Convert hexadecimal \xhh to decadic \ddd in IFJcode22
                tDynamicBuffer *hex_str = dynamicBuffer_INIT();
                dynamicBuffer_ADD_CHAR(hex_str, tmp_copy->data[i+1]);
                dynamicBuffer_ADD_CHAR(hex_str, tmp_copy->data[i+2]);
                // Convert hexadecimal string to decadic int value
                char *hex_end_ptr = NULL;
                long hex_int_val = strtol(hex_str->data, &hex_end_ptr, 16);
                if (*hex_end_ptr != '\0'){ // Conversion failed, escape sequence is invalid and do not change characters
                    dynamicBuffer_ADD_STRING(*string, "\\092x");
                } else { // Conversion was successful
                    format_esc_sq(hex_int_val, *string);
                    i += 2;
                }
                dynamicBufferFREE(hex_str);
            } else if(current_char >= '0' && current_char <= '3'){ // Convert octal \ooo to decadic \ddd in IFJcode22
                tDynamicBuffer *oct_str = dynamicBuffer_INIT();
                dynamicBuffer_ADD_CHAR(oct_str, (char) current_char);
                dynamicBuffer_ADD_CHAR(oct_str, tmp_copy->data[i+1]);
                dynamicBuffer_ADD_CHAR(oct_str, tmp_copy->data[i+2]);
                // Convert octal string to decadic int value
                char *oct_end_ptr = NULL;
                long oct_int_val = strtol(oct_str->data, &oct_end_ptr, 8);
                if (*oct_end_ptr != '\0'){ // Conversion failed, escape sequence is invalid and do not change characters
                    dynamicBuffer_ADD_STRING(*string, "\\092");
                    dynamicBuffer_ADD_CHAR(*string, (char) current_char);
                } else { // Conversion was successful
                    format_esc_sq(oct_int_val, *string);
                    i += 2;
                }
                dynamicBufferFREE(oct_str);
            } else { // For incorrect escape sequence, preserve \ and characters after
                dynamicBuffer_ADD_STRING(*string, "\\092");
                dynamicBuffer_ADD_CHAR(*string, (char) current_char);
            }
        } else { // For non-special characters and non-escape sequences do not change character
            dynamicBuffer_ADD_CHAR(*string, (char) current_char);
        }
    }

    dynamicBufferFREE(tmp_copy);
}

tDynamicBuffer *double_2_string(double num){
    tDynamicBuffer *float_str = dynamicBuffer_INIT();
    char buffer[50] = {0};
    sprintf(buffer, "%a", num);
    dynamicBuffer_ADD_STRING(float_str, buffer);
    return float_str;
}

tDynamicBuffer *long_2_string(long int num){
    tDynamicBuffer *long_str = dynamicBuffer_INIT();
    long int number = num;
    // Count number of digits
    long long int digit_count = 1;
    while (number != 0){
        number /= 10;
        digit_count++;
    }
    // Create temporary buffer to store converted string
    char *buffer = calloc(sizeof(char), digit_count + 1);
    if (buffer == NULL){
        exit(99);
    }
    sprintf(buffer, "%ld", num);
    // Store converted string to result
    dynamicBuffer_ADD_STRING(long_str, buffer);
    free(buffer);
    return long_str;
}

bool parse_expression(tToken *start_token, tToken *end_token, DLList *instruction_list, tToken *extra_token){
    PRECED_TAB;

    tStack expr_stack;
    StackInit(&expr_stack);
    // Allocate end token with type of token that should end the expression
    tToken *end_token_alloc = (tToken*) malloc(sizeof(tToken));
    if (end_token_alloc == NULL){
        exit(99);
    }
    end_token_alloc->type = end_token->type;
    // Allocate expression item with ending token and push it to expression stack
    tExprItem *end_item = (tExprItem*) malloc(sizeof(tExprItem));
    if (end_item == NULL){
        exit(99);
    }
    end_item->token = end_token_alloc;
    end_item->type = T_END_EXPR;
    end_item->is_terminal = true;
    end_item->handle = false;
    StackPush(&expr_stack, end_item);
    // Allocate memory in order to store first token
    tToken *current_token = (tToken*) malloc(sizeof(tToken));
    if (current_token == NULL){
        exit(99);
    }
    // Decide if first token is start token or extra token
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

    // Initialize level of nested parenthesis to 0
    size_t par_level = 0;

    while (!is_expr_end_token(current_token, end_token, &par_level) || !is_stack_end_state(&expr_stack, end_token)){

        if (current_token->type == T_ERROR){ // Check for lexical error
            error_exit(current_token, 1);
        } else if (current_token->type == T_VAR_ID){ // Check for undefined variable
            if (!htab_find(instruction_list->called_from->data.fun_data.localST, current_token->data.STRINGval->data)){
                error_exit(current_token, 5);
            }
        } else if (current_token->type == T_L_PAR){ // Increase nested parenthesis level
            par_level++;
        }

        // Get topmost terminal in expression stack and precedence table indexes
        tExprItem *top_terminal = get_stack_top_terminal(&expr_stack);
        size_t top_terminal_idx = top_terminal->type;
        size_t input_token_idx = token_to_preced_idx(current_token->type, is_expr_end_token(current_token, end_token, &par_level) && top_terminal_idx != T_LPAR_EXPR);
        if (input_token_idx == T_UNKNOW_EXPR){ // Check for invalid syntax
            return false;
        }

        // Handle combination of topmost terminal and current input token
        if (preced_tab[top_terminal_idx][input_token_idx] == '='){ // Push current token to stack and get next one
            // Create new expression stack item and push it
            tExprItem *new_expr_item = (tExprItem*) malloc(sizeof(tExprItem));
            if (new_expr_item == NULL){
                exit(99);
            }
            new_expr_item->token = current_token;
            new_expr_item->type = token_to_preced_idx(current_token->type, is_expr_end_token(current_token, end_token, &par_level) && top_terminal_idx != T_LPAR_EXPR);
            new_expr_item->is_terminal = true;
            new_expr_item->handle = false;

            StackPush(&expr_stack, new_expr_item);

            // Allocate space for next token and load next token
            current_token = (tToken*) malloc(sizeof(tToken));
            if (current_token == NULL){
                exit(99);
            }
            // Decide if next token should be start token (if extra_token was passed) or loaded from scanner
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
            // Mark topmost terminal as start of expression reduction and push current token to stack and get next one
            top_terminal->handle = true;

            // Create new expression stack item and push it
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
            // Decide if next token should be start token (if extra_token was passed) or loaded from scanner
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
            // Reduce expression on stack, check syntax, generate code for reduced part and perform semantic checks
            // Initialize helper stack to help with expression reduction
            tStack help_stack;
            StackInit(&help_stack);

            // CODE GENERATION START
            tDynamicBuffer *instruction = dynamicBuffer_INIT();
            tDynamicBuffer *num_str;
            // Decide what part of expression to generate code for
            switch (top_terminal->type) {
                case T_OPERAND_EXPR:  // Generate code for operand (push them to stack in IFJcode22)
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

            // Get whole expression that needs to be reduced to helper stack
            tExprItem *tmp_stack_item = (tExprItem*) StackTop(&expr_stack);

            while (!tmp_stack_item->handle){
                StackPush(&help_stack, tmp_stack_item);
                StackPop(&expr_stack);
                tmp_stack_item = StackTop(&expr_stack);
            }
            // Remove reduction start mark
            tmp_stack_item->handle = false;

            // Decide according to which rule we are reducing the part of expression
            tmp_stack_item = StackTop(&help_stack);

            if (tmp_stack_item->is_terminal == true){ // Rule E -> i or E -> (E) where i and ( are terminals
                if (tmp_stack_item->type == T_OPERAND_EXPR){ // Rule E -> i where i is variable or literal
                    // Create new expression stack item and push it
                    tExprItem *new_expr_item = (tExprItem*) malloc(sizeof(tExprItem));
                    if (new_expr_item == NULL){
                        exit(99);
                    }
                    new_expr_item->token = NULL;
                    new_expr_item->type = T_OPERAND_EXPR;
                    new_expr_item->is_terminal = false;
                    new_expr_item->handle = false;

                    StackPush(&expr_stack, new_expr_item);

                } else if (tmp_stack_item->type == T_LPAR_EXPR){ // Rule E -> (E)
                    free_expr_token(&tmp_stack_item->token);
                    free(tmp_stack_item);
                    StackPop(&help_stack);
                    // Check whether next expression item matches the rule
                    tmp_stack_item = (tExprItem*) StackTop(&help_stack);

                    if (tmp_stack_item != NULL && tmp_stack_item->type == T_OPERAND_EXPR){ // Match (E
                        free_expr_token(&tmp_stack_item->token);
                        free(tmp_stack_item);
                        StackPop(&help_stack);
                        // Check whether next expression item matches the rule
                        tmp_stack_item = (tExprItem*) StackTop(&help_stack);

                        if (tmp_stack_item != NULL && tmp_stack_item->type == T_RPAR_EXPR){ // Whole match (E)
                            // Create new expression stack item and push it
                            tExprItem *new_expr_item = (tExprItem*) malloc(sizeof(tExprItem));
                            if (new_expr_item == NULL){
                                exit(99);
                            }
                            new_expr_item->token = NULL;
                            new_expr_item->type = T_OPERAND_EXPR;
                            new_expr_item->is_terminal = false;
                            new_expr_item->handle = false;

                            StackPush(&expr_stack, new_expr_item);
                        } else { // No match ( found (E[NOT RIGHT PARENTHESIS] ), syntax is wrong
                            if (tmp_stack_item != NULL){
                                free_expr_token(&tmp_stack_item->token);
                                free(tmp_stack_item);
                            }
                            free_expr_token(&current_token);
                            return false;
                        }
                    } else { // No match ( found ([NOT OPERAND] ), syntax is wrong
                        if (tmp_stack_item != NULL){
                            free_expr_token(&tmp_stack_item->token);
                            free(tmp_stack_item);
                        }
                        free_expr_token(&current_token);
                        return false;
                    }
                } else { // No rule found for the found terminal, syntax is wrong
                    if (tmp_stack_item != NULL){
                        free_expr_token(&tmp_stack_item->token);
                        free(tmp_stack_item);
                    }
                    free_expr_token(&current_token);
                    return false;
                }
            } else if (tmp_stack_item->type == T_OPERAND_EXPR){ // Rule E [OPERATOR] E where E are non-terminal operands
                free_expr_token(&tmp_stack_item->token);
                free(tmp_stack_item);
                StackPop(&help_stack);
                // Check whether next expression item matches the rule
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
                    case T_NEQ_EXPR: // Match E [OPERATOR]
                        free_expr_token(&tmp_stack_item->token);
                        free(tmp_stack_item);
                        StackPop(&help_stack);
                        // Check whether next expression item matches the rule
                        tmp_stack_item = (tExprItem*) StackTop(&help_stack);

                        if (tmp_stack_item != NULL && tmp_stack_item->type == T_OPERAND_EXPR){ // Whole match E [OPERATOR] E
                            // Create new expression stack item and push it
                            new_expr_item->token = NULL;
                            new_expr_item->type = T_OPERAND_EXPR;
                            new_expr_item->is_terminal = false;
                            new_expr_item->handle = false;

                            StackPush(&expr_stack, new_expr_item);

                        } else { // No match, ( found E [OPERATOR] [NOT OPERAND] ), syntax is wrong
                            if (tmp_stack_item != NULL){
                                free_expr_token(&tmp_stack_item->token);
                                free(tmp_stack_item);
                            }
                            free_expr_token(&current_token);
                            return false;
                        }
                        break;
                    default: // No match, ( found E [NOT OPERATOR] ), syntax is wrong
                        if (tmp_stack_item != NULL){
                            free_expr_token(&tmp_stack_item->token);
                            free(tmp_stack_item);
                        }
                        free_expr_token(&current_token);
                        return false;
                }
            } else { // No rule found to match
                if (tmp_stack_item != NULL){
                    free_expr_token(&tmp_stack_item->token);
                    free(tmp_stack_item);
                }
                free_expr_token(&current_token);
                return false;
            }

            // Clean helper stack
            clean_expr_stack(&help_stack);

        } else if (preced_tab[top_terminal_idx][input_token_idx] == '\0'){ // Found error combination in precedence table
            free_expr_token(&current_token);
            return false;
        }
    }
    // Expression is syntactically and semantically correct
    free_expr_token(&current_token);
    clean_expr_stack(&expr_stack);
    return true;
}

void free_expr_token(tToken **token){
    if ((*token) != NULL){
        switch ((*token)->type) { // If token is any of those types it has allocated data that need to be freed
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
    // Check if it is indeed ending parenthesis according to level
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

    // Pop expression stack until terminal is found
    tExprItem *current_item = (tExprItem*) StackTop(expr_stack);
    while (current_item->is_terminal == false){
        StackPop(expr_stack);
        // Store popped items into helper stack
        StackPush(&help_stack, current_item);
        current_item = (tExprItem*) StackTop(expr_stack);
    }

    // Return expression stack to original state
    while (!StackIsEmpty(&help_stack)){
        StackPush(expr_stack, (tExprItem*) StackTop(&help_stack));
        StackPop(&help_stack);
    }

    return current_item;
}

bool is_stack_end_state(tStack *expr_stack, tToken *end_token){
    // Check if topmost item is non-terminal
    tExprItem *stack_top_item = (tExprItem*) StackTop(expr_stack);
    bool top_is_expr = !stack_top_item->is_terminal;
    // Get second expression stack item that should be end_token pushed at the beginning
    StackPop(expr_stack);
    tExprItem *expr_start_item = (tExprItem*) StackTop(expr_stack);
    // Return topmost item to stack
    StackPush(expr_stack, stack_top_item);

    // Evaluate state
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
            // Check if right parenthesis is end_token or regular parenthesis in expression
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
