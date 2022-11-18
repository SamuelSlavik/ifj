#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"

bool fstart(tToken *token);

bool fprog(tToken *token);

bool fbody(tToken *token);

bool fbody_as(tToken *token);

bool fbody_var(tToken *token);

bool fbody_ret(tToken *token);

bool fn_call_l(tToken *token);

#endif //__PARSER_H__