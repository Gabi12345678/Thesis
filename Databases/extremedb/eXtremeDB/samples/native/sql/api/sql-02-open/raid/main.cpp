/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "persondb.h"

char sample_descr[] = 
"Sample 'raid' opens and initializes a RAID database, then displays\n"
"the results.\n";

const char * db_name = "persondb";
const int n_devices = 5;
const int n_persons = 10;
const size_t MAX_LINE_LENGTH = 2048;

using namespace McoSql;

// Define the structure correponding to database table Person
struct _Person
{
  char const* name;
  uint4       ordinal;
};

/* forward declarations */
void generate_persons_records( McoSqlEngine& engine );
void show_results( McoSqlEngine& engine );
int showPersons( QueryResult * result );

int main( int argc, char ** argv )
{
  MCO_RET rc;
  mco_device_t dev[n_devices];

  McoSqlEngine engine; 
  McoSqlOpenParameters params;

  sample_os_initialize(DEFAULT);
  sample_header(sample_descr);

  dev[0].type       = MCO_MEMORY_CONV;
  dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;
  dev[0].size       = DATABASE_SIZE;
  dev[0].dev.conv.ptr = (void*)malloc( DATABASE_SIZE );

  dev[1].type       = MCO_MEMORY_CONV;
  dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;
  dev[1].size       = CACHE_SIZE;
  dev[1].dev.conv.ptr = (void*)malloc( CACHE_SIZE );
  
  dev[2].type       = MCO_MEMORY_RAID;
  dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
  strcpy(dev[2].dev.raid.name, "raidpart.1");
  dev[2].dev.raid.flags = MCO_FILE_OPEN_DEFAULT;
  dev[2].dev.raid.level = 1;

  dev[3].type       = MCO_MEMORY_RAID;
  dev[3].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
  strcpy(dev[3].dev.raid.name, "raidpart.2");
  dev[3].dev.raid.flags = MCO_FILE_OPEN_DEFAULT;
  dev[3].dev.raid.level = 1;

  dev[4].type       = MCO_MEMORY_FILE;
  dev[4].assignment = MCO_MEMORY_ASSIGN_LOG;
  strcpy(dev[4].dev.file.name, "persondb.log");
  dev[4].dev.file.flags = MCO_FILE_OPEN_DEFAULT;

  params.databaseName = (char *)db_name;
  params.dictionary = persondb_get_dictionary();
  params.mainMemoryDatabaseSize = DATABASE_SIZE;
  params.mainMemoryPageSize = MEMORY_PAGE_SIZE;
  params.n_devices = n_devices;
  params.devices = dev;

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open(params);

  generate_persons_records(engine);

  show_results(engine);

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

void generate_persons_records(McoSqlEngine& engine)
{
  int   ord;
  char  name[16];
  char  sql[MAX_LINE_LENGTH * 2];
  
  for( ord=1; ord <= n_persons; ord++ ) {
    sprintf( name, "Person%d", ord );
    sprintf(sql, "insert into Person (name,ordinal) values ('%s','%d')", name, ord);
    engine.executeStatement(sql);
  }
}

void show_results(McoSqlEngine& engine)
{
  QueryResult result(engine.executeQuery("select * from Person"));
  
  showPersons( &result );
}

int showPersons(QueryResult * result)
{
  int count = 1;
  Cursor* cursor = (*result)->records();

  printf("\n\tContents of table Person:\n");
  while (cursor->hasNext())
  {
    Record* rec = cursor->next();
    _Person p;

    // Extract the Person record to the corresponding struct
    (*result)->extract(rec, &p, sizeof(p));
    printf("\t\t%d) Name=%s, Ordinal=%u\n", count, p.name, p.ordinal );
    count++;
  }
  return count;
}

