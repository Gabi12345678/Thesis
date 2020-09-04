/*******************************************************************
 *                                                                 *
 *  exceptions.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include "mcosqldl.h"

#ifndef USE_STD_EXCEPTION
#define USE_STD_EXCEPTION 1
#endif

#if USE_STD_EXCEPTION
/* hack around qnx gcc 4.3.3 */
#if defined(_QNX)
#ifdef __cplusplus
#define tcpls __cplusplus
#endif
#undef __cplusplus
#define __cplusplus 0
#include "exception"
#undef __cplusplus
#ifdef tcpls
#define __cplusplus tcpls
#undef tcpls
#endif
#else
#include "exception"
#endif
#endif


namespace McoSql
{

    class String;

    /**
     * Root class for all McoSql exceptions
     */	
    class MCOSQL_API McoSqlException
    #if USE_STD_EXCEPTION
        : public std::exception
    #endif
    {
    public:
        enum ErrorCode
        {
            NO_SQL_ERROR, NO_MORE_ELEMENTS, INVALID_TYPE_CAST, COMPILE_ERROR, NOT_SINGLE_VALUE,
            INVALID_OPERATION, INDEX_OUT_OF_BOUNDS, NOT_ENOUGH_MEMORY, NOT_UNIQUE, NOT_PREPARED,
            RUNTIME_ERROR, COMMUNICATION_ERROR, UPGRADE_NOT_POSSIBLE, TRANSACTION_CONFLICT,
            NULL_REFERENCE_EXCEPTION, INVALID_STATE, INVALID_ARGUMENT, NULL_VALUE_EXCEPTION,
            CSV_FORMAT_EXCEPTION, SYSTEM_ERROR, SCHEMA_CHANGED, QUERY_INTERRUPTED, REMOTE_ERROR,
			SEQUENCE_OUT_OF_ORDER, END_OF_SET
        };

        String* getMessage() const {
			if (message == NULL) {
				((McoSqlException*)this)->message = createMessage();
			}
			return message;
		}

		virtual String* createMessage() const;
        virtual const char* what() const throw();

		McoSqlException(ErrorCode code, String* message = NULL);
		virtual~McoSqlException() throw();

        /**
         * Error code
         */
        ErrorCode code;
		String* message;
    };


    /**
     * Exception thrown in case of OS error
     */
    class SystemError : public McoSqlException
    {
    public:
        char const* file;
        int         line;
        int         errcode;

        /**
         * Get exception message.
         */
        virtual String* createMessage()const;

        SystemError(int errcode, char const* file, int line);
    };

    /**
     * Exception thrown during parsing of CSV file
     */
    class InvalidCsvFormat : public McoSqlException
    {
    public:
        String*     file;
        int         line;
        int         column;
        char const* msg;

        /**
         * Get exception message.
         */
        virtual String* createMessage()const;

        InvalidCsvFormat(String* file, int line, char const* msg);
        InvalidCsvFormat(String* file, int line, int column, char const* msg);
    };

    /**
     * Exception thrown when memory reserved for the SQL engine is exhausted
     */
    class NotEnoughMemory: public McoSqlException
    {
    public:
        /**
         * Amount of reserved space
         */
        size_t allocated;

        /**
         * Amount of used space
         */
        size_t used;

        /**
         * Requested number of bytes
         */
        size_t requested;

        /**
         * Get exception message.
         */
        virtual String* createMessage()const;

        NotEnoughMemory(size_t allocated, size_t used, size_t requested): McoSqlException(NOT_ENOUGH_MEMORY)
        {
            this->allocated = allocated;
            this->used = used;
            this->requested = requested;
        }
    };

    /**
     * Exception thrown when requested operation is invalid or not supported in this context
     */
	class MCOSQL_API InvalidOperation: public McoSqlException
    {
    public:
        InvalidOperation(char const* operation): McoSqlException(INVALID_OPERATION)
        {
            this->operation = operation;
        }

        virtual String* createMessage()const;

        /**
         * Name of the operation
         */
        char const* operation;
    };

    /**
     * Exception thrown when requested operation is possiblw in the current state
     */
    class MCOSQL_API InvalidState: public McoSqlException
    {
    public:
        InvalidState(char const* msg): McoSqlException(INVALID_STATE)
        {
            this->msg = msg;
        }

        virtual String* createMessage()const;

        /**
         * Error message
         */
        char const* msg;
    };

    /**
     * Exception thrown when argument value doesn't belong to the requested domain
     */
	class MCOSQL_API InvalidArgument: public McoSqlException
    {
    public:
        InvalidArgument(String* msg): McoSqlException(INVALID_ARGUMENT, msg) {}
        InvalidArgument(char const* msg);
    };

    /**
     * Exception thrown when requested type conversion cannot be performed
     */
    class MCOSQL_API InvalidTypeCast: public McoSqlException
    {
    public:
        InvalidTypeCast(char const* realType, char const* castType): McoSqlException(INVALID_TYPE_CAST)
        {
            this->realType = realType;
            this->castType = castType;
        }

        virtual String* createMessage()const;

        /**
         * Type of converter value
         */
        char const* realType;

        /**
         * Type to which value is converted
         */
        char const* castType;
    };

    /**
     * Runtime error during statement execution
     */
	class MCOSQL_API RuntimeError: public McoSqlException
    {
    public:
        RuntimeError(char const* message, ErrorCode code = RUNTIME_ERROR);
        RuntimeError(String* message): McoSqlException(RUNTIME_ERROR, message) {}
    };

