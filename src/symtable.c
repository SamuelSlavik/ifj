/**
 * @file symtable.c
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.3
 * @date 2022-11-16
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "htab.h"

#include "stack.h"
#include "scanner.h"



htab_data_t *st_fun_create(htab_t *global_ST, char *fun_name){

    /* Check if already does not exist */
    htab_data_t *data_ST = htab_find(global_ST, fun_name);
    if (data_ST != NULL){
        return data_ST;
    }

    /* Add to global table */
    data_ST = htab_lookup_add(global_ST, fun_name);
    if (data_ST == NULL){
        return NULL;
    }

    data_ST->isfun = true;

    /* Set declared to true and defined to false and return to to Unknow*/
    data_ST->data.fun_data.declared = true;
    data_ST->data.fun_data.defined = false;
    data_ST->data.fun_data.return_type = T_UNKNOW;

    /* Create stack of parameters */
    tStack *param_stack = malloc(sizeof(tStack));
    if (param_stack == NULL){
        htab_free(global_ST);
        return NULL;
    }

    StackInit(param_stack);

    data_ST->data.fun_data.TaV = param_stack;

    /* Set local symbol table to NULL */
    data_ST->data.fun_data.localST = NULL;

    return data_ST;
}



bool st_fun_param_type(tStack *s, enum token_type type){

    tVar_TaV *tuple = malloc(sizeof(tVar_TaV));
    if (tuple == NULL){
        return false;
    }

    tuple->var_type = type;

    if (!StackPush(s, (void *)tuple)) return false;

    return true;
}



bool st_fun_param_name(tStack *s, char *name){

    char *var = malloc(sizeof(char) * (strlen(name) + 1));
    if (var == NULL){
        return false;
    }
    strcpy(var, name);

    
    tVar_TaV *tuple = (tVar_TaV *)StackTop(s);

    tuple->var = var;

    return true;
}



void st_fun_retrun_type(htab_data_t *data, enum token_type type){
    data->data.fun_data.return_type = type;
}



void st_fun_definition(htab_data_t *data){
    data->data.fun_data.defined = true;
}



htab_t *st_fun_call(htab_t *global_ST, tStack *s, char *fun_name){

    /* Find function data */
    htab_data_t *data = htab_find(global_ST, fun_name);
    if (data == NULL){ // not found
        /* TODO: "Function was not defined" err handle */
        return NULL;
    }

    /* Create ST for function */
    htab_t *local_ST = htab_init(HTAB_SIZE);
    if (local_ST == NULL){
        return NULL;
    }
    data->data.fun_data.localST = local_ST;
    local_ST->globalST = global_ST;

    if (!StackPush(s, (void *) local_ST)){
        htab_free(local_ST);
        return NULL;
    }
    return local_ST;
}



htab_t *st_fun_return(tStack *s){

    htab_t *ST = (htab_t *)StackTop(s);
    htab_free(ST);
    StackPop(s);
    return StackTop(s);
}



htab_data_t *st_var_create(htab_t *t, char *name){

    htab_data_t *data_ST = htab_find(t, name);

    if (data_ST != NULL){
        return data_ST;
    }
    else{
        /* Add to symbol table */
        data_ST = htab_lookup_add(t, name);
        if (data_ST == NULL){
            return NULL;
        }

        data_ST->isfun = false;

        /* Set init to false */
        data_ST->data.var_data.init = false;

        return data_ST;
    }
}



void st_var_set(htab_t *t, char *name, enum token_type type){

    htab_data_t *data = htab_find(t, name);
    if (data == NULL){ // if variable does not exist do nothing
        return;
    }

    data->data.var_data.type = type;
    data->data.var_data.init = true;
}



void st_fun_param_free(htab_data_t *data){
    if (data->isfun){
        while (!StackIsEmpty(data->data.fun_data.TaV)){
            free(((tVar_TaV *)StackTop(data->data.fun_data.TaV))->var);
            free(((tVar_TaV *)StackTop(data->data.fun_data.TaV)));
            StackPop(data->data.fun_data.TaV);
        }                
        free(data->data.fun_data.TaV);
    }
}
