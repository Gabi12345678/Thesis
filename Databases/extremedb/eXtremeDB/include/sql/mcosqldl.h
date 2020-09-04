/*******************************************************************
 *                                                                 *
 *  exceptions.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __MCOSQLDL_H__
#define __MCOSQLDL_H__


#ifdef _WIN32
#  ifdef MCOSQL_DLL
#    define MCOSQL_API __declspec(dllexport)
#  else 
#    ifdef MCOSQL_USE_DLL
#      define MCOSQL_API __declspec(dllimport)
#    else
#      define MCOSQL_API 
#    endif
#  endif
#else
#  define MCOSQL_API
#endif

#ifndef MCORSQL_API
# ifdef _WIN32
#  ifdef MCORSQL_DLL
#    define MCORSQL_API __declspec(dllexport)
#  else
#    ifdef MCORSQL_USE_DLL
#       define MCORSQL_API __declspec(dllimport)
#    else
#       define MCORSQL_API 
#    endif
#  endif
# else
#  define MCORSQL_API
# endif
#endif


#endif

