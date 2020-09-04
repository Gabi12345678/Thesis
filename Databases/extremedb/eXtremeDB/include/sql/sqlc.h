/*******************************************************************
 *                                                                 *
 *  sqlc.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __CSQL_H__
    #define __CSQL_H__

    #include <stdio.h>
    #include "stdtp.h"
    #include "mcosqldl.h"

    #ifdef __cplusplus
        extern "C"
        {
        #endif

        typedef struct mco_sql* database_t;
        typedef struct mco_storage* storage_t;
        typedef struct sql_cursor* cursor_t;
        typedef struct data_source* data_source_t;
        typedef struct record* record_t;
        typedef struct blob_value* blob_t;
        typedef struct array_value* array_t;
        typedef struct struct_value* struct_t;
        typedef struct raw_value* raw_value_t;
        typedef struct transaction* transaction_t;
        typedef struct prepared_statement*  prepared_statement_t;

        /**
         * Column types
         */
        enum mcosql_column_type
        {
            CT_NULL, CT_BOOL, CT_INT1, CT_UINT1, CT_INT2, CT_UINT2, CT_INT4, CT_UINT4, CT_INT8, CT_UINT8, CT_REAL4,
            CT_REAL8, CT_TIME, CT_NUMERIC, CT_UNICODE, CT_STRING, CT_BINARY, CT_REFERENCE, CT_ARRAY, CT_STRUCT, CT_BLOB, CT_DATA_SOURCE, CT_LIST, CT_SEQUENCE
        };

        typedef enum mcosql_column_type type_t;

        /**
         * Transaction mode
         */
        enum mcosql_transaction_mode
        {
            TM_READ_ONLY, TM_UPDATE, TM_READ_WRITE, TM_EXCLUSIVE
        };

        /**
         * Transaction isolation level
         */
        enum mcosql_transaction_isolation_level
        {
            TL_DEFAULT, TL_READ_COMMITTED, TL_REPEATABLE_READ, TL_SERIALIZABLE
        };

        /**
         * Error codes
         */
        enum mcosql_error_code
        {
            SQL_OK, NO_MORE_ELEMENTS, INVALID_TYPE_CAST, COMPILE_ERROR, NOT_SINGLE_VALUE,
            INVALID_OPERATION, INDEX_OUT_OF_BOUNDS, NOT_ENOUGH_MEMORY, NOT_UNIQUE, NOT_PREPARED,
            RUNTIME_ERROR, COMMUNICATION_ERROR, UPGRAGE_NOT_POSSIBLE, SQL_CONFLICT,
            SQL_NULL_REFERENCE, SQL_INVALID_STATE, SQL_INVALID_OPERAND, SQL_NULL_VALUE,
            SQL_BAD_CSV_FORMAT, SQL_SYSTEM_ERROR
        };
        typedef enum mcosql_error_code status_t;

        typedef void *(*alloc_t)(size_t size);
        typedef void(*free_t)(void* ptr);

        /**
         * Initialize memory manager used by McoSQL and underlying storage implementation.
         * Dynamic memory manager allocates space on demand using alloc and free functions provided by user.
         * This function should be called before any other McoSQL function.
         * @param alloc memory allocation function
         * @param free memory de-allocation function
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_initialize_memory_manager(alloc_t alloc, free_t free);

        /**
         * Open database and load its metadata. Memory manager should be initialized first.
         * @param database pointer to the location to receive handle of the opened database
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_open(database_t* database);

        /**
         * Opens a database and loads the runtime metadata. Note that a memory
        manager must
         * be initialized before this function is called.
         * @param database a pointer to receive the handle of the opened database
         * @param storage an underlying storage implementating MCOAPI
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_open2(database_t* database, storage_t storage);


        /**
         * Start new transaction with default isolation level.
         * @param database handle to the database created by mcosql_open function
         * @param trans new transaction
         * @param mode transaction mode
         * @param priority transaction priority (0 - normal)
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_begin_transaction(database_t database, transaction_t* trans, enum mcosql_transaction_mode mode, int priority);

        /**
         * Start new transaction.
         * @param database handle to the database created by mcosql_open function
         * @param trans new transaction
         * @param mode transaction mode
         * @param priority transaction priority (0 - normal)
         * @param level transaction isolation level
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_begin_transaction_ex(database_t database, transaction_t* trans, enum mcosql_transaction_mode mode, int priority, enum mcosql_transaction_isolation_level level);


        /**
         * Commit current transaction.
         * @param trans transaction handle created by mcosql_begin_transaction
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_commit_transaction(transaction_t trans);

         /**
         * Commit specified phase of the current transaction.
         * @param trans transaction handle created by mcosql_begin_transaction
         * @param phase commin phase: 1 or 2
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_commit_transaction_phase(transaction_t trans, int phase);

        /**
         * Roll back current transaction.
         * @param trans transaction handle created by mcosql_begin_transaction
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_rollback_transaction(transaction_t trans);

        /**
         * Upgrade current transaction from TM_READ_ONLY to TM_READ_WRITE.
         * If transaction is already ReadWrite, then execution of this method has no effect.
         * @param trans transaction handle created by mcosql_begin_transaction
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_upgrade_transaction(transaction_t trans);

        /**
         * Release transaction.
         * This method is used to release transaction object and associated
         * resources when not needed anymore.
         * This function should be called for transactions explicitly created using mcosql_begin_transaction.
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_release_transaction(transaction_t trans);

        /**
         * Execute UPDATE, INSERT, DELETE or CREATE SQL statements with varying number of parameters.
         * Use the following placeholders for parameters in the query:
         * <table border>
         * <tr><th>Placeholder</th><th>Parameter C type</th></tr>
         * <tr><td><code>%b</code></td><td><code>uint1</code></td></tr>
         * <tr><td><code>%i</code></td><td><code>int</code></td></tr>
         * <tr><td><code>%u</code></td><td><code>unsigned</code></td></tr>
         * <tr><td><code>%l</code></td><td><code>int64_t</code></td></tr>
         * <tr><td><code>%p</code></td><td><code>int64_t* (treated as reference)</code></td></tr>
         * <tr><td><code>%f</code></td><td><code>double</code></td></tr>
         * <tr><td><code>%t</code></td><td><code>mco_datetime</code></td></tr>
         * <tr><td><code>%s</code></td><td><code>char*</code></td></tr>
         * <tr><td><code>%w</code></td><td><code>wchar_t*</code></td></tr>
         * <tr><td><code>%v</code></td><td><code>Value*</code></td></tr>
         * <tr><td><code>%r</code></td><td><code>struct mapped to the record (record fields with string type should have "char*" type)</code></td></tr>
         * <tr><td><code>%R</code></td><td><code>struct mapped to the record (record fields with char&lt;N&gt; type should have "char[N]" type)</code></td></tr>
         * </table>
         * @param database handle to the database created by mcosql_open function
         * @param trans transaction handle returned by mcosql_begin_transaction or <code>NULL</code> if
         * statement should be executed in auto-commit mode
         * @param n_records number of affected records (may be NULL)
         * @param sql string with SQL statement
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_execute_statement(database_t database, transaction_t trans, int64_t* n_records, char const* sql,
                                          ...);

        /**
         * Prepare statement with varying number of parameters.
         * Use the following placeholders for parameters in the query:
         * <table border>
         * <tr><th>Placeholder</th><th>Parameter C type</th></tr>
         * <tr><td><code>%b</code></td><td><code>bool</code></td></tr>
         * <tr><td><code>%i</code></td><td><code>int</code></td></tr>
         * <tr><td><code>%u</code></td><td><code>unsigned</code></td></tr>
         * <tr><td><code>%l</code></td><td><code>int64_t</code></td></tr>
         * <tr><td><code>%p</code></td><td><code>int64_t*</code></td></tr>
         * <tr><td><code>%f</code></td><td><code>double</code></td></tr>
         * <tr><td><code>%t</code></td><td><code>mco_datetime</code></td></tr>
         * <tr><td><code>%s</code></td><td><code>char*</code></td></tr>
         * <tr><td><code>%w</code></td><td><code>wchar_t*</code></td></tr>
         * <tr><td><code>%v</code></td><td><code>Value*</code></td></tr>
         * <tr><td><code>%r</code></td><td><code>struct mapped to the record (record fields with string type should have "char*" type)</code></td></tr>
         * <tr><td><code>%R</code></td><td><code>struct mapped to the record (record fields with char&lt;N&gt; type should have "char[N]" type)</code></td></tr>
         * <tr><td><code>%*b</code></td><td><code>bool*</code></td></tr>
         * <tr><td><code>%*i</code></td><td><code>int*</code></td></tr>
         * <tr><td><code>%*u</code></td><td><code>unsigned*</code></td></tr>
         * <tr><td><code>%*l</code></td><td><code>int64_t*</code></td></tr>
         * <tr><td><code>%*p</code></td><td><code>int64_t*</code></td></tr>
         * <tr><td><code>%*f</code></td><td><code>double*</code></td></tr>
         * <tr><td><code>%*t</code></td><td><code>mco_datetime*</code></td></tr>
         * <tr><td><code>%*s</code></td><td><code>char**</code></td></tr>
         * <tr><td><code>%*S</code></td><td><code>char*</code></td></tr>
         * <tr><td><code>%*w</code></td><td><code>wchar_t**</code></td></tr>
         * <tr><td><code>%*r</code></td><td><code>struct mapped to the record (record fields with string type should have "char*" type)</code></td></tr>
         * <tr><td><code>%*R</code></td><td><code>struct mapped to the record (record fields with char&lt;N&gt; type should have "char[N]" type)</code></td></tr>
         * </table>
         * @param database handle to the database created by mcosql_open function
         * @param stmt pointer to the prepared statement handle
         * @param sql string with SQL statement
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_prepare_statement(database_t database, prepared_statement_t* stmt, char const* sql, ...);

        /**
         * Execute prepared UPDATE, INSERT, DELETE or CREATE SQL statements.
         * @param database handle to the database created by mcosql_open function
         * @param trans transaction handle returned by mcosql_begin_transaction or <code>NULL</code> if
         * statement should be executed in auto-commit mode
         * @param stmt prepared_statement handle
         * @param n_records number of affected records (may be NULL)
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_execute_prepared_statement(database_t database, transaction_t trans, prepared_statement_t stmt, int64_t* n_records);

        /**
         * Execute prepared SELECT SQL statement
         * @param database handle to the database created by mcosql_open function
         * @param trans transaction handle returned by mcosql_begin_transaction or <code>NULL</code> if
         * query should be executed in auto-commit mode
         * @param stmt prepared_statement handle
         * @param data_source pointer to the location to receive data source handle
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_execute_prepared_query(database_t database, transaction_t trans, prepared_statement_t stmt, data_source_t* data_source);

        /**
         * Release prepared statement
         * @param database handle to the database created by mcosql_open function
         * @param stmt prepared_statement handle
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_release_prepared_statement(database_t database, prepared_statement_t stmt);

        /**
         * Execute SELECT SQL statement with varying number of parameters.
         * Use the following placeholders for parameters in the query:
         * <table border>
         * <tr><th>Placeholder</th><th>Parameter C type</th></tr>
         * <tr><td><code>%b</code></td><td><code>bool</code></td></tr>
         * <tr><td><code>%i</code></td><td><code>int</code></td></tr>
         * <tr><td><code>%u</code></td><td><code>unsigned</code></td></tr>
         * <tr><td><code>%l</code></td><td><code>int64_t</code></td></tr>
         * <tr><td><code>%p</code></td><td><code>int64_t* (treated as reference)</code></td></tr>
         * <tr><td><code>%f</code></td><td><code>double</code></td></tr>
         * <tr><td><code>%t</code></td><td><code>mco_datetime</code></td></tr>
         * <tr><td><code>%s</code></td><td><code>char*</code></td></tr>
         * <tr><td><code>%w</code></td><td><code>wchar_t*</code></td></tr>
         * <tr><td><code>%v</code></td><td><code>Value*</code></td></tr>
         * <tr><td><code>%r</code></td><td><code>struct mapped to the record (record fields with string type should have "char*" type)</code></td></tr>
         * <tr><td><code>%R</code></td><td><code>struct mapped to the record (record fields with char&lt;N&gt; type should have "char[N]" type)</code></td></tr>
         * </table>
         * @param database handle to the database created by mcosql_open function
         * @param trans transaction handle returned by mcosql_begin_transaction or <code>NULL</code> if
         * query should be executed in auto-commit mode
         * @param data_source pointer to the location to receive data source handle
         * @param sql  string with SQL statement
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_execute_query(database_t database, transaction_t trans, data_source_t* data_source, char const* sql,
                                      ...);


        /**
         * Get cursor to iterate through records in specified data source.
         * @param data_source handle of data source created by execute_query function
         * @param cursor pointer to the location to receive cursor handle
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_cursor(data_source_t data_source, cursor_t* cursor);

        /**
         * Get number of columns in data source.
         * @param data_source handle of data source created by execute_query function
         * @param n_columns pointer to the location to receive number of columns in data source
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_number_of_columns(data_source_t data_source, size_t* n_columns);

        /**
         * Get column information.
         * @param data_source handle of data source created by execute_query function
         * @param iterator pointer to the location to receive handle of iterator through data source columns
         * @param col_no column number (0-based)
         * @param type pointer to the location to receive type of column
         * @param name pointer to the location to receive name of column
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_column_info(data_source_t data_source, size_t col_no, type_t* type, char** name);

        /**
         * Move record cursor to the next record. Initially cursor is at the position before the first record.
         * @param cursor handle of the cursor created by get_cursor function
         * @param record pointer to the location to receive handle of record
         * This handle is valid only until
         * the next call of cursor_move_next function.
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_cursor_move_next(cursor_t cursor, record_t* record);

        /**
         * Extract record components to corresponding C struct.
         * The rules below describe the definition of C struct to be used as destination of this method:
         * <OL>
         * <LI>C struct should have exactly the same components in the same order as
         * database structure.
         * <LI>McoSQL assumes <I>default alignment</I> of all struct members - i.e.,
         * alignment used by the compiler without special align pragmas.
         * <LI>Array components are represented by pointer to <code>Array</code> value
         * <LI>String components are stored as zero-terminated ANSI string.
         * <LI>If component of the structure doesn't belong to any table and is the result of some
         * SELECT statement expression calculation, then its type is determined by the following rules:
         * <UL>
         * <LI>integer types (char, short, unsigned short, int,...)
         * are represented by int64_t type
         * <LI>floating point types (float, double) - by double type.
         * <LI>Other types are represented by themselves.
         * </UL>
         * <LI>Nested structures should be represented by the same C structs.
         * </OL>
         * @param data_source handle of data source created by execute_query function
         * @param record handle of the current record created by preceded call of cursor_move_next function
         * @param dst pointer to the C struct to receive components of the database structures
         * @param dst_size size of C struct - it should match the size of database record and is used only
         * for verification
         * @param null_indicators This is the array which elements will be set to <code>true</code> if
         * value of corresponding field is Null. If this array is not specified (is NULL), then
         * attempting to extract Null value of scalar field will cause RuntimeException.
         * This array should be large enough to collect indicators of all fields, in case of nested
         * structure it should contain an element for each component of this substructure.
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_extract_struct(data_source_t data_source, record_t record, void* dst, size_t dst_size,
                                       char null_indicators[]);


        /**
         * Get pointer at the value of the field of the current record. This value remains valid only until
         * mcosql_release_query_result function is executed.
         * @param record handle of the current record created by preceded call of cursor_move_next function
         * @param columnNo number of column (0 based)
         * @param type pointer to the location to receive type of column value (it is the same as type of
         * corresponding field or Null type)
         * @param buf pointer to the buffer to get value
         * @param buf_size buffer size in bytes
         * @param value_size if not null, receive actual of column value in bytess
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_column_value(record_t record, size_t columnNo, type_t* type, void* buf, size_t buf_size, size_t* value_size);


        /**
         * Get column value converted to the specified type. In case of string types,
         * only pointer to the zero terminated string will be placed in the buffer
         * and it remains valid only until mcosql_release_query_result function is executed.
         * @param record handle of the current record created by preceded call of cursor_move_next function
         * @param columnNo number of column (0 based)
         * @param type type of the extracted value
         * @param buf pointer to the buffer to get value
         * @param buf_size buffer size in bytes
         * @param value_size if not null, receive actual of column value in bytess
         * @return status of operation as described in error_code enum. In case of null value
         * of the column, SQL_NULL_VALUE is returned and buffer is not updated.
         */
        MCOSQL_API status_t mcosql_get_column_value_as(record_t record, size_t columnNo, type_t type, void* buf, size_t buf_size, size_t* value_size);

        /**
         * Get value of the structure or array field of the current record for update of
         * its components. This value remains valid only until
         * mcosql_release_query_result function is executed. This value can be used only
         * as SQL statement parameter (used with '%v' format placeholder).
         * @param record handle of the current record created by preceded call of cursor_move_next function
         * @param columnNo number of column (0-based)
         * @param type pointer to the location to receive type of column value (it is the same as type of
         * corresponding field or Null type)
         * @param value pointer to the location to receive pointer to the raw value
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_column_value_for_update(record_t record, size_t columnNo, type_t* type, raw_value_t value);

        /**
         * Set value of the column.
         * @param record handle of the current record created by preceded call of cursor_move_next function
         * @param columnNo number of column (0-based)
         * @param type type of value (as described in mcosql_column_type enum)
         * @param value pointer to the assigned value
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_set_column_value(record_t record, size_t columnNo, type_t type, void* value);

        /**
         * Release resources used by data source. This function should be called after the end
         * of work with data source (after iteration through all data source records).
         * @param data_source handle of data source created by execute_query function
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_release_query_result(data_source_t data_source);

        /**
         * Close database.
         * @param database handle to the database created by mcosql_open function
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_close(database_t database);

#if SQL_INTERPRETER_SUPPORT
        /**
         * Interactive SQL
         * This method reads SQL requests from input stream and writes results to output stream.
         * @param database handle to the database created by mcosql_open function
         * Control is returned from this function only in the case of closing input stream.
         * @param prompt interactive SQL prompt, if value of this parameter is <code>NULL</code> then
         *  prompt is not printed
         * @param in input stream to read SQL statement
         * @param out output stream to write results of SQL statements execution
         * @param err stream to print error messages
         */
        MCOSQL_API void mcosql_interpreter(database_t database, char const* prompt, FILE* in, FILE* out, FILE* err);
