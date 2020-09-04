/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "referencesdb.h"

char sample_descr[] = 
"Sample 'references' opens and initializes a database with three tables\n"
"and performs joins through varied select statements.  The joins are\n"
"implemented through references to auto-id fields in the parent records.\n";

const char * db_name = "joindb";
const int nRecords = 10;
const size_t MAX_LINE_LENGTH = 2048;

using namespace McoSql;

// Define the structure corresponding to database tables
struct _Address
{
  int4        zipcode;
  char const* city;
  char const* street;
};

// timer variables
time_t start_time;
time_t delta;

/* forward declarations */
void insertRecords( McoSqlEngine& engine );
void searchAddresses( McoSqlEngine& engine );
void showJoin1( McoSqlEngine& engine );
void showJoin2( McoSqlEngine& engine );
void showJoin3( McoSqlEngine& engine );
void deleteRecords( McoSqlEngine& engine );

int main( int argc, char ** argv )
{
  McoSqlEngine engine;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, referencesdb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);

  // Insert records
  insertRecords( engine );

  // Search Addresses by zip
  searchAddresses( engine );

  // Show Company-Addresses
  showJoin1( engine );

  // Show Company-Orders
  showJoin2( engine );

  // Show Company-Orders where desc like '%1%'
  showJoin3( engine );

  // Clean up
  deleteRecords( engine );

  engine.close();
  
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

void insertRecords( McoSqlEngine& engine )
{
  int i;
  char city[32];
  char street[256];
  char company[64];
  char order[64];
  char statement[ MAX_LINE_LENGTH + 1 ];

  printf("\n\tInsert %d records ...\n\t", nRecords );
  start_time = time(NULL);
  for (i = 1; i <= nRecords; i++)
  {
    sprintf( city, "Arzamas");
    sprintf( street, "%dth Aveny, %d", i + 3, i);
    sprintf( statement,  "insert into PostalAddress (zipcode, city, street) values ('%d','%s','%s')", i, city, street );
    engine.executeStatement( statement );

    sprintf( company, "Factory %d", 100 + i );
    sprintf( statement, "insert into Company (location, name) "
                        "values ((select autoid from PostalAddress where zipcode='%d'),'%s')", 
             i, company );
    engine.executeStatement( statement );

    sprintf( order, "Microcontroller XYZ-%d", i);
    sprintf( statement, "insert into Orders (company,shipment,amount,description) "
                        "values ((select autoid from Company where name='%s'),now,'%d','%s')", 
             company, i % 100, order );
    engine.executeStatement( statement );

    if (i % (nRecords / 10) == 0)
    {
      printf(".");
    }
  }

  delta = time(NULL) - start_time;
  printf("\n\t%d objects: %d seconds (%d object/second)\n", nRecords, (int)delta,
                                      (int)delta != 0 ? nRecords / (int)delta: nRecords);
}

void searchAddresses( McoSqlEngine& engine )
{
  printf("\n\tDirect search PostalAddress by zip...\n");
  start_time = time(NULL);
  for ( int i = 1; i <= nRecords; i++ )
  {
    QueryResult result( engine.executeQuery( "select * from PostalAddress where zipcode=%i", i ) );
    Cursor* iterator = result->records();
    assert(iterator->hasNext());
    Record* rec = iterator->next();
    _Address a;
    result->extract(rec, &a, sizeof(a));
    printf("\t\tZipcode = %d) %s, %s\n", i, a.street, a.city );
  }
  delta = time(NULL) - start_time;
  printf("\n\t%d objects: %d seconds (%d object/second)\n", nRecords, (int)delta,
                                      (int)delta != 0 ? nRecords / (int)delta: nRecords);
}

void showJoin1( McoSqlEngine& engine )
{
  printf("\n\n\tSELECT C.name FROM PostalAddress A,Company C ...");
  for (int i = 1; i <= nRecords; i++)
  {
    QueryResult result( engine.executeQuery( "SELECT C.name FROM PostalAddress A,Company C "
                                              "WHERE A.zipcode=%i AND A.autoid=C.location", i ) );
    Cursor* iterator = result->records();
    assert(iterator->hasNext());
    Record* rec = iterator->next();
    ValueRef nameRef(rec->get(0));
    String * pName = nameRef.as<String>();
    printf( "\n\t\t%d) %s", i, pName->cstr() );
  }
}

void showJoin2( McoSqlEngine& engine )
{
  printf("\n\n\tSELECT C.name, O.description FROM PostalAddress A,Company C,Orders O ...");
  for (int i = 1; i <= nRecords; i++)
  {
    QueryResult result( engine.executeQuery( "SELECT C.name, O.description FROM PostalAddress A,Company C,Orders O "
                                              "WHERE A.zipcode=%i AND A.autoid=C.location AND C.autoid=O.company", i ) );
    Cursor* iterator = result->records();
    assert(iterator->hasNext());
    Record* rec = iterator->next();
    ValueRef nameRef(rec->get(0));
    ValueRef descRef(rec->get(1));
    String * pName = nameRef.as<String>();
    String * pDesc = descRef.as<String>();
    printf( "\n\t\t%d) %s: %s", i, pName->cstr(), pDesc->cstr() );
  }
}

void showJoin3( McoSqlEngine& engine )
{
  printf("\n\n\tSELECT C.name, O.description ... WHERE O.description like '%%1%%' ...");
  for (int i = 1; i <= nRecords; i++)
  {
    QueryResult result( engine.executeQuery( "SELECT C.name, O.description FROM PostalAddress A,Company C,Orders O "
                                              "WHERE O.description like '%1%' AND "
                                              "A.zipcode=%i AND A.autoid=C.location AND C.autoid=O.company", i ) );
    Cursor* iterator = result->records();
    while ( iterator->hasNext() ) {
      Record* rec = iterator->next();
      ValueRef nameRef(rec->get(0));
      ValueRef descRef(rec->get(1));
      String * pName = nameRef.as<String>();
      String * pDesc = descRef.as<String>();
      printf( "\n\t\t%d) %s: %s", i, pName->cstr(), pDesc->cstr() );
    }
  }
}

void deleteRecords( McoSqlEngine& engine )
{
  printf("\n\n\tDelete %d records ...\n\t", nRecords );
  start_time = time(NULL);
  engine.executeStatement("delete from Orders");
  engine.executeStatement("delete from Company");
  engine.executeStatement("delete from PostalAddress");

  delta = time(NULL) - start_time;
  printf("\n\t%d objects: %d seconds (%d object/second)\n", nRecords, (int)delta,
                                    (int)delta != 0 ? nRecords / (int)delta: nRecords);
}
