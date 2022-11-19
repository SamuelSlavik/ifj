#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"

bool f_start(tToken *token);

bool f_prog(tToken *token);

bool f_body(tToken *token);

bool f_body_as(tToken *token);

bool f_body_var(tToken *token);

bool f_body_ret(tToken *token);

bool f_fn_call_l(tToken *token);

bool f_fn_call_lc(tToken *token);

bool f_func(tToken *token);

bool f_func_dedf(tToken *token);

bool f_func_type(tToken *token);

bool f_in_body(tToken *token);

bool f_func_param(tToken *token);

bool f_func_mparam(tToken * token);

#endif //__PARSER_H__
