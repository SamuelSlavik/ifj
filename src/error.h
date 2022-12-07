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
 * @brief function for handling unexpected errors like allocation fail etc...
 * 
 */
int error_handle();

/**
 * @brief function that exits program with error code and writes at stderr message where error orcured
 * 
 * @param token pointer on token for displaying line
 * @param code error code
 */
int error_exit(tToken *token,int code);

#endif // ERROR_H