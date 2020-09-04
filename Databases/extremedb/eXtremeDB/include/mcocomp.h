#ifndef __MCOCOMP_H_LOADED___
#define __MCOCOMP_H_LOADED___

#ifdef __cplusplus
extern "C"
{
#endif 

/*
 *   Defines interface to mcocomp
 */

#include "mcowrap.h"

#ifdef _WIN32
#  ifdef MCOCOMPLIB_DLL
#    define MCOCOMPLIB_API __declspec(dllexport)
#  else
#    ifdef MCOCOMPLIB_USE_DLL
#       define MCOCOMPLIB_API __declspec(dllimport)
#    else
#       define MCOCOMPLIB_API 
#    endif
#  endif
#else
#  define MCOCOMPLIB_API
#endif

typedef struct
{
	mco_bool 		debug;
	mco_bool        nosort;
	mco_bool        dumpxml;
	mco_bool        genhpp;
	mco_bool        gencs;
	mco_bool        genjava;
	mco_bool        gensmartptr;
	const char     *outDir;
	const char     *csNamespace;
	const char     *javaPackage;
	mco_bool        cmode1;
	mco_bool        genXmlMethods;
	mco_bool        genJsonMethods;
	mco_bool        genSql;
	mco_bool        largeDatabase;
	int             wcharSize;
	mco_bool        use_prefix;
	const char *    include_dir;
	mco_bool 	    allCompact;
	mco_bool        persistent;
	mco_bool        transient;
	mco_bool        suppress_api;
	mco_bool        atomic;
} mcocomp_options_t;

typedef enum
{
	MCOCOMP_WARNING = 0,
	MCOCOMP_ERROR
} CompilerErrorType;

typedef void (*dicterrfn)(CompilerErrorType errtype, const char *msg, const char *file, int line, int pos, void *ctx);

MCOCOMPLIB_API MCO_RET mco_db_load_dictionary(const char *schema, mcocomp_options_t *comp_opts,
		mco_dict_h *ret_dict, size_t *dict_size,
		char *ret_dbname, size_t *ret_dbname_size,
		dicterrfn errfn, void *ctx);

#ifdef __cplusplus
}
#endif 
#endif
