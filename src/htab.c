/**
 * @file htab.c
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.1
 * @date 2022-11-10
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "htab.h"
#include "stack.h"
#include "scanner.h"
#include "dynamic_buffer.h"




size_t htab_hash_function(htab_key_t str) {
    uint32_t h = 0;
    const unsigned char *p;
    for(p = (const unsigned char*)str; *p != '\0'; p++)
        h = 65599 * h + *p;
    return h;
}


htab_t *htab_init(size_t n){
    htab_t *new = malloc(sizeof(htab_t));
    if (new == NULL){
        return NULL;
    }
    htab_item_t **list = malloc(n * sizeof(htab_item_t *));
    if (list == NULL){
        return NULL;
    }
    new->size = 0;
    new->arr_size = n;
    new->arr_ptr = list;
    for (size_t i = 0; i < n; i++){
        new->arr_ptr[i] = NULL;
    }
    return new;
}


size_t htab_size(const htab_t * t){
    return t->size;
}


size_t htab_bucket_count(const htab_t * t){
    return t->arr_size;
}


void htab_resize(htab_t *t, size_t newn){
    /* New list allocation */
    htab_item_t **list = malloc(newn * sizeof(htab_item_t *));
    if (list == NULL || newn <= 0){
        /* Allocation failed t without changes */
        return;
    }
    else {
        /* init of new list */
        for (size_t i = 0; i < newn; i++){
            list[i] = NULL;
        }

        /* Moving through old list */
        for (size_t i = 0; i < htab_bucket_count(t); i++){
            htab_item_t *current_item = t->arr_ptr[i];
            while (current_item != NULL){
                /* Calculate new position */
                size_t index = (htab_hash_function(current_item->data->key) % newn);

                htab_item_t *item_to_sort = current_item;
                current_item = current_item->next;
                item_to_sort->next = NULL;

                /* Fill new list */
                htab_item_t *list_item = list[index];
                if (list_item != NULL){
                    while (list_item->next != NULL){
                        list_item = list_item->next;
                    }
                    list_item->next = item_to_sort;
                }
                else {
                    list[index] = item_to_sort;
                }
            }
        }

        /* Free old list, adding new list to hash table and set arr_size */
        free(t->arr_ptr);
        t->arr_ptr = list;
        t->arr_size = newn;
    }
}


htab_data_t * htab_find(htab_t * t, htab_key_t key){
    size_t index = (htab_hash_function(key) % htab_bucket_count(t));
    htab_item_t *item = t->arr_ptr[index];

    while (item != NULL){
        if ((strcmp(key, item->data->key)) == 0){
            return item->data;
        }
        item = item->next;
    }
    return NULL;
}


htab_data_t * htab_lookup_add(htab_t * t, htab_key_t key){
    /* Check if item with same key is not already in hash table */
    htab_data_t *pair = htab_find(t, key);
    if (pair != NULL){
        return pair;
    }
    /* Key is not in hash table */
    else{
        /* Memory allocation */
        htab_item_t *new_item = malloc(sizeof(htab_item_t));
        if (new_item == NULL){
            return NULL;
        }
        htab_data_t *new_pair = malloc(sizeof(htab_data_t));
        if (new_pair == NULL){
            free(new_item);
            return NULL;
        }
        new_pair->key = malloc(sizeof(char) * (strlen(key) + 1));
        if (new_pair->key == NULL){
            free(new_pair);
            free(new_item);
            return NULL;
        }
        /* New item init */
        strcpy((void *)new_pair->key, key);

        // INIT DATA

        new_item->data = new_pair;
        new_item->next = NULL;
        /* Limit check */
        if ((t->size / t->arr_size) > AVG_LEN_MAX){
            htab_resize(t, (2 * t->arr_size));
        }
        /* Adding item */
        t->size++;

        size_t index = (htab_hash_function(key) % htab_bucket_count(t));
        htab_item_t *current_item = t->arr_ptr[index];
        /* List is not empty */
        if (current_item != NULL){
            while (current_item->next != NULL){
                current_item = current_item->next;
            }
            current_item->next = new_item;
        }
        /* List is empty */
        else{
            t->arr_ptr[index] = new_item;
        }
        return new_pair;
    }
}


bool htab_erase(htab_t * t, htab_key_t key){
    size_t index = (htab_hash_function(key) % htab_bucket_count(t));
    htab_item_t *item = t->arr_ptr[index];
    /* List is not empty */
    if (item != NULL){
        /* First item is searching item */
        if ((strcmp(key, item->data->key)) == 0){
            t->arr_ptr[index] = item->next;
            free((void *)item->data->key);
            free(item->data);
            free(item);
            t->size--;
            if ((t->size / t->arr_size) < AVG_LEN_MIN){
                htab_resize(t, (t->arr_size / 2));
            }
            return true;
        }
        htab_item_t *item_before = item;
        item = item->next;
        /* Other items */
        while (item != NULL){
            if ((strcmp(key, item->data->key)) == 0){
                item_before->next = item->next;
                free((void *)item->data->key);
                free(item->data);
                free(item);
                t->size--;
                if ((t->size / t->arr_size) < AVG_LEN_MIN){
                    htab_resize(t, (t->arr_size / 2));
                }
                return true;
            }
            item_before = item;
            item = item->next;
        }
        return false;
    }
    else {
        return false;
    }
}


