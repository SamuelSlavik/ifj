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
#include "scanner.h"
#include "stack.h"


#define AVG_LEN_MIN 0.5
#define AVG_LEN_MAX 2


#define HTAB_SIZE 997               //99991


typedef struct htab htab_t;


/**
 * @brief Hash tabe structure
 * 
 */
struct htab{
    size_t size;                    // current size of HT
    size_t arr_size;                // allocated size
    struct htab_item **arr_ptr;     // pointer to list of synonyms
    htab_t *globalST;               // pointer to Global hash table
};


typedef const char *htab_key_t;


/**
 * @brief Data for variables
 * 
 */
typedef struct data_var
{
    bool init;                      // true if variable was initialized false if not
    enum token_type type;           // type of variable
} tData_var;


/**
 * @brief tuple variable type and variable name
 * 
 */
typedef struct fun_TaV
{
    enum token_type var_type;       // type of variable
    char *var;                      // variable name
} tVar_TaV;


/**
 * @brief Data for functions
 * 
 */
typedef struct data_fun
{
    size_t number_of_params;
    bool declared;                  // true if function was declared false if not
    bool defined;                   // true if function was defined false if not
    tStack *TaV;                    // stack of types and variables in parameter of function
    enum token_type return_type;    // return type of function
    htab_t *localST;                // pointer to local symbol table for this function
    tDynamicBuffer *label_name;
} tData_fun;


/**
 * @brief Union for data variable or function
 * 
 */
typedef union htab_data_type
{
    tData_var var_data;
    tData_fun fun_data;

} htab_data_type_t;


/**
 * @brief Hash table data item structure
 * 
 */
typedef struct htab_data {
    htab_key_t key;
    bool isfun;                     // set to true if there are function data
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
 * @return htab_pair_t* pointer to new item or existing item with same key or NULL if allocation fail
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
void htab_clear(htab_t * t);


/**
 * @brief Hash table deletion
 * 
 * @param t hash table
 */
void htab_free(htab_t * t);


/******************************************/
/********* SYMBOL TABLE FUNCTION **********/
/******************************************/


/**
 * @brief Create and initialize data type for storing information about
 * functions in symbol table
 * 
 * @param global_ST global symbol table
 * @param fun_name name of fanction => this is also key to hash table 
 * @return htab_data_t* pointer to new data or NULL if fail
 */
htab_data_t *st_fun_create(htab_t *global_ST, char *fun_name);


/**
 * @brief Allocate memory for tuple of function parameters(type & name) 
 * and initialize type of function
 * 
 * @param s pointer to stack of parameters
 * @param type of parameter to be set
 * @return true if success
 * @return false if fail
 */
bool st_fun_param_type(tStack *s, enum token_type type);


/**
 * @brief Set parameter name of function
 * 
 * @param s pointer to stack of parameters
 * @param name of parameter to be set
 * @return true if success
 * @return false if fail
 */
bool st_fun_param_name(tStack *s, char *name);



/**
 * @brief Set return type of function
 * 
 * @param data pointer to function data
 * @param type return type of function to be set
 */
void st_fun_retrun_type(htab_data_t *data, enum token_type type);


/**
 * @brief Set function definition to true
 * 
 * @param data pointer to function data
 */
void st_fun_definition(htab_data_t *data);


/**
 * @brief Create new symbol table for function.
 * New frame (symbol table) will have pointer to global table
 * 
 * @param global_ST global symbol table
 * @param s pointer to Stack of frames (local symbol tables)
 * @param fun_name the name of function for which frame is set
 * @return htab_t* pointer to new local symbol table or NULL if fail
 */
htab_t *st_fun_table_create(htab_t *global_ST, char *fun_name);


/**
 * @brief Close fram on top of the stack frame and free local symbol table
 * 
 * @param s pointer to Stack of frames (local symbol tables)
 * @return htab_t* pointer to next local symbol table or NULL stack is empty
 */
htab_t *st_fun_return(tStack *s);


/**
 * @brief Create and initialize data type for storing information about
 * variables in symbol table
 * 
 * @param t symbol table
 * @param name of variable
 * @return htab_data_t* pointer to new data or NULL if fail
 */
htab_data_t *st_var_create(htab_t *t, char *name);


/**
 * @brief Set type of variable. If variable with given name 
 * does not exist do nothing
 * 
 * @param t symbol table
 * @param name of variable which type will be set
 * @param type of variable that will be set
 */
void st_var_set(htab_t *t, char *name, enum token_type type);


/**
 * @brief Free function parameters, stack of parameters and symbol table
 * 
 * @param data pointer to data
 */
void st_fun_free(htab_data_t *data);



#endif // HTAB_H