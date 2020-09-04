/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "joindb.h"

char sample_descr[] = 
"Sample 'join' opens and initializes a database with three tables\n"
"and performs joins through varied select statements.\n";

const char * db_name = "joindb";
const size_t MAX_LINE_LENGTH = 2048;
int nRecords = 500;

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
void showJoin0( McoSqlEngine& engine );
void showJoin1( McoSqlEngine& engine );
void showJoin2( McoSqlEngine& engine );
void deleteRecords( McoSqlEngine& engine );

int main( int argc, char ** argv )
{
  McoSqlEngine engine;

  if ( argc > 1 ) {
    nRecords = atoi(argv[1]);
  }

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, joindb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);

  // Insert records
  insertRecords(engine);

  // Show Company-Orders where desc like '%1%' using references
  printf("\n\n\tSELECT C.name, O.description ... WHERE O.description like '%%1%%'\n");
  showJoin0(engine);

  // Show Company-Orders where desc like '%1%' using indexes but no references
  showJoin1(engine);

  // Show Company-Orders where desc like '%1%' using no indexes or references
  showJoin2(engine);

  // Clean up
  deleteRecords(engine);

  engine.close();
  
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

void insertRecords( McoSqlEngine& engine )
{
  int i;
  int base_id;
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
    sprintf( statement,  "insert into PostalAddress1 (zipcode, city, street) values ('%d','%s','%s')", i, city, street );
    engine.executeStatement( statement );
    sprintf( statement,  "insert into PostalAddress2 (zipcode, city, street) values ('%d','%s','%s')", i, city, street );
    engine.executeStatement( statement );

    base_id = i / 10 + 100;
    sprintf( company, "Factory %d", base_id + i );
    sprintf( statement, "insert into Company (location, name) "
                        "values ((select autoid from PostalAddress where zipcode='%d'),'%s')", 
             i, company );
    engine.executeStatement( statement );
    sprintf( statement, "insert into Company1 (id, name, zipcode) " 
                        "values ('%d','%s','%d')", base_id + i, company, i );
    engine.executeStatement( statement );
    sprintf( statement, "insert into Company2 (id, name, zipcode) "
                        "values ('%d','%s','%d')", base_id + i, company, i );
    engine.executeStatement( statement );

    sprintf( order, "Microcontroller XYZ-%d", i);
    sprintf( statement, "insert into Orders (company,shipment,amount,description) "
                        "values ((select autoid from Company where name='%s'),now,'%d','%s')", 
             company, i % 100, order );
    engine.executeStatement( statement );
    sprintf( statement, "insert into Orders1 (company_id,shipment,amount,description) "
                        "values ('%d',now,'%d','%s')", 
             base_id + 1, i % 100, order );
    engine.executeStatement( statement );
    sprintf( statement, "insert into Orders2 (company_id,shipment,amount,description) "
                        "values ('%d',now,'%d','%s')", 
             base_id + 1, i % 100, order );
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

void showJoin0( McoSqlEngine& engine )
{
  printf("\n\n\t0) Using references...\n");
  start_time = time(NULL);
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
  delta = time(NULL) - start_time;
  printf("\t\t%d objects: %d seconds (%d object/second)\n", nRecords, (int)delta,
                                (int)delta != 0 ? nRecords / (int)delta: nRecords);
}

void showJoin1( McoSqlEngine& engine )
{
  printf("\n\n\t1) No references, no indexes...\n");
  start_time = time(NULL);
  for (int i = 1; i <= nRecords; i++)
  {
    QueryResult result( engine.executeQuery( "SELECT C.name, O.description FROM PostalAddress1 A,Company1 C,Orders1 O "
                                              "WHERE O.description like '%1%' AND "
                                              "A.zipcode=%i AND C.zipcode=A.zipcode AND O.company_id=C.id", i ) );
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
  delta = time(NULL) - start_time;
  printf("\t\t%d objects: %d seconds (%d object/second)\n", nRecords, (int)delta,
                                (int)delta != 0 ? nRecords / (int)delta: nRecords);
}

void showJoin2( McoSqlEngine& engine )
{
  printf("\n\n\t3) With indexes with no references...\n");
  start_time = time(NULL);
  for (int i = 1; i <= nRecords; i++)
  {
    QueryResult result( engine.executeQuery( "SELECT C.name, O.description FROM PostalAddress2 A,Company2 C,Orders2 O "
                                              "WHERE O.description like '%1%' AND "
                                              "A.zipcode=%i AND C.zipcode=A.zipcode AND O.company_id=C.id", i ) );
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
  delta = time(NULL) - start_time;
  printf("\t\t%d objects: %d seconds (%d object/second)\n", nRecords, (int)delta,
                                (int)delta != 0 ? nRecords / (int)delta: nRecords);
}

void deleteRecords( McoSqlEngine& engine )
{
  engine.executeStatement("delete from Orders");
  engine.executeStatement("delete from Orders1");
  engine.executeStatement("delete from Orders2");
  engine.executeStatement("delete from Company");
  engine.executeStatement("delete from Company1");
  engine.executeStatement("delete from Company2");
  engine.executeStatement("delete from PostalAddress");
  engine.executeStatement("delete from PostalAddress1");
  engine.executeStatement("delete from PostalAddress2");
}
