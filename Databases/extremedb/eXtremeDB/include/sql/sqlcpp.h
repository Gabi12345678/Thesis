/*******************************************************************
 *                                                                 *
 *  sqlcpp.h                                                       *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __SQL_H__
#define __SQL_H__

#include "dbapi.h"
#include "mcotime.h"
#include "mcosmp.h"

namespace McoSql
{

    class HashTable;
    class SymbolTable;
    class SqlEngine;
    class ExprNode;
    class StmtNode;
    class ExecuteNode;
    class FuncCallNode;
    class TransactionNode;
    class Server;
    class Runtime;
    class CreateDomainNode;
    class RemoteSqlEngine;
    class DistributedSqlEngine;
    class DataSourceStub;
    class DatabaseCatalog;
	class Sort;
	class SqlAggregator;
	class SyntheticTable;

    #define MCO_DATABASE_NAME "eXtremeDB"

    const size_t MAX_INDEX_VARS = 16;
    const size_t MAX_QUERY_ID_SIZE = 64;

    typedef int dsid_t;

    class RuntimeCursor : public Cursor
    {
      public:
        Runtime* const runtime;
        RuntimeCursor(Runtime* _runtime);
    };

    class MCOSQL_API ResultSet : public DataSource
    {
        friend class RemoteSqlEngine;
        friend class DistributedSqlEngine;
        friend class DataSourceStub;

        class CursorImpl : public RuntimeCursor
        {
            ResultSet* ds;
            Cursor* cursor;
            Record* curr;
            Record* succ;
			CursorImpl* nextCursor; // list of opened cursors

        public:
            bool hasNext();
            Record* next();

            CursorImpl(Runtime* runtime, ResultSet* ds);
            ~CursorImpl();
            DESTROY(CursorImpl)
        };

        Runtime* runtime;
        DataSource* input;
		CursorImpl* cursors; // list of opened cursors
    public:
        Cursor* cursor(Runtime* runtime);
        Vector<Field>* fields();
        Cursor* records();
		void closeCursors();
        bool isNumberOfRecordsKnown();
        int64_t nRecords(Runtime* runtime);
		void cleanup();

		void extract(Record* rec, void* dst, size_t size, bool nullIndicators[] = NULL,
					 ExtractMode mode = emReferenceToBody);

        Runtime* getRuntime() {
            return runtime;
        }
        ResultSet(Runtime* runtime, DataSource* input);
        void destroy(Allocator* allocator);

    protected:
        ResultSet(Allocator* allocator, Runtime* runtime = NULL);
    };

    struct MCOSQL_API QueryStat {
        size_t        memory;
        size_t        result_size_rows;
        size_t        result_size_bytes;
        timer_unit    execution_time;
        timer_unit    transmission_time;

        static const size_t serialized_size = sizeof(int64_t) * 5 + 1;
        size_t serialize(char *buf, size_t size) const;
        bool deserialize(char *buf, size_t &size);
    };

    /**
     * Descriptor of parameter
     */
    struct ParamDesc
    {
        Type type;
        void* ptr;
        int* lenptr;
        bool indirectStr;
    };

    /**
     * Placeholder for prepared statement
     */
    class MCOSQL_API PreparedStatement: public DynamicObject
    {
        friend class SqlEngine;
        friend class RemoteSqlEngine;
        friend class DistributedSqlEngine;
        friend class SqlAggregator;
		friend class Compiler;
		friend class ExecuteNode;

        Allocator allocator;
        Vector<Value>* params;
        ParamDesc* paramDescs;
        int shardParam;
        String* sql;
        StmtNode* node;
        dsid_t dsid;
		ExprNode* parallelBy;

	  public:
        /**
         * Return information about result columns (for non-selectable statement returns NULL)
         */
        Vector<Field>* describeResultColumns();

        /**
         * Check if prepared node is select
         */
        bool isQuery() const;
        bool isReadOnly() const;

        /**
         * Get memory allocator
         */
        Allocator* getAllocator()
        {
            return &allocator;
        }

        PreparedStatement(): node(NULL) {}
        DESTROY(PreparedStatement)
    };

    typedef Value* (*SqlFunction)(Runtime* runtime, Vector<Value>* params);

	struct MCOSQL_API SqlFuncParamDesc
	{
		bool        in;        // IN parameter
		char const* name;      // parameter name, NULL indicates end of parameters list
		Type        type;      // parameter type
		Type        elemType;  // parameter element type
		int         precision; // NUMERIC precision
	};

    /**
     * Class for user-defined function which can be used in McoSQL
     */
    class MCOSQL_API SqlFunctionDeclaration: public DynamicObject
    {
        friend class SqlEngine;
        friend class Compiler;
		friend class DynamicTable;
        friend class FuncCallNode;
        friend class AggregateNode;
        friend class DatabaseCatalog;
    public:
        enum FuncArgs {
            FUNC_VARARG = -1,   /* function accepting varying number of arguments */
            FUNC_AGGREGATE = -2 /* function with two arguments which can be used for aggregate calculation: call of this function in as expression with single
                                 *  argument is tranformed to function(accumulator, current_value)
                                 */
        };

		enum FuncFlags {
			FUNC_VOLATILE = 1, /* Function value should not be cached */
			FUNC_DYNAMIC = 2   /* Function created through xSQL */
		};

        /**
         * Global function declaration constructor
         * User-defined function should receive specified number of arguments of <code>Value*</code> type and return result of
         * <code>Value*</code> type. Type of return result should be the same as declared or tpNull.
         * It is possible to return NULL, which is treated as return of SQL Null value.
         * @param type function return type
         * @param name function name
         * @param func pointer to the function
         * @param nArgs number of function arguments (if nArgs = FUNC_VARARG, then function accepts vector with varying
         * number of arguments, if nArgs == FUNC_AGGREGATE then function should accept two arguments, but can be used in expressoin just with one argument
         * @elemType type of array or sequence element
		 * @flags combination of FuncFlags
         */
        SqlFunctionDeclaration(Type type, char const* name, SqlFunction func, int nArgs = FUNC_VARARG, Type elemType = tpNull, int flags = 0);

        /**
         * Global function declaration constructor
         * User-defined function should receive specified number of arguments of <code>Value*</code> type and return result of
         * <code>Value*</code> type. Type of return result should be the same as declared or tpNull.
         * It is possible to return NULL, which is treated as return of SQL Null value.
         * @param name function name
         * @param func pointer to the function
		 * @param params array of parameter descriptors, terminates by parameter with NULL name
		 * @flags combination of FuncFlags
         */
        SqlFunctionDeclaration(char const* name, SqlFunction func, SqlFuncParamDesc const* params, int flags = 0);

        /**
         * Per-engine User function declaration constructor
         * User-defined function should receive specified number of arguments of <code>Value*</code> type and return result of
         * <code>Value*</code> type. Type of return result should be the same as declared or tpNull.
         * It is possible to return NULL, which is treated as return of SQL Null value.
         * @param type function return type
         * @param name function name
         * @param func pointer to the function
         * @param nArgs number of function arguments (if nArgs = FUNC_VARARG, then function accepts vector with varying
         * number of arguments, if nArgs == FUNC_AGGREGATE then function should accept two arguments, but can be used in expressoin just with one argument
         * @elemType type of array or sequence element
         * @param ctx user defined context associated with function
         */
        SqlFunctionDeclaration(Type type, char const* name, SqlFunction func, int nArgs, Type elemType, void* ctx, int flags);

        /**
         * Dynamically created UDF (using CREATE FUNCION statement)
         * User-defined function should receive specified number of arguments of <code>Value*</code> type and return result of
         * <code>Value*</code> type. Type of return result should be the same as declared or tpNull.
         * It is possible to return NULL, which is treated as return of SQL Null value.
         * @param type function return type
         * @param elemType function return element type (is used only for arrays)
         * @param retTableColumns columns of returned table
         * @param name function name
         * @param func pointer to the function
         * @param args vector of function arguments
         * @param engine SqlEngine in which this function is registered
         * @param ctx user defined context associated with function
         */
        SqlFunctionDeclaration(Type type, Type elemType, Vector<Field>* retTableColumns, char const* name, SqlFunction func, Vector<Field>* args, void* ctx, int flags);

        DESTROY(SqlFunctionDeclaration)

        Type getType() const { return type; }
        Type getElemType() const { return elemType; }
        char const* getName() const { return name; }
		Vector<Field>* getReturnColumns() const { return retColumns; }
        int getNumberOfArguments() const { return nArgs; }
		Vector<Field>* getArguments() const { return args; }
        void* getContext() const { return userCtx; }
		SqlFunction getFunction() const { return func; }

      private:
        Type const type;
        Type const elemType;
		Vector<Field>* const retColumns;
        const char* const name;
        int const nArgs;
        Vector<Field>* const args;
        SqlFunction const func;
		int const flags;
		SqlFuncParamDesc const* const desc;
        static SqlFunctionDeclaration* chain;
        SqlFunctionDeclaration* next;
		
	  public:
        void* userCtx;
		bool compiled;
    };

    #define SQL_DEF_FUNC1(type, elemType, name, func)                    \
        static Value* func##_proxy(Runtime* runtime, Vector<Value>* params) {  return func(&runtime->memory, params->items[0]); } \
        static SqlFunctionDeclaration func##_decl(type, name, &func##_proxy, 1, elemType)
    #define SQL_DEF_FUNC2(type, elemType, name, func) \
        static Value* func##_proxy(Runtime* runtime, Vector<Value>* params) {  return func(&runtime->memory, params->items[0], params->items[1]); } \
        static SqlFunctionDeclaration func##_decl(type, name, &func##_proxy, 2, elemType)
    #define SQL_DEF_FUNC3(type, elemType, name, func) \
        static Value* func##_proxy(Runtime* runtime, Vector<Value>* params) {  return func(&runtime->memory, params->items[0], params->items[1], params->items[2]); } \
        static SqlFunctionDeclaration func##_decl(type, name, &func##_proxy, 3, elemType)
    #define SQL_DEF_FUNC4(type, elemType, name, func) \
        static Value* func##_proxy(Runtime* runtime, Vector<Value>* params) {  return func(&runtime->memory, params->items[0], params->items[1], params->items[2], params->items[3]); } \
        static SqlFunctionDeclaration func##_decl(type, name, &func##_proxy, 4, elemType)
    #define SQL_DEF_FUNC5(type, elemType, name, func) \
        static Value* func##_proxy(Runtime* runtime, Vector<Value>* params) {  return func(&runtime->memory, params->items[0], params->items[1], params->items[2], params->items[3], params->items[4]); } \
        static SqlFunctionDeclaration func##_decl(type, name, &func##_proxy, 5, elemType)

    #if SQL_INTERPRETER_SUPPORT
    /**
     * Class for interactive SQL extension definition
     */
    class InteractiveSqlExtension: public DynamicObject
    {
        friend class SqlEngine;
    public:
        /**
         * Callback function prototype
         * @param engine reference to SqlEngine instance
         * @param nParams number of command parameters
         * @param params array with parsed command parameters (quote character is removed)
         * params[0] contains name of the command
         * @param in input stream as passed to SqlEngine::main method
         * @param out output stream as passed to SqlEngine::main method
         * @param err error stream as passed to SqlEngine::main method
         * @return <code>true</code> to continue interactive SQL loop, <code>false</code> to terminate
         */
        typedef bool(*command_t)(SqlEngine* engine, int nParams, char* params[], FILE*  &in, FILE*  &out, FILE*
                                 &err);

        /**
         * Constructor of interactive SQL extension
         * This mechanism can be used to implement database-specific commands for interactive SQL.
         * (SqlEngine::main method)
         * @param name command name
         * @param syntax command syntax
         * @param description command description
         * @param callback pointer to the function
         * @param minArgs minimum number of command arguments
         * @param maxArgs maximum number of command arguments
         */
        InteractiveSqlExtension(char const* name, char const* syntax, char const* description, command_t
                                callback, int minArgs, int maxArgs);

    private:
        InteractiveSqlExtension(){}
        DESTROY(InteractiveSqlExtension)

        static InteractiveSqlExtension* chain;
        InteractiveSqlExtension* next;

        char const* name;
        char const* syntax;
        char const* description;
        command_t callback;
        int minArgs;
        int maxArgs;
    };
    #endif

    /**
     * Tune parameter for SQL optimizer
     */
    class MCOSQL_API SqlOptimizerParameters
    {
    public:
        /**
         * Size of buffer used for batch insert
         */
        size_t batchInsertBufferSize;

        /**
         * Work memory limit (used by sort,...)
         */
        size_t workMemLimit;

		/**
		 * Number of file for shuffle join
		 */
		int nShuffleFiles;

        /**
         *  Preload UDF's on engine start
         */
        bool preloadUDF;

        /**
         * Default constructor setting default values of parameters
         */
        SqlOptimizerParameters();
    };

	/**
	 * Backup kind
	 */
	enum BackupKind
	{
		BackupKindAuto,
	    BackupKindSnapshot,
		BackupKindIncremental
    };

	/**
	 * Language API enumeration
	 */
	enum LanguageAPI
	{
  	    LuaAPI,
		PythonAPI,
		JavaAPI,
		CSharpAPI,
		LastLanguageAPI
	};

    /**
     * SQL engine
     * Only one instance of this class can be used at the same time.
     */
    class MCOSQL_API SqlEngine : public DynamicObject
    {
        friend class Server;
        friend class Runtime;
        friend class Compiler;
		friend class DatabaseCatalog;
        friend class CreateDomainNode;
        friend class CreateFunctionNode;
        friend class DropFunctionNode;
        friend class AlterTableNode;
        friend class StatisticTable;
        friend class IndexStatisticTable;
        friend class ConnectionInfoTable;
		friend class DistributedShuffleJoin;

    public:
        void* operator new(size_t size) {
            return ::operator new(size);
        }
        void operator delete(void* ptr) {
            ::operator delete(ptr);
        }
        void* operator new(size_t size, Allocator* allocator) {
            return allocator->allocate(size);
        }
        void operator delete(void* ptr, Allocator*){}

        /**
         * Check if engine is local or remote
         */
        virtual bool isRemote();

        /**
         * Check if engine is executing distributed query on some shard
         */
        virtual bool isShard();

        /**
         * Default constructor
         */
        SqlEngine();

        DESTROY(SqlEngine)
        virtual ~SqlEngine();

        /**
         * Open engine - this method loads database schema from underlying storage.
         * @param db implementation of underlying storage level
         * If this parameter is not specified,
         * then value of static field <code>Database::instance</code> will be used.
         * <code>OutOfMemory</code> exception will be thrown
         */
        virtual void open(Database* db = NULL);

        /**
         * Lookup table by name
         * @return located table or NULL if not found
         */
        Table* findTable(char const* name);

        /**
         * Lookup table by name
         * @return located table or NULL if not found
         */
        Table* findTable(String* name);

        /**
         * Lookup index by name
         * @return located table or NULL if not found
         */
        Index* findIndex(String* name);


        /**
         * Trigger trace output
         * @param enable if <code>true</code> then engine will output trace information about query execution
         */
        void trace(bool enable);

        /**
         * Trigger output of query execution progress information
         * @param enable if <code>true</code> then engine will show query execution progress
         */
        void showProgress(bool enable);

        /**
         * Print execyted commands
         * @param enable if <code>true</code> then engine will print executed commands
         */
        void echo(bool on);

        /**
         * Get current database connection
         * @return database
         */
        Database* database();

        /**
         * Execute SQL query statement with varying list of parameters.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * It is possible to specify varying list of parameters.
         * Use the following placeholders for parameters in query:
         * <table border>
         * <tr><th>Placeholder</th><th>Parameter C type</th></tr>
         * <tr><td><code>%b</code></td><td><code>bool</code></td></tr>
         * <tr><td><code>%i</code></td><td><code>int</code></td></tr>
         * <tr><td><code>%i8</code></td><td><code>int64_t</code></td></tr>
         * <tr><td><code>%u</code></td><td><code>unsigned</code></td></tr>
         * <tr><td><code>%l</code></td><td><code>int64_t</code></td></tr>
         * <tr><td><code>%L</code></td><td><code>double (casted as int64_t)</code></td></tr>
         * <tr><td><code>%I1</code></td><td><code>size_t,int8_t*</code></td></tr>
         * <tr><td><code>%I2</code></td><td><code>size_t,int16_t*</code></td></tr>
         * <tr><td><code>%I4</code></td><td><code>size_t,int32_t*</code></td></tr>
         * <tr><td><code>%I8</code></td><td><code>size_t,int64_t*</code></td></tr>
         * <tr><td><code>%U1</code></td><td><code>size_t,uint8_t*</code></td></tr>
         * <tr><td><code>%U2</code></td><td><code>size_t,uint16_t*</code></td></tr>
         * <tr><td><code>%U4</code></td><td><code>size_t,uint32_t*</code></td></tr>
         * <tr><td><code>%U8</code></td><td><code>size_t,uint64_t*</code></td></tr>
         * <tr><td><code>%F4</code></td><td><code>size_t,float*</code></td></tr>
         * <tr><td><code>%F8</code></td><td><code>size_t,double*</code></td></tr>
         * <tr><td><code>%A</code></td><td><code>size_t nElems,size_t elemSize,char* body</code></td></tr>
         * <tr><td><code>%p</code></td><td><code>int64_t* (treated as reference)</code></td></tr>
         * <tr><td><code>%f</code></td><td><code>double</code></td></tr>
         * <tr><td><code>%t</code></td><td><code>mco_datetime</code></td></tr>
         * <tr><td><code>%s</code></td><td><code>char*</code></td></tr>
         * <tr><td><code>%w</code></td><td><code>wchar_t*</code></td></tr>
         * <tr><td><code>%v</code></td><td><code>Value*</code></td></tr>
         * <tr><td><code>%r</code></td><td><code>struct mapped to the record (record fields with string type should have "char*" type)</code></td></tr>
         * <tr><td><code>%R</code></td><td><code>struct mapped to the record (record fields with char&lt;N&gt; type should have "char[N]" type)</code></td></tr>
         * </table>
         * @param sql string with SQL statement
         * @return In case of queries, returns result data source. In all other cases returns NULL.
         */
        ResultSet* executeQuery(char const* sql, ...);

        /**
         * Execute SQL query statement with extracted list of parameters.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param sql string with SQL statement
         * @param params varying list of SQL statement parameters
         * @return In case of queries, returns result data source. In all other cases returns NULL.
         * @see DataSource* executeQuery(char const* sql, ...)
         */
        ResultSet* vexecuteQuery(char const* sql, va_list* params);

        /**
         * Execute SQL query statement with array of parameter values.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param sql string with SQL statement
         * This string can contains references
         * to array elements represented as %0...%9. There can be no more than 10 parameters.
         * @param params array of parameter values
         * @return In case of queries, returns result data source. In all other cases returns NULL.
         */
        ResultSet* vexecuteQuery(char const* sql, Value** params);

        /**
         * Execute SQL query statement with array of parameter values.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param sql string with SQL statement
         * This string can contains references
         * to array elements represented as %0...%9. There can be no more than 10 parameters.
         * @param params array of parameter values
		 * @params nParams number of parameters
         * @return In case of queries, returns result data source. In all other cases returns NULL.
         */
        ResultSet* vexecuteQuery(char const* sql, Value** params, size_t nParams);

        /**
         * Execute SQL update or delete statement with varying list of parameters.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * It is possible to specify varying list of parameters.
         * Use the following placeholders for parameters in query:
         * <table border>
         * <tr><th>Placeholder</th><th>Parameter C type</th></tr>
         * <tr><td><code>%b</code></td><td><code>bool</code></td></tr>
         * <tr><td><code>%i</code></td><td><code>int</code></td></tr>
         * <tr><td><code>%u</code></td><td><code>unsigned</code></td></tr>
         * <tr><td><code>%l</code></td><td><code>int64_t</code></td></tr>
         * <tr><td><code>%L</code></td><td><code>double (casted as int64_t)</code></td></tr>
         * <tr><td><code>%I1</code></td><td><code>size_t,int8_t*</code></td></tr>
         * <tr><td><code>%I2</code></td><td><code>size_t,int16_t*</code></td></tr>
         * <tr><td><code>%I4</code></td><td><code>size_t,int32_t*</code></td></tr>
         * <tr><td><code>%I8</code></td><td><code>size_t,int64_t*</code></td></tr>
         * <tr><td><code>%U1</code></td><td><code>size_t,uint8_t*</code></td></tr>
         * <tr><td><code>%U2</code></td><td><code>size_t,uint16_t*</code></td></tr>
         * <tr><td><code>%U4</code></td><td><code>size_t,uint32_t*</code></td></tr>
         * <tr><td><code>%U8</code></td><td><code>size_t,uint64_t*</code></td></tr>
         * <tr><td><code>%F4</code></td><td><code>size_t,float*</code></td></tr>
         * <tr><td><code>%F8</code></td><td><code>size_t,double*</code></td></tr>
         * <tr><td><code>%A</code></td><td><code>size_t nElems,size_t elemSize,char* body</code></td></tr>
         * <tr><td><code>%p</code></td><td><code>int64_t* (treated as reference)</code></td></tr>
         * <tr><td><code>%f</code></td><td><code>double</code></td></tr>
         * <tr><td><code>%t</code></td><td><code>mco_datetime</code></td></tr>
         * <tr><td><code>%s</code></td><td><code>char*</code></td></tr>
         * <tr><td><code>%w</code></td><td><code>wchar_t*</code></td></tr>
         * <tr><td><code>%v</code></td><td><code>Value*</code></td></tr>
         * <tr><td><code>%r</code></td><td><code>struct mapped to the record (record fields with string type should have "char*" type)</code></td></tr>
         * <tr><td><code>%R</code></td><td><code>struct mapped to the record (record fields with char&lt;N&gt; type should have "char[N]" type)</code></td></tr>
         * </table>
         * @param sql string with SQL statement
         * @return number of affected records or -1 if precise number is unknown
         */
        int64_t executeStatement(char const* sql, ...);

        /**
         * Execute SQL UPDATE or DELETE statement with extracted list of parameters.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param sql string with SQL statement
         * @param params varying list of SQL statement parameters
         * @return number of affected records or -1 if precise number is unknown
         * @see int executeStatement(char const* sql, ...)
         */
        int64_t vexecuteStatement(char const* sql, va_list* params);

        /**
         * Execute SQL update or delete statement with array of parameter values.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param sql string with SQL statement
         * This string can contain references
         * to array elements represented as %0...%9. There can be no more than 10 parameters.
         * @param params array of parameter values
         * @return number of affected records or -1 if precise number is unknown
         */
        int64_t vexecuteStatement(char const* sql, Value** params);

        /**
         * Execute SQL update or delete statement with array of parameter values.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param sql string with SQL statement
         * This string can contain references
         * to array elements represented as %0...%9. There can be no more than 10 parameters.
         * @param params array of parameter values
		 * @param nParams number of parameters
         * @return number of affected records or -1 if precise number is unknown
         */
        int64_t vexecuteStatement(char const* sql, Value** params, size_t nParams);

        /**
         * Execute SQL query statement with varying list of parameters.
         * Statement will be executed in the context of specified transaction.
         * It is possible to specify varying list of parameters.
         * Use the following placeholders for parameters in query:
         * <table border>
         * <tr><th>Placeholder</th><th>Parameter C type</th></tr>
         * <tr><td><code>%b</code></td><td><code>bool</code></td></tr>
         * <tr><td><code>%i</code></td><td><code>int</code></td></tr>
         * <tr><td><code>%u</code></td><td><code>unsigned</code></td></tr>
         * <tr><td><code>%l</code></td><td><code>int64_t</code></td></tr>
         * <tr><td><code>%L</code></td><td><code>double (casted as int64_t)</code></td></tr>
         * <tr><td><code>%I1</code></td><td><code>size_t,int8_t*</code></td></tr>
         * <tr><td><code>%I2</code></td><td><code>size_t,int16_t*</code></td></tr>
         * <tr><td><code>%I4</code></td><td><code>size_t,int32_t*</code></td></tr>
         * <tr><td><code>%I8</code></td><td><code>size_t,int64_t*</code></td></tr>
         * <tr><td><code>%U1</code></td><td><code>size_t,uint8_t*</code></td></tr>
         * <tr><td><code>%U2</code></td><td><code>size_t,uint16_t*</code></td></tr>
         * <tr><td><code>%U4</code></td><td><code>size_t,uint32_t*</code></td></tr>
         * <tr><td><code>%U8</code></td><td><code>size_t,uint64_t*</code></td></tr>
         * <tr><td><code>%F4</code></td><td><code>size_t,float*</code></td></tr>
         * <tr><td><code>%F8</code></td><td><code>size_t,double*</code></td></tr>
         * <tr><td><code>%A</code></td><td><code>size_t nElems,size_t elemSize,char* body</code></td></tr>
         * <tr><td><code>%p</code></td><td><code>int64_t*</code></td></tr>
         * <tr><td><code>%f</code></td><td><code>double</code></td></tr>
         * <tr><td><code>%t</code></td><td><code>mco_datetime</code></td></tr>
         * <tr><td><code>%s</code></td><td><code>char*</code></td></tr>
         * <tr><td><code>%w</code></td><td><code>wchar_t*</code></td></tr>
         * <tr><td><code>%v</code></td><td><code>Value*</code></td></tr>
         * <tr><td><code>%r</code></td><td><code>struct mapped to the record (record fields with string type should have "char*" type)</code></td></tr>
         * <tr><td><code>%R</code></td><td><code>struct mapped to the record (record fields with char&lt;N&gt; type should have "char[N]" type)</code></td></tr>
         * </table>
         * @param trans current transaction
         * @param sql string with SQL statement
         * @return In case of queries, returns result data source. In all other cases returns NULL.
         */
        ResultSet* executeQuery(Transaction* trans, char const* sql, ...);

        /**
         * Execute SQL query statement with extracted list of parameters.
         * Statement will be executed in the context of specified transaction.
         * @param trans current transaction
         * @param sql string with SQL statement
         * @param params varying list of SQL statement parameters
         * @return In case of queries, returns result data source. In all other cases returns NULL.
         * @see ResultSet* executeQuery(char const* sql, ...)
         */
        ResultSet* vexecuteQuery(Transaction* trans, char const* sql, va_list* params);

        /**
         * Execute SQL QUERY statement with array of parameter values.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param trans current transaction
         * @param sql string with SQL statement. This string can contain references
         * to array elements represented as %0...%9. There can be no more than 10 parameters.
         * @param params array of parameter values
         * @return In case of queries, returns result data source. In all other cases returns NULL.
         */
        ResultSet* vexecuteQuery(Transaction* trans, char const* sql, Value** params);

        /**
         * Execute SQL QUERY statement with array of parameter values.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param trans current transaction
         * @param sql string with SQL statement. This string can contain references
         * to array elements represented as %0...%9. There can be no more than 10 parameters.
         * @param params array of parameter values
		 * @param nParams number of parameters
         * @return In case of queries, returns result data source. In all other cases returns NULL.
         */
        ResultSet* vexecuteQuery(Transaction* trans, char const* sql, Value** params, size_t nParams);


        /**
         * Execute SQL UPDATE or DELETE statement with varying list of parameters.
         * Statement will be executed in the context of specified transaction.
         * It is possible to specify varying list of parameters.
         * Use the following placeholders for parameters in query:
         * <table border>
         * <tr><th>Placeholder</th><th>Parameter C type</th></tr>
         * <tr><td><code>%b</code></td><td><code>bool</code></td></tr>
         * <tr><td><code>%i</code></td><td><code>int</code></td></tr>
         * <tr><td><code>%u</code></td><td><code>unsigned</code></td></tr>
         * <tr><td><code>%l</code></td><td><code>int64_t</code></td></tr>
         * <tr><td><code>%L</code></td><td><code>double (casted as int64_t)</code></td></tr>
         * <tr><td><code>%I1</code></td><td><code>size_t,int8_t*</code></td></tr>
         * <tr><td><code>%I2</code></td><td><code>size_t,int16_t*</code></td></tr>
         * <tr><td><code>%I4</code></td><td><code>size_t,int32_t*</code></td></tr>
         * <tr><td><code>%I8</code></td><td><code>size_t,int64_t*</code></td></tr>
         * <tr><td><code>%U1</code></td><td><code>size_t,uint8_t*</code></td></tr>
         * <tr><td><code>%U2</code></td><td><code>size_t,uint16_t*</code></td></tr>
         * <tr><td><code>%U4</code></td><td><code>size_t,uint32_t*</code></td></tr>
         * <tr><td><code>%U8</code></td><td><code>size_t,uint64_t*</code></td></tr>
         * <tr><td><code>%F4</code></td><td><code>size_t,float*</code></td></tr>
         * <tr><td><code>%F8</code></td><td><code>size_t,double*</code></td></tr>
         * <tr><td><code>%A</code></td><td><code>size_t nElems,size_t elemSize,char* body</code></td></tr>
         * <tr><td><code>%p</code></td><td><code>int64_t*</code></td></tr>
         * <tr><td><code>%f</code></td><td><code>double</code></td></tr>
         * <tr><td><code>%t</code></td><td><code>mco_datetime</code></td></tr>
         * <tr><td><code>%s</code></td><td><code>char*</code></td></tr>
         * <tr><td><code>%w</code></td><td><code>wchar_t*</code></td></tr>
         * <tr><td><code>%v</code></td><td><code>Value*</code></td></tr>
         * <tr><td><code>%r</code></td><td><code>struct mapped to the record (record fields with string type should have "char*" type)</code></td></tr>
         * <tr><td><code>%R</code></td><td><code>struct mapped to the record (record fields with char&lt;N&gt; type should have "char[N]" type)</code></td></tr>
         * </table>
         * @param trans current transaction
         * @param sql string with SQL statement
         * @return number of affected records or -1 if precise number is unknown
         */
        int64_t executeStatement(Transaction* trans, char const* sql, ...);

        /**
         * Execute SQL UPDATE or DELETE statement with extracted list of parameters.
         * Statement will be executed in the context of specified transaction.
         * @param trans current transaction
         * @param sql string with SQL statement
         * @param params varying list of SQL statement parameters
         * @return number of affected records or -1 if precise number is unknown
         * @see int executeStatement(char const* sql, ...)
         */
        int64_t vexecuteStatement(Transaction* trans, char const* sql, va_list* params);

        /**
         * Execute SQL UPDATE or DELETE statement with array of parameter values.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param trans current transaction
         * @param sql string with SQL statement. This string can contain references
         * to array elements represented as %0...%9. There can be no more than 10 parameters.
         * @param params array of parameter values
         * @return number of affected records or -1 if precise number is unknown
         */
        int64_t vexecuteStatement(Transaction* trans, char const* sql, Value** params);

        /**
         * Execute SQL UPDATE or DELETE statement with array of parameter values.
         * Statement will be executed in separate transaction which will be automatically committed after the statement execution.
         * @param trans current transaction
         * @param sql string with SQL statement. This string can contain references
         * to array elements represented as %0...%9. There can be no more than 10 parameters.
         * @param params array of parameter values
		 * @param nParams number of parameters
         * @return number of affected records or -1 if precise number is unknown
         */
        int64_t vexecuteStatement(Transaction* trans, char const* sql, Value** params, size_t nParams);

        /**
         * Prepare statement using default allocator
         * It is possible to specify varying list of parameters.
         * Use the following placeholders for parameters in query:
         * <table border>
         * <tr><th>Placeholder</th><th>Parameter C type</th></tr>
         * <tr><td><code>%b</code></td><td><code>bool</code></td></tr>
         * <tr><td><code>%i</code></td><td><code>int</code></td></tr>
         * <tr><td><code>%u</code></td><td><code>unsigned</code></td></tr>
         * <tr><td><code>%l</code></td><td><code>int64_t</code></td></tr>
         * <tr><td><code>%L</code></td><td><code>double (casted as int64_t)</code></td></tr>
         * <tr><td><code>%I1</code></td><td><code>size_t,int8_t*</code></td></tr>
         * <tr><td><code>%I2</code></td><td><code>size_t,int16_t*</code></td></tr>
         * <tr><td><code>%I4</code></td><td><code>size_t,int32_t*</code></td></tr>
         * <tr><td><code>%I8</code></td><td><code>size_t,int64_t*</code></td></tr>
         * <tr><td><code>%U1</code></td><td><code>size_t,uint8_t*</code></td></tr>
         * <tr><td><code>%U2</code></td><td><code>size_t,uint16_t*</code></td></tr>
         * <tr><td><code>%U4</code></td><td><code>size_t,uint32_t*</code></td></tr>
         * <tr><td><code>%U8</code></td><td><code>size_t,uint64_t*</code></td></tr>
         * <tr><td><code>%F4</code></td><td><code>size_t,float*</code></td></tr>
         * <tr><td><code>%F8</code></td><td><code>size_t,double*</code></td></tr>
         * <tr><td><code>%A</code></td><td><code>size_t nElems,size_t elemSize,char* body</code></td></tr>
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
         * @param stmt prepared statement allocated by user
         * @param sql SQL statement text
         */
        virtual void prepare(PreparedStatement &stmt, char const* sql, ...);

        /**
         * Prepare statement using default allocator
         * @param stmt prepared statement allocated by user
         * @param sql SQL statement text
         * @param params varying list of SQL statement parameters
         */
        virtual void vprepare(PreparedStatement &stmt, char const* sql, va_list* list);

        /**
         * Prepare statement using default allocator
         * @param stmt prepared statement allocated by user
         * @param sql SQL statement text
         * @param params descriptors of parameters
         */
        virtual void vprepare(PreparedStatement &stmt, char const* sql, ParamDesc* params);

        /**
         * Execute parepared select statement
         * @param stmt statement prepared usign prepare method
         * @param trans current transaction (optional, if NULL then new transaction is created for execution of this statement)
         * @return result data source.
         */
        ResultSet* executePreparedQuery(PreparedStatement &stmt, Transaction* trans = NULL);

        /**
         * Execute parepared insert/update/delete statement
         * @param stmt statement prepared usign prepare method
         * @param trans current transaction (optional, if NULL then new transaction is created for execution of this statement)
         * @return number of affected records
         */
        int64_t executePreparedStatement(PreparedStatement &stmt, Transaction* trans = NULL);

        /**
         * Start new transaction.
         * @param mode transaction mode
         * @param prioroty transaction priority
         * @param level transaction isolation level
         * @return new transaction
         */
        virtual Transaction* beginTransaction(Transaction::Mode mode, int priority = 0, Transaction::IsolationLevel level = Transaction::DefaultIsolationLevel);

        /**
         * Register user function
         * User defined function should
         * receive from 0 to 4 arguments of <code>Value*</code> type and return result also of
         * <code>Value*</code> type. Type of return result should be the same as declared or tpNull.
         * It is also possible to return NULL, which will be treated as return of SQL Null value.
         * @param type function return type
         * @param name function name
         * @param func pointer to the function
         * @param nArgs number of function arguments (if nArgs == -1, then function accepts vector with varying
         * number of arguments)
         * @param ctx user defined context associated with function
         */
        void registerFunction(Type type, char const* name, SqlFunction func, int nArgs = SqlFunctionDeclaration::FUNC_VARARG, Type elemType = tpNull, void* ctx = NULL, int flags = 0);

        /**
         * Register dynamcically loaded user defined function
         * User defined function should
         * receive from 0 to 4 arguments of <code>Value*</code> type and return result also of
         * <code>Value*</code> type. Type of return result should be the same as declared or tpNull.
         * It is also possible to return NULL, which will be treated as return of SQL Null value.
         * @param type function return type
         * @param elemType function return element type (used only for arrays)
         * @param retTableColumns columns of returned table
         * @param name function name
         * @param func pointer to the function
         * @param args vectors with arguments descriptors
         * @param ctx user defined context associated with function
         */
        void registerFunction(Type type, Type elemType, Vector<Field>* retTableColumns, String* name, SqlFunction func, Vector<Field>* args, void* ctx = NULL, int flags = 0);


        /**
         * Unregister function with specified name
         * @param name function name
         */
        void unregisterFunction(char const* name);

#if SQL_INTERPRETER_SUPPORT
        /**
         * Register interactive SQL extension
         * This mechanism can be used to implement database-specific commands for interactive SQL.
         * (SqlEngine::main method)
         * @param name command name
         * @param syntax command syntax
         * @param description command description
         * @param callback pointer to the function
         * @param minArgs minimum number of command arguments
         * @param maxArgs maximum number of command arguments
         */
        void registerExtension(char const* name, char const* syntax, char const* description,
                               InteractiveSqlExtension::command_t callback, int minArgs, int maxArgs);

        /**
         * Parse command arguments and call interactive SQL extension if exist
         * This mechanism can be used to call database-specific commands for interactive SQL out of SqlEngine::main method
         * @param command the command line
         * @param in file descriptor for input stream
         * @param out file descriptor for output stream
         * @param err file descriptor for error stream
         * @return 0 - no command has been found, 1 - a command excepting 'exit' has been found, 2 - 'exit' command has been found
         */
        int callInteractiveExtension(char *command, FILE* &in, FILE* &out, FILE* &err);
#endif

        /**
         * Close engine.
         */
        virtual void close();

        /**
         * Output format for interactive SQL
         */
        enum OutputFormat
        {
            TEXT, HTML, XML, CSV, LIST
        };

        /**
         * Set output format for interactive SQL.
         * This format specifies how result of query will be represented.
         * @param format output format
         */
        void setOutputFormat(OutputFormat format);

        /**
         * Set delimiter for CSV output format
         * Overrides default comma delimiter
         * @param delimiter new delimiter character
         */
        void setCsvDelimiter(char delimiter);

        /**
         * Trigger printing the header for CSV output format
         * Enabled by default
         * @param enable if true show the CSV header
         */
        void showCsvHeader(bool enable);

        /**
         * Print column value to the specified buffer
         * @param buf destibation buffer
         * @param bufSize destination buffer size
         * @param v printed value
         * @return pointer to the zero terminated string (it may be placed in the buffer or be a string constant)
         */
        virtual char* printValue(char* buf, size_t bufSize, Value* v);

        #if SQL_INTERPRETER_SUPPORT
        /**
         * Interactive SQL
         * This method inputs SQL statements from specified input stream
         * and prints results to the specified output stream.
         * Engine should be previously opened. Control is returned from this function
         * only in case of closing input stream or execution of "exit" command.
         * Commands "trace on" and "trace off" can be used to toggle tracing option.
         * @param prompt interactive SQL prompt, if value of this parameter is <code>NULL</code> then
         *  prompt is not printed
         * @param in input stream to read SQL statement
         * @param out output stream to write results of SQL statement execution
         * @param err stream to print error messages
         */
        void main(char const* prompt, FILE* in, FILE* out, FILE* err);

        /**
         * Create default engine for interactive SQL.
         * This method initializes engine with default parameters.
         * Then it starts <code>main(stdin, stdout, stderr)</code> which reads requests from
         * standard input stream and write results to standard output.
         * When input stream is closed, engine is also closed and control is returned from this function.
         */
        static void defaultEngine();

        static bool helpCommand(SqlEngine* engine, int nParams, char* params[], FILE* &in, FILE* &out, FILE* &err);
        static bool logSessionCommand(SqlEngine* engine, int nParams, char* params[], FILE* &in, FILE* &out, FILE* &err);
        static bool seqFormatCommand(SqlEngine* engine, int nParams, char* params[], FILE* &in, FILE* &out, FILE* &err);
        static bool arrayFormatCommand(SqlEngine* engine, int nParams, char* params[], FILE* &in, FILE* &out, FILE* &err);
        static bool strFormatCommand(SqlEngine* engine, int nParams, char* params[], FILE* &in, FILE* &out, FILE* &err);

        #endif

        virtual ResultSet* vexecute(Transaction* trans, char const* sql, va_list* list, Value** array, size_t arraySize, int64_t& nRecords);

        ResultSet* vexecute(Transaction* trans, char const* sql, va_list* list, Value** array, int64_t& nRecords) {
			return vexecute(trans, sql, list, array, (size_t)~0, nRecords);
		}

        /**
         * Get memory allocator
         */
        Allocator* getAllocator()
        {
            return &allocator;
        }

        /**
         * Tune parameter for SQL optimizer
         */
        SqlOptimizerParameters optimizerParams;

        void attachRemoteEngine(SqlEngine* engine)
        {
            remoteEngine = engine;
        }


        virtual void printQueryStat(FILE *out);

        virtual void setInterrupted(bool enable);

        bool isInterrupted() {
            return interrupted;
        }

		/**
		 * Associate current thread with this engine instance.
		 * This method is invoked by SqlEngine constructor, so engine is implicitly associated with creator thread.
		 */
		void grab();

		/**
		 * Check if engine is accessed only by owner's thread (set by grab()).
		 */
		void checkOwner();

        void checkInterrupted();

	    virtual void backupCreate(String* file, String* label, BackupKind kind, int compressionLevel, String* cipherKey);
 	    virtual void backupRestore(String* file, String* label, String* cipherKey);
 	    virtual void backupVerify(String* file, String* label, String* cipherKey);
 	    virtual void backupInfo(String* file, SyntheticTable* backupInfo, Allocator* allocator);

		void setShards(int shardId, Array* hosts, Array* ports);
		virtual void fileReceived(String* fileName);
		virtual void ignoreQueryResult();

        SqlFunctionDeclaration* findFunction(char const* name, bool tryToLoad = false);

        bool traceEnabled;
        bool showProgressEnabled;
		bool autoCheckpoint;
		bool appendMode;
		bool implicitAggregates;
        bool transientSystables;
        DatabaseCatalog* catalog;
		String* likePattern;
		int stmtOffs;
		int stmtSize;
        Transaction::IsolationLevel defaultIsolationLevel;
        int defaultPriority;
		bool nullableByDefault;
	    void* languageAPICtx[LastLanguageAPI];

		int  nParallelWorkers;
		int  parallelWorkerId;
		SqlEngine* parallelEngine;

	  protected:
        struct CompilerContext {
            SymbolTable* symtab;
            HashTable* domainHash;
            bool loadingFunction;
        };

		class Shard : public DynamicObject
		{
		  public:
			String*     host;
			int         port;

			DESTROY(Shard);

			Shard(String* _host,  int _port) : host(_host), port(_port) {}
		};

		SqlEngine* getCoordinator() {
			return parent ? parent : this;
		}

        HashTable* userFuncs;
        HashTable* extensions;
        HashTable* preparedStatements;
        CompilerContext compilerCtx;
        Database* db;
        bool echoOn;
        OutputFormat format;
        char CSVDelimiter;
        bool showCSVHeader;
        bool alterTable;
        Allocator allocator;
        Transaction* currTrans;
        Transaction* compileTrans;
		bool transAborted;
        SqlEngine* remoteEngine;
        bool interrupted;
		int myShardId;
		Vector<Shard>* shards;
		SqlEngine* parent;
		mco_process_t owner_pid;
		mco_process_t owner_tid;

		void setDefaults();

		virtual SqlEngine* createParallelEngine();
		void setParallelWorkers(int nWorkers);

		virtual void startQuery(char const* sql);
		virtual void endQuery();
		virtual void updateConnectionInfo(SyntheticTable* table, Runtime* runtime);

		virtual void connectShards();
		virtual void writeToShard(int shardNo, String* fileName, String* data);
		virtual void waitShuffleCompletion(String* fileName);
		virtual void getQueryId(char* buf);

        void manageTransaction(Runtime* runtime, TransactionNode* tnode);
		void completeCompilation();

        void registerFunction(SqlFunctionDeclaration* decl);

        #if SQL_INTERPRETER_SUPPORT
        void registerExtension(InteractiveSqlExtension* decl);
        bool readStatement(char const* prompt, FILE*  &in, FILE*  &out, FILE*  &err, char* buf, size_t bufSize);
        int parseArguments(char* buf, char* cmd, char* args[]);

        void print(const char *format, ...);
        FormattedOutput outs[2]; // 0 - 'main', 1 - 'session'
        int nOuts;
        #endif

        // Listener of query execution state in the SqlEngine::main(), can be redefined in derived class.
        typedef enum { q_started, q_done, q_failed } query_state_t;
        virtual void onInteractiveQuery(query_state_t state, size_t nRecords, const char *sql, const char *errmsg) {}

        void checkSchema();
        virtual ResultSet* executePrepared(PreparedStatement &stmt, Transaction* trans, int64_t& nRecords);
        void preloadFunctions();
        virtual void init();        
        SqlEngine(Database* db, SqlEngine* engine);

    private:
	/*
	** copy constructor is disabled
	*/

    SqlEngine(const SqlEngine &e) {}

    };

    /**
     * Class used to control life area of ResultSet object
     * ResultSet returned by executeQuery method has to be released
     * when no longer needed. Forgetting to release data source can cause
     * application deadlock if transaction lock is not re-entered.
     * This class helps you remember to release data source -
     * release method is called in destructor. It also correctly releases
     * data source in case of thrown exceptions.<BR>
     * Intended usage:<BR>
     * <pre>
     * {
     *      QueryResult result(db-&gt;executeQuery("select * from T"));
     *      Cursor* cursor = result->records();
     *      while (cursor-&gt;hasNext()) {
     *          Record* rec = cursor->next();
     *      }
     *  }
     * </pre>
     */
    class QueryResult
    {
    public:
        QueryResult(ResultSet* src)
        {
            this->src = src;
        }

        ResultSet* source()
        {
            return src;
        }

        ResultSet* operator->()
        {
            return src;
        }

        ~QueryResult()
        {
            if (src != NULL) {
                src->release();
            }
        }

    private:
        ResultSet* src;
    };


	struct MCOSQL_API BMSearchContext
    {
        enum SearchKind {
			SK_UNDEFINED,
            SK_EQ,
            SK_SUBSTRING,
            SK_MATCH
        };
        SearchKind kind;
        int nWildcards;
        unsigned short shift[256];

        BMSearchContext() {
			kind = SK_UNDEFINED;
        }
    };

    class MCOSQL_API Runtime : public Resource
    {
        friend class SqlEngine;
        friend class RemoteSqlEngine;
        friend class DistributedSqlEngine;
        friend class Sort;
        friend class ResultSet;
        friend class FuncCallNode;
        friend class LoadColumnNode;
        friend class UpdateAtNode;
        friend class TargetNode;
        friend class UpdateNode;
        friend class DataSourceStub;
        friend class SqlAggregator;

      public:
        SqlEngine* const engine;
        Transaction* trans;
        Allocator memory;
        SqlFunctionDeclaration* currFunc;

        int indexVar[MAX_INDEX_VARS];
        BMSearchContext bmCtx;

        Value* getResult(dsid_t dsid) {
            return results->items[dsid];
        }

        void setResult(dsid_t dsid, Value* res) {
            assert(dsid != 0);
			if (dsid == targetDsid) {
				DELETE_OBJ(&memory, results->items[dsid]);
				res = (Record*)res->clone(&memory);
			}
            results->items[dsid] = res;
            resultCounts[dsid] += 1;
        }

        Runtime(SqlEngine* _engine) : Resource(&_engine->allocator), engine(_engine), trans(NULL), memory(&_engine->allocator), autoCommit(false), normalCompletion(false), resultCounts(NULL), results(NULL), targetDsid(-1), appendedVector(NULL) {
            if (engine) {
                engine->setInterrupted(false);
            }
        }

      private:
        bool autoCommit;
        bool normalCompletion;
        size_t* resultCounts;  // number of fetched rows for each data source: use both for correct work of cache and statistic
        Vector<Value>* results; // current data source record or cached function value
		dsid_t targetDsid;
		Array* appendedVector;

#if defined(__GNUC__) && defined (__GNUC_MINOR__) && (3 == __GNUC__ && 4 == __GNUC_MINOR__)
       public:
#endif
        void initialize(Transaction* trans, bool autoCommit, dsid_t nIds);

        ~Runtime();
#if defined(__GNUC__) && defined (__GNUC_MINOR__) && (3 == __GNUC__ && (4 == __GNUC_MINOR__ || 3 == __GNUC_MINOR__)) || defined(_AIX)
       public:
#endif
        DESTROY(Runtime)
    };



	/*
	 * Class from transaction with static scope
	 */
	class Work
	{
		Transaction* trans;

	  public:
		Work(SqlEngine* engine, Transaction::Mode mode, int priority = 0, Transaction::IsolationLevel level = Transaction::DefaultIsolationLevel)
		{
			trans = engine->beginTransaction(mode, priority, level);
		}

		bool commit()
		{
			assert(trans != NULL);
			bool ok = trans->commit();
			trans->release();
			trans = NULL;
			return ok;
		}

		void rollback()
		{
			assert(trans != NULL);
			trans->rollback();
			trans->release();
			trans = NULL;
		}

		~Work()
		{
			if (trans != NULL)
			{
				rollback();
			}
		}

	};
}

#endif