#endif

        /**
         * Get array element value.
         * @param array pointer to array object obtained using mcosql_get_column_value function
         * @param index index in the array
         * @param type pointer to the location to receive type of element value (it is the same as type of
         * corresponding array element or Null type)
         * @param buf pointer to the buffer to get value
         * @param buf_size buffer size in bytes
         * @param value_size if not null, receive actual of column value in bytess
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_array_element_value(array_t array, size_t index, type_t* type, void* buf, size_t buf_size, size_t* value_size);

        /**
         * Get array of structure element value for update of its components.
         * @param array pointer to array object obtained using mcosql_get_column_value function
         * @param index index in the array
         * @param type pointer to the location to receive type of element value (it is the same as type of
         * corresponding array element or Null type)
         * @param value pointer to the location to receive pointer to the value
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_array_element_value_for_update(array_t array, size_t index, type_t* type, raw_value_t value);

        /**
         * Set array element value.
         * @param array pointer to array object obtained using mcosql_get_column_value function
         * @param index index in the array
         * @param type of value (as described in mcosql_column_type enum)
         * @param value pointer to new element value
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_set_array_element_value(array_t array, size_t index, type_t type, void* value);

        /**
         * Get array length.
         * @param array pointer to array object obtained using mcosql_get_column_value function
         * @param length pointer to the location to receive array length
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_array_length(array_t array, size_t* length);

        /**
         * Set array length.
         * @param array pointer to array object obtained using mcosql_get_column_value function
         * @param length new array length
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_set_array_length(array_t array, size_t length);

        /**
         * Get array body: copy specified number of elements with offset to the buffer.
         * This method can be used only for arrays of scalar types.
         * @param array pointer to array object obtained using mcosql_get_column_value function
         * @param dst buffer to receive array elements
         * @param offs offset in array from which elements will be taken
         * @param len how much elements will be copied to the buffer
         * @return status of operation as described in error_code enum
         * @throws OutOfBounds exception if offs and len doesn't specify valid segment within array
         */
        MCOSQL_API status_t mcosql_get_array_body(array_t array, void* dst, size_t offs, size_t len);

        /**
         * Set array body: copy specified number of elements from buffer to the array with specified offset
         * This method can be used only for arrays of scalar types.
         * @param array pointer to array object obtained using mcosql_get_column_value function
         * @param src buffer with elements to be copied
         * @param offs offset in array from which elements will be stored
         * @param len how many elements will be copied from the buffer
         * @return status of operation as described in error_code enum
         * @throws OutOfBounds exception if offs and len doesn't specify valid segment within array
         */
        MCOSQL_API status_t mcosql_set_array_body(array_t array, void* src, size_t offs, size_t len);

        /**
         * Get structure component value.
         * @param s pointer to structure object obtained using mcosql_get_column_value function
         * @param index index of component
         * @param type pointer to the location to receive type of component value (it is the same as type of
         * corresponding record field or Null type)
         * @param buf pointer to the buffer to get value
         * @param buf_size buffer size in bytes
         * @param value_size if not null, receive actual of column value in bytess
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_struct_component_value(struct_t s, int index, type_t* type, void* buf, size_t buf_size, size_t* value_size);

        /**
         * Get structure or array component of structure for updates its components or elements.
         * @param s pointer to structure object obtained using mcosql_get_column_value function
         * @param index index of component
         * @param type pointer to the location to receive type of component value (it is the same as type of
         * corresponding record field or Null type)
         * @param value pointer to the location to receive pointer to the value
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_struct_component_value_for_update(struct_t s, size_t index, type_t* type, raw_value_t* value);

        /**
         * Set structure component value.
         * @param s pointer to structure object obtained using mcosql_get_column_value function
         * @param index index of component
         * @param type of value (as described in mcosql_column_type enum)
         * @param value pointer to the assigned value
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_set_struct_component_value(struct_t s, size_t index, type_t type, void* value);

        /**
         * Get number of components in the structure.
         * @param s pointer to structure object obtained using mcosql_get_column_value function
         * @param size pointer to the location to receive number of components
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_get_struct_size(struct_t s, size_t* size);

        /**
         * Return number of bytes available to be extracted.
         * It is not the total size of BLOB. It can be smaller than BLOB size, for
         * example, if BLOB consists of several segments, it can be size of segment.
         * @param blob pointer to BLOB obtained using mcosql_get_column_value function
         * @param size pointer to the location to receive number of bytes which can be read using one operation
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_blob_available_size(blob_t blob, size_t* size);

        /**
         * Copy BLOB data to the buffer. This method copies up to <code>buffer_size</code> bytes
         * from the current position in the BLOB to the specified buffer. Then, current position
         * is moved forward by number of fetched bytes.
         * @param blob pointer to BLOB obtained using mcosql_get_column_value function
         * @param buffer destination for BLOB data
         * @param buffer_size buffer size
         * @param n_bytes actual number of bytes tranfered. It can be smaller than <code>size</code>
         * if end of BLOB or BLOB segment is reached.
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_blob_get_data(blob_t blob, void* buffer, size_t buffer_size, size_t* n_bytes);

        /**
         * Append new data to the BLOB.
         * Append always performed at the end of BLOB and doesn't change current position for GET method.
         * @param blob pointer to BLOB obtained using mcosql_get_column_value function
         * @param buffer source of the data to be inserted in BLOB
         * @param size number of bytes to be added to BLOB
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_blob_append_data(blob_t blob, void const* buffer, size_t size);

        /**
         * Reset current position to the beginning of the BLOB.
         * @param blob pointer to BLOB obtained using mcosql_get_column_value function
         * @return status of operation as described in error_code enum
         */
        MCOSQL_API status_t mcosql_blob_reset(blob_t blob);

        /**
         * Free value previously obtained using mcosql_get_* functions
         */
        MCOSQL_API status_t mcosql_free_value(record_t record, raw_value_t value);

        #ifdef __cplusplus
        }
    #endif


#endif
