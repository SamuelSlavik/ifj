/**
 * @file error.c
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.1
 * @date 2022-11-10
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "error.h"
#include "scanner.h"

int error_handle(int code){
    static int err_code = NO_ERROR;

    if (code != NO_ERROR && err_code == NO_ERROR){
        err_code = code;

        switch (err_code)
        {
        case LEX_ERROR:
            fprintf(stderr, "ERROR: A lexical error has occurred !\n");
            break;
        case SYNTAX_ERROR:
            fprintf(stderr, "ERROR: Wrong syntax !\n");
            break;
        case RE_DEF_ERROR:
            fprintf(stderr, "ERROR: Using an undefined function !\n");
            break;
        case PARAM_ERROR:
            fprintf(stderr, "ERROR: Wrong number of parameters in function \n");
            break;
        case UN_DEF_VAR_ERROR:
            fprintf(stderr, "ERROR: Using an undefined variable !\n");
            break;
        case RETURN_ERROR:
            fprintf(stderr, "ERROR: Wrong type or number of return paramters !\n");
            break;
        case EXPRESSION_ERROR:
            fprintf(stderr, "ERROR: Wrong expression entered !\n");
            break;
        case OTHER_ERROR:
            fprintf(stderr, "ERROR: Error during compilation !\n");
            break;
        case ERROR:
            fprintf(stderr, "ERROR: Some unexpected error occured !\n");
            break;
        default:
            break;
        }
    }
    
    return err_code;
}

int error_exit(tToken *token,int code){
        switch (code)
        {
        case LEX_ERROR:
            fprintf(stderr, "ERROR: A lexical error has occurred! At line %ld.\n",token->line);
            break;
        case SYNTAX_ERROR:
            fprintf(stderr, "ERROR: Wrong syntax! At line %ld.\n",token->line);
            break;
        case RE_DEF_ERROR:
            fprintf(stderr, "ERROR: Using an undefined function! At line %ld.\n",token->line);
            break;
        case PARAM_ERROR:
            fprintf(stderr, "ERROR: Wrong number of parameters in function! At line %ld.\n",token->line);
            break;
        case UN_DEF_VAR_ERROR:
            fprintf(stderr, "ERROR: Using an undefined variable! At line %ld.\n",token->line);
            break;
        case RETURN_ERROR:
            fprintf(stderr, "ERROR: Wrong type or number of return paramters! At line %ld.\n",token->line);
            break;
        case EXPRESSION_ERROR:
            fprintf(stderr, "ERROR: Wrong expression entered! At line %ld.\n",token->line);
            break;
        case OTHER_ERROR:
            fprintf(stderr, "ERROR: Error during compilation! At line %ld.\n",token->line);
            break;
        case ERROR:
            fprintf(stderr, "ERROR: Some unexpected error occured! At line %ld.\n",token->line);
            break;
        default:
            break;
        }
    exit(code);
}
