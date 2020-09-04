/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <common.h>
#include "groupbydb.h"

char sample_descr[] = 
"Sample 'group_by' demonstrates use of the eXtremeSQL 'group by' clause.\n";
const char * db_name = "groupbydb";

using namespace McoSql;

#define MAX_SQL_LENGTH  128
const int nRecords = 10;
char sql[ MAX_SQL_LENGTH + 1 ];

// Define the structure corresponding to database table Object
struct _Contribution
{
  char const * name;
  uint4        date_val;
  uint4        amount;
};

_Contribution contributions[] = {
  { "Michael", 91224, 250 },
  { "Sharon", 91224, 50 },
  { "George", 100101, 100 },
  { "Sharon", 100101, 150 },
  { "Michael", 100321, 300 },
  { "Liz", 100321, 20 },
  { "Bob", 100322, 25 }
};

// Forward declarations
int addContribution( McoSqlEngine& engine, _Contribution* c );
int queryContributions( McoSqlEngine& engine, char * sql );
void printContribution( int i, _Contribution c );
void showGroup1( McoSqlEngine& engine );
void showGroup2( McoSqlEngine& engine );

// Declare and instantiate the user-defined DateFormat function
static McoSql::Value* dateformat( Runtime* runtime, Vector<McoSql::Value>* params )
{
  Allocator *allocator = runtime->engine->getAllocator();
  char str[64];
  McoSql::Value* date = params->at(0);
  int date_val = (int)date->intValue();
  int yy = date_val / 10000;
  int mm = date_val / 100 % 100;
  int dd = date_val % 100;
  // Format dd.mm.yyyy assuming all dates are after 2000
  sprintf( str, "%d.%d.20%02d", dd, mm, yy );
  return String::create( allocator, str );
}
static SqlFunctionDeclaration udf(
  tpString,          // tpString is the return value of the UDF
  "dateformat",      // the name of the function as we’ll use it in a query
  dateformat,        // the function pointer
  1                  // the number of arguments to the UDF
);

int main( int argc, char ** argv )
{
  McoSqlEngine engine;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, groupbydb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
  
  // Fill database
  for ( int i=0; i < (int)(sizeof( contributions ) / sizeof( contributions[0] )); i++ ) {
    addContribution( engine, &contributions[i] );
  }

  // Show all objects
  queryContributions( engine, "Select * from Contributions" );

  // Show sub-totals by contributor
  showGroup1( engine );

  // Show sub-totals by date using user-defined DateFormat
  showGroup2( engine );

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

int addContribution( McoSqlEngine& engine, _Contribution* c )
{
  int ret = engine.executeStatement("insert into Contributions %r", c);
  if ( 0 == ret )
  {
    printf("\t\tError inserting Contributions %s\n", c->name );
  }
  return ret;
}

int queryContributions( McoSqlEngine& engine, char * sql )
{
  printf( "\n%s\n", sql);
  QueryResult result( engine.executeQuery( sql ) );
  int count = 0;
  Cursor* cursor = result->records();
  while ( cursor->hasNext() )
  {
    Record* rec = cursor->next();
    _Contribution c;
    result->extract( rec, &c, sizeof(c) );
    printContribution( count+1, c );
    count++;
  }
  return count;
}

void printContribution( int i, _Contribution c ) {
  printf("\t%d: name=%s, date=%d, amount=%d\n", i, c.name, c.date_val, c.amount );
}

void showGroup1( McoSqlEngine& engine )
{
  sprintf( sql, "Select name, sum(amount) from Contributions group by name" );
  printf( "\n%s\n", sql );
  QueryResult result( engine.executeQuery( sql ) );
  Cursor* cursor = result->records();
  for (int i = 1; cursor->hasNext(); i++)
  {
    Record* rec = cursor->next();
    ValueRef ref(rec->get(0));
    printf( "\n\t%d)\t%s\t%f", i, ref.as<String>()->cstr(), rec->get(1)->realValue() );
  }
}

void showGroup2( McoSqlEngine& engine )
{
  sprintf( sql, "Select dateformat(date_val), sum(amount) from Contributions group by date_val" );
  printf( "\n\n%s\n", sql );
  QueryResult result( engine.executeQuery( sql ) );
  Cursor* cursor = result->records();
  for (int i = 1; cursor->hasNext(); i++)
  {
    Record* rec = cursor->next();
    ValueRef ref(rec->get(0));
    printf( "\n\t%d)\t%s\t%f", i, ref.as<String>()->cstr(), rec->get(1)->realValue() );
  }
}