    /**
     * Commnication failure
     */
    class MCOSQL_API CommunicationError: public McoSqlException
    {
    public:
        CommunicationError(char const* message, int errcode = 0): McoSqlException(COMMUNICATION_ERROR)
        {
            this->message = message;
            this->errorCode = errcode;
        }

        virtual String* createMessage()const;

        /**
         * Explanation message
         */
        char const* message;

        /**
         * Error code
         */
        int errorCode;
    };


    /**
     * Null reference is dereferenced
     */
    class MCOSQL_API NullReferenceException : public McoSqlException
    {
    public:
        NullReferenceException(int loopId =  - 1): McoSqlException(NULL_REFERENCE_EXCEPTION)
        {
            this->loopId = loopId;
        }
        virtual String* createMessage()const;

        /**
         * Loop identifier (used to implement EXISTS clause)
         */
        int loopId;
    };

    /**
     * Null reference is dereferenced
     */
    class MCOSQL_API NullValueException : public McoSqlException
    {
    public:
        NullValueException(): McoSqlException(NULL_VALUE_EXCEPTION)
        {
        }
        virtual String* createMessage()const;
    };

    /**
     * Array index out of bounds
     */
    class MCOSQL_API IndexOutOfBounds: public McoSqlException
    {
    public:
        IndexOutOfBounds(size_t index, size_t length, int loopId = -1): McoSqlException(INDEX_OUT_OF_BOUNDS)
        {
            this->index = index;
            this->length = length;
            this->loopId = loopId;
        }

        virtual String* createMessage()const;

        /**
         * Value of index
         */
        size_t index;

        /**
         * Array length
         */
        size_t length;

        /**
         * Loop identifier (used to implement EXISTS clause)
         */
        int loopId;
    };


    /**
     * Attempt to move an iterator to next element when element doesn't exist
     */
    class MCOSQL_API NoMoreElements: public McoSqlException
    {
    public:
        NoMoreElements(): McoSqlException(NO_MORE_ELEMENTS){}

        virtual String* createMessage()const;
    };

    /**
     * MVCC transaction conflict
     */
    class MCOSQL_API TransactionConflict: public McoSqlException
    {
    public:
        TransactionConflict(): McoSqlException(TRANSACTION_CONFLICT){}

        virtual String* createMessage()const;
    };

    /**
     * Attempt to insert duplicate value in unique index
     */
    class MCOSQL_API NotUnique: public McoSqlException
    {
		char const* indexName;
    public:
        NotUnique(char const* index = NULL): McoSqlException(NOT_UNIQUE), indexName(index) {}

        virtual String* createMessage()const;
    };

    /**
     * Attempt to execute not prepared statement
     */
    class MCOSQL_API NotPrepared: public McoSqlException
    {
    public:
        NotPrepared(): McoSqlException(NOT_PREPARED){}

        virtual String* createMessage()const;
    };


    /**
     * Subselect query used in expression returns more than one record
     */
    class MCOSQL_API NotSingleValue: public McoSqlException
    {
    public:
        NotSingleValue(char const* reason): McoSqlException(NOT_SINGLE_VALUE)
        {
            this->reason = reason;
        }

        virtual String* createMessage()const;

        /**
         * The problem details
         */
        char const* reason;
    };


    /**
     * Syntax or semantic error detected by compiler
     */
    class MCOSQL_API CompileError: public McoSqlException
    {
    public:
        /**
         * Error message
         */
        String* msg;

        /**
         * SQL query
         */
        char const* sql;

        /**
         * Position in string specifying SQL statement (0-based)
         */
        int pos;

        virtual String* createMessage()const;

        CompileError(String* msg, char const* sql, int pos): McoSqlException(COMPILE_ERROR)
        {
            this->msg = msg;
            this->sql = sql;
            this->pos = pos;
        }
        CompileError(char const* str, char const* sql, int pos);
		~CompileError() throw();
    };

    /**
     * Attempt to upgrade tranaction is failed
     */
    class MCOSQL_API UpgradeNotPossible: public McoSqlException
    {
    public:
        UpgradeNotPossible(): McoSqlException(UPGRADE_NOT_POSSIBLE){}

        virtual String* createMessage()const;
    };

    /**
     * Database schema was changed
     */
    class MCOSQL_API SchemaChanged: public McoSqlException
    {
    public:
        SchemaChanged(): McoSqlException(SCHEMA_CHANGED){}

        virtual String* createMessage()const;
    };

    /**
     * Query execution was interrupted
     */
    class MCOSQL_API QueryInterrupted : public McoSqlException
    {
    public:
        QueryInterrupted(): McoSqlException(QUERY_INTERRUPTED){}

        virtual String* createMessage()const;
    };

    /**
     * Exception delivered from remote node
     */
    class MCOSQL_API RemoteError : public McoSqlException
    {
    public:
        RemoteError(int node, String* message);
		int node;
    };

    /**
     *  Inserted sequence element is out of order
     */
    class MCOSQL_API SequenceOutOfOrder : public RuntimeError
    {
    public:
        SequenceOutOfOrder(): RuntimeError("Sequence item is out of order", SEQUENCE_OUT_OF_ORDER){}
    };


	/**
	 * Exception during INSERT FROM SELECT
	 */
	class MCOSQL_API CopyException : public McoSqlException
	{
	  public:
		char const* column;
		int64_t row;

		CopyException(char const* column, int64_t row, McoSqlException const& x);
	};


	class EndOfSetException : public McoSqlException
	{
    public:
        EndOfSetException(): McoSqlException(END_OF_SET) {}
	};
}

#endif

