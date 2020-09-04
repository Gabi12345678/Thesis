/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "patriciadb.h"

char sample_descr[] = 
"Sample 'patricia' demonstrates eXtremeSQL cursor operations with a patricia index.\n";

const char * db_name = "patriciadb";

using namespace McoSql;

#define MAX_SQL_LENGTH  128
#undef MEMORY_PAGE_SIZE
#define MEMORY_PAGE_SIZE 256

const int nRecords = 20;

// Define the structure correponding to database table AreaCode and array codes
typedef struct _Object
{
  char * areaCode;
  char * strAreaCode;
} _AreaCode_info;
_AreaCode_info codes[] = {
  { "01", "ATT" },
  { "020", "BCC" },
  { "025", "TNT" },
  { "03", "ANC" },
  { "0355", "NCC" },
  { "0355", "UDC" },
  { "045", "WTC" },
  { "0454", "FCC" },
  { "05", "ABB" },
  { "051", "DDA" },
  { "051", "FIY" },
  { "0513", "ASD" },
  { "0515", "ASH" },
  { "05183", "ASW" },
  { "0525", "ASP" },
  { "06", "YUI" },
  { "064", "KEW" },
  { "066", "WER" },
  { "0668", "VBR" },
  { "0668", "MPP" },
  { "0668", "FCA" },
  { "06689", "FCU" },
  { "0699", "FCH" },
  { "06683", "FCL" },
  { "06685", "FCD" },
  { "06687", "FCA" },
  { "07", "MNM" },
  { "07", "MGM" },
  { "079", "MSM" },
  { "0795", "EAE" },
  { "081", "YUI" },
  { "084", "WWW" },
  { "0849", "OLA" },
  { "09", "GEA" },
  { "099", "YTR" },
  { "099", "IUI" },
  { "0988", "JHJ" },
  { "0988", "DFD" }
};
uint4 search_value = 3;

// Forward declarations
int addObject( McoSqlEngine& engine, _Object* o );
int queryObjects( McoSqlEngine& engine, char * sql );
void printObject( _Object o );

int main( int argc, char ** argv )
{
  McoSqlEngine engine;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, patriciadb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
  
  // Fill database
  for ( int i=0; i < (int)(sizeof(codes) / sizeof(codes[0])); i++ ) {
    _Object o;
    o.areaCode = codes[i].areaCode;
    o.strAreaCode = codes[i].strAreaCode;
    addObject( engine, &o );
  }

  // Show all objects
  printf( "\n\tAll AreaCodes ordered by areaCode:" );
  queryObjects( engine, "select * from AreaCode order by areaCode" );
 
  printf( "\n\tAll AreaCodes in ascending order by strAreaCode:" );
  queryObjects( engine, "select * from AreaCode order by strAreaCode" );

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

int addObject( McoSqlEngine& engine, _Object* o )
{
  int ret = engine.executeStatement("insert into AreaCode %r", o);
  if ( 0 == ret )
  {
    printf("\t\tError inserting Object %s\n", o->areaCode );
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
  printf("\t\tAreaCode: %s, %s\n", o.areaCode, o.strAreaCode );
}

