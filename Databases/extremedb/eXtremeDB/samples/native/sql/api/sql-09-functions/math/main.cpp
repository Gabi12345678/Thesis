/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "mathdb.h"

char sample_descr[] = 
"Sample 'math' demonstrates use of the eXtremeSQL math functions.\n";
const char * db_name = "mathdb";

using namespace McoSql;

#define MAX_SQL_LENGTH  128
const int nRecords = 10;

// Define the structure correponding to database table Object
struct _Event
{
  uint4        id;
  char const * event_name;
};

// Forward declarations
int addEvent( McoSqlEngine& engine, _Event* e );
int queryEvents( McoSqlEngine& engine, char * sql );
void printEvent( _Event e );

// Delcare and instantiate the user-defined Mod function
static McoSql::Value* mod( Runtime* runtime, Vector<McoSql::Value>* params ) 
{
  Allocator *allocator = runtime->engine->getAllocator();
  McoSql::Value* a = params->at(0);
  McoSql::Value* b = params->at(1);
  if ( a->isNull() || b->isNull() ) { 
    return NULL;
  }
  return IntValue::create( allocator, a->intValue() % b->intValue() );
}
static SqlFunctionDeclaration udf(
  tpInt,      // tpInt is the return value of the UDF
  "mod",      // the name of the function as we’ll use it in a query
  mod,        // the function pointer
  2           // the number of arguments to the UDF
);

int main( int argc, char ** argv )
{
  McoSqlEngine engine;

  char name[ 64 ];

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif

  engine.open( db_name, mathdb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
  
  // Fill database
  for ( int i=0; i < nRecords; i++ ) {
    _Event e;
    e.id = i;
    sprintf( name, "Event %d", i+1 );
    e.event_name = name;
    addEvent( engine, &e );
  }

  // Show all objects
  queryEvents( engine, "Select * from Events" );

  // Show objects with id > real value 3.14159265
  queryEvents( engine, "select * from Events where real( id ) > 3.14159265" );

  // Show objects with id divisible by 3 using UDF 'mod'
  queryEvents( engine, "select * from Events where mod( id, 3 ) = 0" );

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

int addEvent( McoSqlEngine& engine, _Event* e )
{
  int ret = engine.executeStatement("insert into Events %r", e);
  if ( 0 == ret )
  {
    printf("\t\tError inserting Event %s\n", e->event_name );
  }
  return ret;
}

void printEvent( _Event e ) {
  printf("\t\tObject: id=%d, event_name=%s\n", e.id, e.event_name );
}

int queryEvents( McoSqlEngine& engine, char * sql )
{
  printf( "\n\t%s\n", sql);
  QueryResult result( engine.executeQuery( sql ) );
  int count = 0;
  Cursor* cursor = result->records();
  while ( cursor->hasNext() )
  {
    Record* rec = cursor->next();
    _Event e;
    result->extract( rec, &e, sizeof(e) );
    printEvent( e );
    count++;
  }
  return count;
}