void htab_clear(htab_t * t){
    for (size_t i = 0; i < htab_bucket_count(t); i++){
        htab_item_t *item = t->arr_ptr[i];
        while (item != NULL){
            htab_item_t *item_to_del = item;
            item = item->next;

            free((void *)item_to_del->data->key);
            free(item_to_del->data);
            free(item_to_del);
        }
    }
    t->size = 0;
}


void htab_free(htab_t * t){
    htab_clear(t);
    free(t->arr_ptr);
    free(t);
}


/* Functions for working with symbol table */


htab_data_t *st_fun_declaration(htab_t *global_ST, tStack *param_s, tToken token){

    htab_data_t *data_ST;

    /* Add to global table */
    data_ST = htab_lookup_add(global_ST, token.data.STRINGval->data);
    if (data_ST == NULL){
        return NULL;
    }

    /* Free token string */
    dynamicBufferFREE(token.data.STRINGval);

    /* Set declared to true and defined to false and return to to Unknow*/
    data_ST->data.fun_data.declared = true;
    data_ST->data.fun_data.defined = false;
    data_ST->data.fun_data.return_type = T_UNKNOW;

    /* Create stack of parameters */
    StackInit(param_s);

    data_ST->data.fun_data.TaV = param_s;

    /* Set local symbol table to NULL */
    data_ST->data.fun_data.localST = NULL;

    return data_ST;
}



tVar_TaV* st_fun_param_create(tToken token){

    tVar_TaV *tuple = malloc(sizeof(tVar_TaV));
    if (tuple == NULL){
        return NULL;
    }

    tuple->var_type = token.type;
    return tuple;
}



bool st_fun_param_set(htab_data_t *data, tVar_TaV *tuple, tToken token){

    char *var = malloc(sizeof(char) * (strlen(token.data.STRINGval->data) + 1));
    if (var == NULL){
        return false;
    }
    strcpy(var, token.data.STRINGval->data);

    dynamicBufferFREE(token.data.STRINGval);

    tuple->var = var;

    if (!StackPush(data->data.fun_data.TaV, (void *) tuple)){
        return false;
    }
    return true;
}



void st_fun_retrun_type(htab_data_t *data, tToken token){
    data->data.fun_data.return_type = token.type;
}



void st_fun_definition(htab_data_t *data){
    data->data.fun_data.defined = true;
}



htab_t *st_fun_call(htab_data_t *data, tStack *s, htab_t *global_ST){
    /* Create ST for function */
    htab_t *local_ST = htab_init(HTAB_SIZE);
    if (local_ST == NULL){
        return NULL;
    }
    data->data.fun_data.localST = local_ST;
    local_ST->globalST = global_ST;

    if (!StackPush(s, (void *) local_ST)){
        return NULL;
    }
    return local_ST;
}



htab_t *st_fun_return(tStack *s){

    htab_t *ST = StackTop(s);
    htab_free(ST);
    StackPop(s);
    return StackTop(s);
}




htab_data_t *st_var_create(htab_t *t, tToken token){

    htab_data_t *data_ST = htab_find(t, token.data.STRINGval->data);

    if (data_ST != NULL){
        return data_ST;
    }
    else{
        /* Add to global table */
        data_ST = htab_lookup_add(t, token.data.STRINGval->data);
        if (data_ST == NULL){
            return NULL;
        }

        /* Free token string */
        dynamicBufferFREE(token.data.STRINGval);

        /* Set init to true */
        data_ST->data.var_data.init = true;

        return data_ST;
    }
}



void st_var_set(htab_data_t *data, tToken token){

    data->data.var_data.type = token.type;

    if (token.type == T_STRING){
        dynamicBufferFREE(token.data.STRINGval);
    }
}

/*
void st_fun_free(){
    if (item_to_del->data->data.fun_data.declared && !item_to_del->data->data.var_data.init){
        while (!StackIsEmpty(item_to_del->data->data.fun_data.TaV)){
            free(((tVar_TaV *)StackTop(item_to_del->data->data.fun_data.TaV))->var);
            free(((tVar_TaV *)StackTop(item_to_del->data->data.fun_data.TaV)));
            StackPop(item_to_del->data->data.fun_data.TaV);
        }                
    }
}
*/

int main(){



    return 0;
}