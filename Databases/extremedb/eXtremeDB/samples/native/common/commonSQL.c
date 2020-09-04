/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include "commonSQL.h"

const char * sql_ret_string( MCO_RET rc )
{
  switch ( (status_t)rc ) 
  {
    case SQL_OK:                return "SQL_OK - successful SQL execution";
    case NO_MORE_ELEMENTS:      return "NO_MORE_ELEMENTS - end of result set reached";

    case INVALID_TYPE_CAST:     return "INVALID_TYPE_CAST - attempt to cast to an invalid SQL type";
    case COMPILE_ERROR:         return "COMPILE_ERROR - error compiling SQL statement";
    case NOT_SINGLE_VALUE:      return "NOT_SINGLE_VALUE - result set with more than one row returned";
    case INVALID_OPERATION:     return "INVALID_OPERATION - not a valid SQL operation";
    case INDEX_OUT_OF_BOUNDS:   return "INDEX_OUT_OF_BOUNDS - attempt to access outside of array bounds";
    case NOT_ENOUGH_MEMORY:     return "NOT_ENOUGH_MEMORY - out of memory executing SQL statement";
    case NOT_UNIQUE:            return "NOT_UNIQUE - attempt to insert a record with duplicate identifier value";
    case NOT_PREPARED:          return "NOT_PREPARED - attempt to execute a pre-compiled statement that was not prepared";
    case RUNTIME_ERROR:         return "RUNTIME_ERROR - unknown SQL runtime error";
    case COMMUNICATION_ERROR:   return "COMMUNICATION_ERROR - unknown communication error";
    case UPGRAGE_NOT_POSSIBLE:  return "UPGRAGE_NOT_POSSIBLE - unable to upgrade transaction to READ_WRITE";
    case SQL_CONFLICT:          return "SQL_CONFLICT - SQL conflict while updating a record";
    case SQL_NULL_REFERENCE:    return "SQL_NULL_REFERENCE - dereferencing NULL referenecs";
    case SQL_INVALID_STATE:     return "SQL_INVALID_STATE - invalid executor state";
    case SQL_INVALID_OPERAND:   return "SQL_INVALID_OPERAND - invalid function operand";
    case SQL_NULL_VALUE:        return "SQL_NULL_VALUE - null value in context not accepting null values";
    case SQL_BAD_CSV_FORMAT:    return "SQL_BAD_CSV_FORMAT - wrang format of CSV file";
    case SQL_SYSTEM_ERROR:      return "SQL_SYSTEM_ERROR - internal SQL engine error";
    default: return "";
  }

  return "";
}

void sample_sql_rc_check(const char * msg, MCO_RET rc)
{
  const char * sql_error_msg = sql_ret_string( rc );
  if ( MCO_S_OK == rc )
    printf("\n%s : Successful", msg );
  else if ( NO_MORE_ELEMENTS == (status_t)rc )
    printf("\n%s : End of result set", msg );
  else if ( 0 < strlen( sql_error_msg ) )
    printf("\n%s : \n\tReturn Code %d: %s\n", msg, (unsigned int)rc, sql_error_msg );
  else
    printf("\n%s : \n\tReturn Code %d: %s\n", msg, (unsigned int)rc, mco_ret_string( rc, 0 ) );
}

