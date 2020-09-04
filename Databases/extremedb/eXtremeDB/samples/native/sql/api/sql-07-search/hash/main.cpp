/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "hashdb.h"

char sample_descr[] = 
"Sample 'hash' demonstrates SQL selects on unique and non-unique hash indexes.\n";
const char * db_name = "hashdb";

using namespace McoSql;

#define SERIES_SIZE     5
#define MAX_SQL_LENGTH  128

const int nRecords = 20;

// Define the structure correponding to database table Object
struct _Object
{
  uint4 iIdx;
  uint4 iSeries;
};

uint4 search_value = 3;

// Forward declarations
int addObject( McoSqlEngine& engine, _Object* o );
int queryObjects( McoSqlEngine& engine, char * sql );
void printObject( _Object o );

int main( int argc, char ** argv )
{
  McoSqlEngine engine;

  char sql[ MAX_SQL_LENGTH + 1 ];

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, hashdb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
  
  // Fill database
  printf("\n\tInsert 1000 test records ");
  for ( int i=0; i < nRecords; i++ ) {
    _Object o;
    o.iIdx = i;
    o.iSeries = ( ( nRecords - i ) / SERIES_SIZE) + 1;
    addObject( engine, &o );
  }

  // Show all objects
  queryObjects( engine, "select * from AnObject order by iIdx" );
  queryObjects( engine, "select * from AnObject order by iSeries" );

  // Objects with iIdx == search_value
  sprintf( sql, "select * from AnObject where iIdx = %u", search_value );
  queryObjects( engine, sql );
  sprintf( sql, "select * from AnObject where iSeries = %u", search_value );
  queryObjects( engine, sql );

  // Objects with iIdx <= search_value
  sprintf( sql, "select * from AnObject where iIdx <= %u", search_value );
  queryObjects( engine, sql );

  // Objects with iSeries > search_value
  sprintf( sql, "select * from AnObject where iSeries > %u", search_value );
  queryObjects( engine, sql );

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

int addObject( McoSqlEngine& engine, _Object* o )
{
  int ret = engine.executeStatement("insert into AnObject %r", o);
  if ( 0 == ret )
  {
    printf("\t\tError inserting AnObject %d\n", o->iIdx );
  }
  return ret;
}

int queryObjects( McoSqlEngine& engine, char * sql )
{
  printf( "\n\t%s\n", sql);
  QueryResult result( engine.executeQuery( sql ) );
  int count = 1;
  Cursor* cursor = result->records();
  while ( cursor->hasNext() )
  {
    Record* rec = cursor->next();
    _Object o;
    result->extract( rec, &o, sizeof(o) );
    printObject( o );
    count++;
  }
  return count-1;
}

void printObject( _Object o ) {
  printf("\t\tObject: iIdx=%u, iSeries=%u\n", o.iIdx, o.iSeries );
}
