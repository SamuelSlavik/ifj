/**
 * @file error.h
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.1
 * @date 2022-11-10
 * 
 */

#ifndef ERROR_H
#define ERROR_H
#include <stdio.h>
#include "scanner.h"

#define NO_ERROR 0
#define LEX_ERROR 1
#define SYNTAX_ERROR 2
#define RE_DEF_ERROR 3
#define PARAM_ERROR 4
#define UN_DEF_VAR_ERROR 5
#define RETURN_ERROR 6
#define EXPRESSION_ERROR 7
#define OTHER_ERROR 8
#define ERROR 99

/**
 * @brief return 0 if there is no error or non 0 if there is some error
 * 
 */
#define CHECK_ERROR error_handle(NO_ERROR)

/**
 * @brief Function to handle the error messaging. 
 * When code is set 0 the function return error code of first error that occures.
 * When code is set to else then 0 function print error message code and
 * set program error code to given error code
 * 
 * @param code given error code, is set to 0 on default
 * @return int number of error code that occures
 */
int error_handle(int code);

int error_exit(tToken *token,int code);

#endif // ERROR_H