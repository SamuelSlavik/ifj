/**
 * @file error.c
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.1
 * @date 2022-11-10
 */

#include <stdio.h>
#include <stdbool.h>
#include "error.h"

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