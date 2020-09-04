/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "searchdb.h"

char sample_descr[] = 
"Sample 'btree' demonstrates eXtremeSQL searches with a btree index.\n";
const char * db_name = "searchdb";

using namespace McoSql;

#define MAX_SQL_LENGTH  128

// Define the structure correponding to database table Object
struct _Object
{
  uint4 id;
  uint4 value;
};

uint4 values[] = { 1,2,2,3,3,3,4,4,5,5,5 };
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
  engine.open( db_name, searchdb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
  
  // Fill database
  for ( int i=0; i < (int)(sizeof( values ) / sizeof( values[0] )); i++ ) {
    _Object o;
    o.id = i;
    o.value = values[i];
    addObject( engine, &o );
  }

  // Show all objects
  queryObjects( engine, "select * from AnObject order by value" );

  // Show objects with value > search_value 
  sprintf( sql, "select * from AnObject where value > %u", search_value );
  queryObjects( engine, sql );

  // Objects with value greater or equal than search_value
  sprintf( sql, "select * from AnObject where value >= %u", search_value );
  queryObjects( engine, sql );

  // Objects with value equal to search_value
  sprintf( sql, "select * from AnObject where value = %u", search_value );
  queryObjects( engine, sql );

  // Objects with value less or equal than search_value
  sprintf( sql, "select * from AnObject where value <= %u", search_value );
  queryObjects( engine, sql );

  // Objects with value less than search_value
  sprintf( sql, "select * from AnObject where value < %u", search_value );
  queryObjects( engine, sql );

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

int addObject( McoSqlEngine& engine, _Object* o )
{
  int ret = engine.executeStatement("insert into AnObject %R", o);
  if ( 0 == ret )
  {
    printf("\t\tError inserting AnObject %d\n", o->value );
  }
  return ret;
}

int queryObjects( McoSqlEngine& engine, char * sql )
{
  printf( "\n\t%s\n", sql);
  QueryResult result( engine.executeQuery( sql ) );
  int count = 0;
  Cursor* cursor = result->records();
  while ( cursor->hasNext() )
  {
    Record* rec = cursor->next();
    _Object o;
    result->extract( rec, &o, sizeof(o) );
    printObject( o );
    count++;
  }
  return count;
}

void printObject( _Object o ) {
  printf("\t\tObject: id=%u, value=%u\n", o.id, o.value );
}
