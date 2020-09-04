/*******************************************************************
 *                                                                 *
 *  multisql.h                                                     *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __MULTISQL_H__
#define __MULTISQL_H__

#include "sqlcpp.h"

namespace McoSql
{
    class Tuple;

    class SqlAggregatorImpl;

    class MCOSQL_API SqlAggregator: public SqlEngine
    {
        class AggregatedDataSource: public ResultSet
        {
            class CursorImpl : public RuntimeCursor
            {
                SqlAggregator* const supervisor;
                Tuple* succ;
                Tuple* curr;

              public:
                bool hasNext();
                Record* next();

                CursorImpl(Runtime* runtime, SqlAggregator* supervisor);
                DESTROY(CursorImpl)
            };
            SqlAggregator* const supervisor;

          public:
            virtual Vector<Field>* fields();
            virtual Cursor* cursor(Runtime* runtime);

            AggregatedDataSource(SqlAggregator* agg, Runtime* runtime)
            : ResultSet(&runtime->memory, runtime), supervisor(agg) {}
            void destroy(McoSql::Allocator* allocator);
        };

	  public:
		/**
		 * Create SQL aggregator: creates thread for each specified engine.
		 * @param _engines array of openned SQL engines.
		 * @param _nEngines number of elements in the array
		 */
		SqlAggregator(SqlEngine** _engines, size_t _nEngines, size_t bufferSize = 1024);


		/**
		 * Stop all threads and close all engines
		 */
		~SqlAggregator();

		DESTROY(SqlAggregator);

        void vprepare(PreparedStatement &stmt, char const* sql, va_list* list);
        void vprepare(PreparedStatement &stmt, char const* sql, ParamDesc* params);
        ResultSet* executePrepared(PreparedStatement &stmt, Transaction* trans, int64_t& nRecords);

        ResultSet* vexecute(Transaction* trans, char const* sql, va_list* list, Value** array, size_t arraySize, int64_t& nRecords);

		/**
		 * Stop all threads and close all engines
		 */
		void close();

        /**
         * Check if engine is local or remote
         */
        virtual bool isRemote();

        void setInterrupted(bool enable);

        friend class SqlAggregatorWorker;
        friend class SqlAggregatorImpl;

      protected:
        ResultSet* mergeResults(Runtime* runtime, char const* sql, DataSource* ds);

        SqlAggregatorImpl *impl;

        char const*     sql;
        Vector<Value>*  parameters;
        Runtime*        runtime;
		SqlEngine**     engines;
		size_t          nEngines;
    };

	class MCOSQL_API ParallelExecutor : public SqlAggregator
    {
		static SqlEngine** createWorkers(SqlEngine* coordinator, size_t nWorkers);
	  public:
		/**
		 * Create parallel executor: creates nWorkers sessions for engine each working in dedicaed thread.
		 * @param coordinator coordinator engine
		 * @param nWorkers number of laucnhed workers
		 */
		ParallelExecutor(SqlEngine* coordinator, size_t nWorkers, size_t bufferSize = 1024);
		~ParallelExecutor();
	};

}


#endif

