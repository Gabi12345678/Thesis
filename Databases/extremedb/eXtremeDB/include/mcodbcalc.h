/*******************************************************************
 *                                                                 *
 *  mcodbcalc.h                                                    *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *  eXtremeDB database calculator public API.                      *
 *                                                                 *
 ***************************************************************** */

/**
 * @file mcodbcalc.h
 * MCO database calculator
 */
#ifndef __MCODBCALC_H__
#define __MCODBCALC_H__

/*#include "mcolist.h"*/
#include <stddef.h>

#include "mco.h"
#include "mcotime.h"

#ifndef MCODBCALC_API
# ifdef _WIN32
#  ifdef MCODBCALC_DLL
#    define MCODBCALC_API __declspec(dllexport)
#  else
#    ifdef MCODBCALC_USE_DLL
#       define MCODBCALC_API __declspec(dllimport)
#    else
#       define MCODBCALC_API 
#    endif
#  endif
# else
#  define MCODBCALC_API
# endif
#endif


#ifdef __cplusplus
extern "C" {
#endif 

/* a simple link-list */

#ifndef offsetof
#define offsetof(st, m) ((char *)&((st *)(0))->m - (char *)&((st *)(0)))
#endif /* offsetof */


#define LIST_INIT(lst, node_type, node_name) \
												 do {                                                         \
												 (lst)->head.next = (lst)->head.prev = &((lst)->head);    \
												 (lst)->offset =                                          \
												 (unsigned long)offsetof(node_type, node_name);       \
												 } while (0)

#define LIST_GET_ITEM(lst, nptr)	((char *)(nptr) - (lst)->offset)
#define LIST_ISEND(list, node)		(&((list)->head) == node)
#define LIST_ISEMPTY(list)			((list)->head.next == (list)->head.prev)
#define LIST_HEAD(list)				((list)->head.next)
#define LIST_TAIL(list)				((list)->head.prev)
#define list_add2head(list, new)	list_add(LIST_HEAD(list), new)
#define list_add2tail(list, new)	list_add(LIST_TAIL(list), new)
#define list_delfromhead(list)		list_del(LIST_HEAD(list))
#define list_delfromtail(list)		list_del(LIST_TAIL(list))
#define list_for_each(liter, lst)    \
									for (liter = LIST_HEAD(lst);  \
									 !LIST_ISEND(lst, liter); liter = liter->next)
#define list_for_each_safe(liter, save, lst)      \
											for (liter = LIST_HEAD(lst), save = liter->next;      \
											!LIST_ISEND(lst, liter); liter = save, save = liter->next)

typedef struct list_node {
	struct list_node *next;
	struct list_node *prev;
} list_node;

typedef struct list {
	list_node head;       /**< Head element of the list */
	unsigned long offset; /**< An offset from list node to list item */
} list;

#define list_add(after, new)  \
									  do {                                          \
									  list_node *save, *tmp;                        \
									  \
									  tmp = (after);                                \
									  save = tmp->next;                             \
									  tmp->next = (new);                            \
									  tmp->next->prev = tmp;                        \
									  save->prev = tmp->next;                       \
									  tmp->next->next = save;                       \
									  } while(0)

#define list_del(n)        \
								do {                                    \
								list_node *save;                        \
								\
								save = (n)->prev;                       \
								(n) = (n)->next;                        \
								save->next = (n);                       \
								(n)->prev = save;                       \
								} while(0)
/* end of the link-lists */

/**
 * @def mco_calc_class_reg(calc, name, n)
 * @brief Allocate and register a class within a given calculator
 * @param calc - A pointer to calculator object
 * @param name - Class name(without quotes)
 * @param n    - Number of objects which will be inserted to database
 *
 * @return MCO_RET code
 * @see mco_calc_register_classes
 * @see MCO_CALC_DEFINE_CLASS
 */
#define mco_calc_class_reg(calc, name, n)                   \
    (__mco_calc_class_reg(calc, #name, name##_code, n))

/**
 * @def MCO_CALC_DEFINE_CLASS(name, n)
 * @brief Initialize a member in mco_cc_t array
 * @param name - Class name without quotes
 * @param n    - Number of objects which will be inserted to database
 * @see mco_calc_class_reg
 * @see mco_calc_register_classes
 * @see MCO_CALC_CLASSES_END
 */
#define MCO_CALC_DEFINE_CLASS(name, n)          \
    {                                           \
            .cc_name = #name,                   \
            .cc_code = name##_code,             \
            .cc_nobjects = n,                   \
            .cc_dyn = 0,                        \
            .cc_dynsize = 0,                    \
            .cc_nvecs = 0,                      \
            .cc_idx = 0,                        \
            .cc_nstrs = 0,                      \
            .cc_nblobs = 0,                     \
    }

/**
 * @def MCO_CALC_CLASSES_END
 * @brief Specifies the end of calculator classes list
 * @see MCO_CALC_DEFINE_CLASS
 */
#define MCO_CALC_CLASSES_END { NULL }

/**
 * @struct mco_cc_info_t
 * @brief This structure describes class memory resources in detales.
 *
 * mco_cc_info_t structure describes in detales all memory information
 * about given mco_cc_t class. 
 * @see mco_calc_get_class_info
 * @see mco_calc_cinfo_browse
 * @see mco_cc_t
 */
typedef struct mco_cc_info {
    int nobjs;       /**< Number of objects of a given class in database */
    int pages_min;   /**< Minimal number of pages given class uses */
    int pages_cur;   /**< Current number of used pages */
    int pages_max;   /**< Maximal number of pages given class can take */
    uint8 bytes_cur; /**< Current amount of used space */
    uint8 bytes_min; /**< Mininal space given class may use */
    uint8 bytes_max; /**< Maximal space given class may use */
    int nvecs;       /**< Number of vectors inside the class */
    int nstrs;       /**< Number of strings inside the class */
    int nblobs;      /**< Number of blobs inside the class*/
    int is_pers;     /**< True if class is persistent and false otherwice */
} mco_cc_info_t;

/**
 * @struct mco_cc_t
 * @brief Calculator class description
 *
 * Any classes defined in a given schema can be registered in the calculator
 * object attached to the particular database.
 * Each registered class is tied with dedicated mco_cc_t structure
 * wich will hold all necessary information about memory used by that class, its
 * indices and size of dynamic objects.
 * @see mco_calc_t
 * @see mco_cc_info_t
 * @see mco_calc_reg_schema_classes
 * @see mco_calc_class_add
 * @see MCO_CALC_DEFINE_CLASS
 */
typedef struct mco_cc {
    char *cc_name;     /**< Class name */ 
    uint2 cc_code;     /**< Class code */
    uint2 cc_dyn;      /**< True if this structure was dynamically allocated */    
    uint8 cc_dynsize;  /**< Number of bytes that can possibly add dynamic structures */
    int cc_nobjects;   /**< Number of objects to fill database */
    long cc_idx;       /**< This field is used to make unique index insides inside the class */
    list_node cc_node; /**< List node */
    timer_unit cc_time;
    void *_mem;
} mco_cc_t;

/**
 * @struct mco_calc_t
 * @brief MCO database calculator
 *
 * mco_calc_t is a structure which can be attached to given
 * database to calculate amount of space used by required classes.
 * Classes whose statistics need to be calculated must be explicitly
 * registered in the calculator. 
 * @see mco_cc_t - See calculator class description
 * @see mco_calc_init - See how calculator can be initialized.
 */
typedef struct mco_calc {
    mco_db_h db;            /**< Alive database handler */
    list cls;               /**< List of registered classes */
    int ncls;               /**< Number of registered classes */
    mco_dictionary_h dict;  /**< Database dictionary */
    long oid_val;           /**< Used to make unique oid values during database filling */
    uint2 pg_size;          /**< Database page size */
    uint2 ops;              /**< Optimal page size */
    uint4 total_pgs;        /**< Total number of pages in database */
    uint4 free_pgs;         /**< Number of free pages in database */    
    struct ds_length {
        int v_lmin;         /**< Min vectors length */
        int v_lmax;         /**< Max vectors length */
        int s_lmin;         /**< Min strings length */
        int s_lmax;         /**< Max strings length */
        int b_lmin;         /**< Min blobs length */
        int b_lmax;         /**< Max blobs length */        
    } dsl;                  /**< Holds a set of min and max limits of dynamic fields */
} mco_calc_t;

/**
 * @typedef void (*mco_calc_cinfo_h)(mco_calc_t *, mco_cc_t *, mco_cc_info_t *, void *)
 */
typedef void (*mco_calc_cinfo_h)(mco_calc_t *, mco_cc_t *, mco_cc_info_t *, void *);

/**
 * @typedef void (*mco_calc_iinfo_h)(mco_calc_t *, mco_index_stat_t *, void *)
 */
typedef void (*mco_calc_iinfo_h)(mco_calc_t *, mco_index_stat_t *, void *);

/**
 * @fn void mco_calc_init(mco_calc_t *calc, mco_dictionary_h dict)
 * @brief initialize a calculator object
 * @param calc - A pointer to calculator object
 * @param dict - Database dictionary
 * @see mco_calc_t
 */
MCODBCALC_API void mco_calc_init(mco_calc_t *calc, mco_dictionary_h dict);

/**
 * @fn MCO_RET mco_calc_reg_schema_classes(mco_calc_t *calc, mco_calculator_h ch)
 * @brief Register a set of calculator classes from schema
 *
 * If a given schema contains builtin calculator object with classes and number of
 * objects of each class calculator application will create this set of classes
 * can be registered automatically. Schema comiler should add @a <dbname>_get_calculator
 * function to the schema source file. This function returns builtin calculator object
 * @a mco_calculator_h.
 *
 * @param calc - a pointer to the mco_calc_t
 * @param ch   - builtin calculator object(mco_calculator_h) returned by <dbname>_get_calculator
 * @return MCO_RET code
 */
MCODBCALC_API MCO_RET mco_calc_reg_schema_classes(mco_calc_t *calc, mco_calculator_h ch);

/**
 * @fn MCO_RET __mco_calc_class_reg(mco_calc_t *calc, const char *cname, uint2 class_code, int n)
 * @see mco_calc_class_reg
 */
MCODBCALC_API MCO_RET __mco_calc_class_reg(mco_calc_t *calc, const char *cname, uint2 class_code, int n);

/**
 * @fn void mco_calc_class_add(mco_calc_t *calc, mco_cc_t *class)
 * @brief Add a given class @a class to the calculator list of classes
 * @param calc  - A pointer to the calculator object
 * @param class - A pointer to the class to add
 */
MCODBCALC_API void mco_calc_class_add(mco_calc_t *calc, mco_cc_t *class_descr);

/**
 * @fn void mco_calc_register_classes(mco_calc_t *calc, mco_cc_t *classes)
 * @brief Register all classes from the @a classes array in the calculator @a calc
 * @param calc    - A pointer to the calculator object
 * @param classes - An array of classes to register
 */
MCODBCALC_API void mco_calc_register_classes(mco_calc_t *calc, mco_cc_t *classes);

/**
 * @fn MCO_RET mco_calc_fill_db(mco_calc_t *calc)
 * @brief Fill the database with the given earlier number of objects of registered classes
 *
 * This function fills the database with objects of classes registered
 * in the calculator @a calc. Filling procedure guaranties unicity of
 * OIDs inside the database and hash indexes inside classes.
 *
 * @param calc - A pointer to the calculator object
 * @return MCO_RET code
 */
MCODBCALC_API MCO_RET mco_calc_fill_db(mco_calc_t *calc);

/**
 * @fn MCO_RET mco_calc_deinit(mco_calc_t *calc)
 * @brief Deinitialize calculator @a calc
 *
 * @NOTE this function must be called @a before database handler
 *       calculator owns will be disconnected from database
 * @param calc - A pointer to calculator object which will be deinitialized
 * @return MCO_RET code
 */
MCODBCALC_API MCO_RET mco_calc_deinit(mco_calc_t *calc);

/**
 * @fn MCO_RET mco_calc_stat_collect(mco_calc_t *calc)
 * @brief Collect memory statistics withing database calculator attached to
 * @param calc - A pointer the calculator object
 * @return MCO_RET code
 */
MCODBCALC_API MCO_RET mco_calc_stat_collect(mco_calc_t *calc);

/**
 * @fn MCO_RET mco_calc_get_class_info(mco_calc_t *calc, mco_cc_t *cls, mco_cc_info_t *info)
 * @brief Get information about memory used by class @a cls
 * @param calc - A pointer to calculator object
 * @param cls  - A pointer to class to collect information
 * @param info - Here information about class will be saved
 * @return MCO_RET code
 * @see mco_cc_info_t
 */
MCODBCALC_API MCO_RET mco_calc_get_class_info(mco_calc_t *calc, mco_cc_t *cls,
                                /* OUT */ mco_cc_info_t *info);

/**
 * @fn MCO_RET mco_calc_cinfo_browse(mco_calc_t *calc, mco_calc_cinfo_h handler, void *data)
 * @brief Browse through the registered in the calculator classes information
 *
 * This procedure browses through calculator classes, gets information about one
 * class at time and then calls @a handler function to give user an opportunity
 * to decide what to do with class information.
 *
 * @param calc    - A pointer to calculator object
 * @param handler - User defined handler function
 * @param data    - Private data
 *
 * @return MCO_RET code
 */
MCODBCALC_API MCO_RET mco_calc_iinfo_browse(mco_calc_t *calc, mco_calc_iinfo_h handler, void *data);

/**
 * @fn MCO_RET mco_calc_iinfo_browse(mco_calc_t *calc, mco_calc_iinfo_h handler, void *data)
 * @brief Browse through all indices in the database
 *
 * This procedure browses through database indices, gets information about one
 * index at time and then calls @a handler function to give user an opportunity
 * to decide what to do with index information.
 *
 * @param calc    - A pointer to calculator object
 * @param handler - User defined handler function
 * @param data    - Private data
 *
 * @return MCO_RET code
 */
MCODBCALC_API MCO_RET mco_calc_cinfo_browse(mco_calc_t *calc, mco_calc_cinfo_h handler, void *data);

#ifdef __cplusplus
}
#endif 

#endif /* __MCODBCALC_H__ */
