/**
 * @file htab.h
 * @author Michal Ľaš (xlasmi00)
 * @brief 
 * @version 0.1
 * @date 2022-11-10
 * 
 */


#ifndef HTAB_H
#define HTAB_H

#include <string.h>
#include <stdbool.h>
#include "dynamic_buffer.h"
#include "scanner.h"


#define AVG_LEN_MIN 0.5
#define AVG_LEN_MAX 2


/**
 * @brief Hash tabe structure
 * 
 */
typedef struct htab{
    size_t size;
    size_t arr_size;
    struct htab_item **arr_ptr;
} htab_t;


typedef const tDynamicBuffer *htab_key_t;

/**
 * @brief Data for variables
 * 
 */
typedef struct data_var
{
    bool init;                      // true if variable was initialized false if not
} tData_var;


/**
 * @brief tuple variable type and variable name
 * 
 */
typedef struct fun_TaV
{
    enum token_type var_type;       // type of variable
    tDynamicBuffer *var;            // variable name
} tVar_TaV;


/**
 * @brief Data for functions
 * 
 */
typedef struct data_fun
{
    bool init;                      // true if function was initialized false if not
    bool defined;                   // true if function was defined false if not
    tVar_TaV *Ta_V[100];            // list of types and variables in parameter of function
    enum token_type return_type;    // return type of function
    htab_t *localST;                // pointer to local symbol table for this function
} tData_fun;


/**
 * @brief Union for data variable or function
 * 
 */
typedef union htab_data_type
{
    tData_var *var_data;
    tData_fun *fun_data;

} htab_data_type_t;


/**
 * @brief Hash table data item structure
 * 
 */
typedef struct htab_data {
    htab_key_t key;
    htab_data_type_t data;
} htab_data_t;


/**
 * @brief Hash table item structure
 * 
 */
typedef struct htab_item{
    htab_data_t *data;
    struct htab_item *next;
} htab_item_t;


/**
 * @brief Hashing function
 * 
 * @param str string that will be hashed
 * @return size_t index to hash table
 * from web: http://www.fit.vutbr.cz/study/courses/IJC/public/DU2.html 
 */
size_t htab_hash_function(htab_key_t str);


/**
 * @brief Function create and initialize new hash table
 * 
 * @param n size of hash table that will be allocated
 * @return htab_t* pointer to new hash table
 */
htab_t *htab_init(size_t n);


/**
 * @brief Function return number of hash table elements
 * 
 * @param t hash table
 * @return size_t number of hash table elements
 */
size_t htab_size(const htab_t * t);


/**
 * @brief Function return number of allocated elements
 * 
 * @param t hash table
 * @return size_t number of allocated elements
 */
size_t htab_bucket_count(const htab_t * t);


/**
 * @brief Function resize existing hash table if fail do nothing
 * 
 * @param t hash table 
 * @param newn new size
 */
void htab_resize(htab_t *t, size_t newn);


/**
 * @brief Function search for item in hash table
 * 
 * @param t hash table
 * @param key that will be searching
 * @return htab_pair_t* pointer to item with given key or NULL if key was not found
 */
htab_data_t * htab_find(htab_t * t, htab_key_t key);


/**
 * @brief Function for adding new item to hash table
 * 
 * @param t hash table
 * @param key item key
 * @return htab_pair_t* pointer to new item or existing item with same key
 */
htab_data_t * htab_lookup_add(htab_t * t, htab_key_t key);


/**
 * @brief Deletion of existing item
 * 
 * @param t hash table
 * @param key key of item that will be deleted
 * @return true if there is item with that key
 * @return false if there is not item with that key
 */
bool htab_erase(htab_t * t, htab_key_t key);


/**
 * @brief Deletion of all items in hash table
 * 
 * @param t hash table
 */
void htab_clear(htab_t * t);    // ruší všechny záznamy


/**
 * @brief Hash table deletion
 * 
 * @param t hash table
 */
void htab_free(htab_t * t);     // destruktor tabulky

#endif // HTAB_H