/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "cursordb.h"

char sample_descr[] = 
"Sample 'cursor' demonstrates eXtremeSQL cursor operations with a btree index.\n";

const char * db_name = "cursordb";

using namespace McoSql;

#define MAX_SQL_LENGTH  128

const int nRecords = 20;

// Define the structure corresponding to database table Object
struct _Object
{
  uint4  id;
  char * name;
};

uint4 search_value = 3;

// Forward declarations
int addObject( McoSqlEngine& engine, _Object* o );
int queryObjects( McoSqlEngine& engine, char * sql );
void printObject( _Object o );

int main( int argc, char ** argv )
{
  McoSqlEngine engine;

  char name[ 64 ];

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, cursordb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
  
  // Fill database
  for ( int i=0; i < nRecords; i++ ) {
    _Object o;
    o.id = i;
    sprintf( name, "Object%d", i );
    o.name = name;
    addObject( engine, &o );
  }

  // Show all objects
  printf( "\n\tAll Objects ordered by Id:" );
  queryObjects( engine, "select * from AnObject order by id" );
 
  printf( "\n\tAll Objects in ascending order by Name:" );
  queryObjects( engine, "select * from AnObject order by name" );

  printf( "\n\tAll Objects in descending order by Name:" );
  queryObjects( engine, "select * from AnObject order by name desc" );

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
    printf("\t\tError inserting AnObject %s\n", o->name );
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
  printf("\t\tObject: id=%u, name=%s\n", o.id, o.name );
}
