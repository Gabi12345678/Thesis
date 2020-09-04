/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <sqlcpp.h>
#include <multisql.h>

#define DATABASE_SIZE 100*1024*1024 //100M

const int   n_engines = 3;
McoSql::SqlEngine *engines[n_engines];

void run_queries(McoSql::SqlAggregator &aggregator)
{
    try {
        printf("Insert data... \n");

        int rc = aggregator.executeStatement( "?:insert into MyTable (pk,value) values (%i,%s)", 2013, "Hello" );
        assert(1 == rc);

        rc = aggregator.executeStatement( "1:insert into MyTable (pk,value) values (%i,%s)", 2012, "Good bye" );
        assert(1 == rc);
 
        rc = aggregator.executeStatement( "?:insert into MyTable (pk,value) values (%i,%s)", 2011, "Cheer!" );
        assert(1 == rc);

        rc = aggregator.executeStatement( "insert into MyTable (pk,value) values (%i,%s)", 2010, "Word" );
        assert(n_engines == rc);

        rc = aggregator.executeStatement( "insert into MyTable (pk,value) values (%i,%s)", 2010, "Word" );
        assert(n_engines == rc);
    } catch (McoSql::McoSqlException &err) {
        printf( "\n\n*** insert has thrown exception '%s'\n", err.getMessage()->cstr() );
        exit (-1);
    }

    printf("Perform queries... \n");

    McoSql::QueryResult result( aggregator.executeQuery( "select pk,value from MyTable" ) );

    McoSql::Allocator allocator;

    int count = 0;
    McoSql::Cursor *cursor = result->records();
    while ( cursor->hasNext() )
    {
      McoSql::Record *rec = cursor->next();
        int pk = rec->getInt(0);
        const char *value = rec->get(1)->stdStringValue(&allocator).c_str();

        printf ("%i:%s\n", pk, value);
        count++;
    }
}

int main( int argc, char ** argv )
{
    int i;
    printf("Starting aggregator engines...\n");
    for (i = 0; i < n_engines; i++) {
        char engname[50];
        sprintf(engname, "eng%d", i);

        McoSqlOpenParameters ops(engname, 0, DATABASE_SIZE);

        ops.flags = McoSqlOpenParameters::ALLOCATE_MEMORY | McoSqlOpenParameters::SET_ERROR_HANDLER | McoSqlOpenParameters::INITIALIZE_DATABASE;
        ops.maxClasses = ops.maxIndexes = 10;
        ops.maxDictionarySize = 16*1024;

    McoSqlEngine *eng = new McoSqlEngine();
    
    try {
      eng->open(ops);
    } catch (McoSql::McoSqlException &err) {
      printf( "\n\n*** Open engine has thrown exception '%s'\n", err.getMessage()->cstr() );
      return -1;
    }

    printf("engine %d opened.\n", i);

        engines[i] = eng;
    }

    printf("Engines created, starting aggregator... \n");

    McoSql::SqlAggregator aggregator(engines, n_engines);

    try {
        printf("Aggregator started, creating tables... \n");
        int rc = aggregator.executeStatement(
            "create table MyTable (pk int, value string, u2 char, i1 tinyint, i2 smallint, i4 int, i8 bigint, flt float, dbl double, "
            "b boolean, vdate date, vdecimal decimal(10,2), blo blob, n_u2 char null, n_i1 tinyint null, n_i2 smallint null, n_i4 int null, n_i8 bigint null, "
            "n_flt float null, n_dbl real null, n_b bit null, n_vdate time null, n_vdecimal decimal(10,2) null, n_blo blob null, n_value string null)");
        assert(n_engines == rc);
    } catch (McoSql::McoSqlException &err) {
        printf( "\n\n*** ExecuteStatement has thrown exception '%s'\n", err.getMessage()->cstr() );
        exit (-1);
    }

    run_queries(aggregator);

    printf ("Closing aggregator.\n");
    aggregator.close();

    printf ("Closing engines.\n");
    for (i = 0; i < n_engines; i++) {
      engines[i]->close();
    }
    
    return 0;
}
