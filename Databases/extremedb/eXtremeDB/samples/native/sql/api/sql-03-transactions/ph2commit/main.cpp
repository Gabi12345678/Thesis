/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include "mcosql.h"
#include <commonSQL.h>

char sample_descr[] = "Sample 'ph2commit' demonstrates a two-phase\n"
                      "transaction using C++ and the SQL engine.\n";
char * dbName1 = "ph2_1";
char * dbName2 = "ph2_2";
char explanation[] =  
"\nThis sample opens and populates two in-memory databases\n"
"\then performs 2-phase commits and displays the contents.\n"
"Note that some attempts are made to insert duplicate key\n"
"values.  These fail, demonstrating how to manage errors\n"
"during the 2-phase commit.\n";

const unsigned int nRecords = 3;
using namespace McoSql;

// Forward declaration of function describing database schema
GET_DICTIONARY(ph2commitdb);

// Define the structure correponding to database table T
struct _T
{
  uint4       IntKey;
};

// Forward declarations
int showRecords( McoSqlEngine* pEngine, int instance );

int main( int argc, char ** argv )
{
  McoSqlEngine engine1, engine2;
  McoSqlOpenParameters params;

  uint4 i, key;
  int nCommitsAttempted = 0;
  int nCommitsSucceded = 0;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);
  printf(explanation);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine1.license(EXTREMEDB_LICENSE_KEY);
  engine2.license(EXTREMEDB_LICENSE_KEY);
  #endif

  params.databaseName            = dbName1;
  params.dictionary              = ph2commitdb_get_dictionary();
  params.mainMemoryDatabaseSize  = DATABASE_SIZE;
  params.mainMemoryPageSize      = MEMORY_PAGE_SIZE;
  engine1.open(params);
        
  params.databaseName = dbName2;
  params.flags &= ~(McoSqlOpenParameters::START_MCO_RUNTIME); // don't start runtime again
  engine2.open(params);

  /* fill database 1 */
  printf("\n\tFill database ph2_%d ...\n", 1 );
  for (i = 0; i < nRecords; i++)
  {
    key = i * 3 + 1;
    engine1.executeStatement("insert into T (intKey) values (%u)", key);
    printf("\t\tInsert into T key = %u\n", key );
  }

  /* fill database 2 */
  printf("\n\tFill database ph2_%d ...\n", 2 );
  for (i = 0; i < nRecords; i++)
  {
    key = i * 3 + 2;
    engine2.executeStatement("insert into T (intKey) values (%u)", key);
    printf("\t\tInsert into T key = %u\n", key );
  }

  printf("\n\n\tBegin 2-phase commits for key values %d to %d ...\n\n", 0, nRecords*3-1 );
  for (i = 0; i < nRecords * 3; ++i)
  {
    Transaction* trans1 = engine1.beginTransaction(Transaction::ReadWrite);
    Transaction* trans2 = engine2.beginTransaction(Transaction::ReadWrite);

    key = i;
    engine1.executeStatement(trans1, "INSERT INTO T (intKey) VALUES(%u)", key);
    engine2.executeStatement(trans2, "INSERT INTO T (intKey) VALUES(%u)", key);
    printf("\t  key = %u\n", key );

#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    try {
#endif
      nCommitsAttempted++;
      printf("\t\tCommit phase 1 to ph2_1 for key = %u\n", key );
      trans1->commit(1);
      nCommitsSucceded++;
      nCommitsAttempted++;
      printf("\t\tCommit phase 1 to ph2_2 for key = %u\n", key );
      trans2->commit(1);
      nCommitsSucceded++;
      // Both phase-1 commits succeded, Commit phase 2 of both transactions
      nCommitsAttempted++;
      printf("\t\tCommit phase 2 to ph2_1 for key = %u\n", key );
      trans1->commit(2);
      nCommitsSucceded++;
      nCommitsAttempted++;
      printf("\t\tCommit phase 2 to ph2_2 for key = %u\n", key );
      trans2->commit(2);
      nCommitsSucceded++;
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    } catch (NotUnique &) {
      printf("\tError committing transactions for key = %u ...\n", key );
      trans1->rollback();
      trans2->rollback();
    }
#endif
    // Release the transactions
    trans1->release();
    trans2->release();
  }
  printf("\t%d 2-phase commits attempted, %d succeded\n\n\tFinal results:\n", nCommitsAttempted, nCommitsSucceded );
  showRecords( &engine1, 1 );
  showRecords( &engine2, 2 );

  engine2.close();
  engine1.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

int showRecords( McoSqlEngine* pEngine, int instance )
{
  DataSource* source = pEngine->executeQuery("select * from T order by intKey");
  Cursor* cursor = source->records();
  int count = 1;
  
  printf("\n\tContents of table T in database ph2_%d:\n", instance );
  while (cursor->hasNext())
  {
    Record* rec = cursor->next();
    _T t;

    // Extract the record to the corresponding struct
    source->extract(rec, &t, sizeof(t));
    printf( "\t\t%d) intKey=%u\n", count, t.IntKey );
    count++;
  }
  return count;
}

