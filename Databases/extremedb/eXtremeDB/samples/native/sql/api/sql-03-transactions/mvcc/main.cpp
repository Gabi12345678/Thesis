/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include "mcosql.h"
#include "mcosmp.h"
#include <commonSQL.h>

using namespace McoSql;

// Forward declaration of function describing database schema
GET_DICTIONARY(transdb);

char sample_descr[] = "Sample 'transactions_mvcc' demonstrates insert\n" 
"operations using C++ with the SQL engine and MVCC Transaction Mgr.\n";
const char * db_name = "transdb";
char explanation[] =  "\n\tFor each of the three isolation levels with MVCC, two\n"
                      "\tthreads are started to run in parallel.  The first thread\n"
                      "\twrites to table A and the second to table B.  As they write\n"
                      "\tthe database records output is written also to a memory buffer\n"
                      "\twhich is unspooled afterwards to show the actual order of\n"
                      "\toperations.\n"
                      "\tThe thread interaction can be effected by three command line arguments:\n"
                      "\t   n_iterations - number of records written to output (default=10) \n"
                      "\t   nap_duration1 - sleep time for task 1 (default=10 msec)\n"
                      "\t   nap_duration2 - sleep time for task 2 (default=5 msec)\n\n";


#define DB_SIZE      (1024 * 1024)
#define DB_PAGE_SIZE    256
#define MAX_TAG_LENGTH  64
#define MAX_BUF_LENGTH  4096

#define N_THREADS 2 
#define N_ITERATIONS 10 
#define NAP_DURATION_1 10 
#define NAP_DURATION_2 5 

/* thread parameters */
static sample_task_t tasks[N_THREADS];
int n_iterations = N_ITERATIONS;
int nap_duration1 = NAP_DURATION_1;
int nap_duration2 = NAP_DURATION_2;
char buf[MAX_BUF_LENGTH + N_ITERATIONS];

// Define global McoSQL engine and Isolation level
using namespace McoSql;
McoMultithreadedSqlEngine engine;
McoSql::Transaction::IsolationLevel isolation_level;
const char* isolation_level_name[] = {
  "",
  "READ COMMITTED",
  "REPEATABLE READ",
  "",
  "SERIALIZABLE"
};

/* Structures covering database classes */
struct StructA
{
  unsigned int ui4;
  char* tag;
};

struct StructB
{
  unsigned int ui4;
  char* tag;
};

mco_mutex_t *buf_mtx;

/* write tag to memory buffer */
void mem_write( const char * tag) {
    buf_mtx->lock(buf_mtx);
    strcat(buf, tag);
    buf_mtx->unlock(buf_mtx);
}

void task1( sample_task_t * descriptor ) 
{
  int rc;
  uint4 i;
  McoSqlSession session( &engine ); 
  Transaction* trans = session.beginTransaction(Transaction::ReadWrite, 0, isolation_level);

#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
  try {
#endif
      for (i = 0; i < (uint4)n_iterations; i++) 
      { 
          StructB a;
          char tag[MAX_TAG_LENGTH+1];
          a.tag = tag;
          a.ui4 = i;
          sprintf(a.tag, "\t\tStream 1, rec %u\n", i);
          mem_write(a.tag);
          //rc = pEngine->executeStatement("insert into A %r", a);
          rc = session.executeStatement(trans, "insert into A %r", &a);
          sample_sleep(nap_duration1);
      }
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
  } catch (McoSqlException &err) {
      mem_write("ERROR: ");
      mem_write(err.getMessage()->cstr());
      trans->rollback();
      trans->release();
      return;
  }
#endif
  trans->commit();
  trans->release();
}

void task2( sample_task_t * descriptor ) 
{
  int rc;
  uint4 i;
  McoSqlSession session( &engine ); 
  Transaction* trans = session.beginTransaction(Transaction::ReadWrite, 0, isolation_level);

#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
  try {
#endif
      for (i = 0; i < (uint4)n_iterations; i++) 
      { 
          StructB b;
          char tag[MAX_TAG_LENGTH+1];
          b.tag = tag;
          b.ui4 = i;
          sprintf(b.tag, "\t\tStream 2, rec %u\n", i);
          mem_write(b.tag);
          //rc = pEngine->executeStatement("insert into B %r", b);
          rc = session.executeStatement(trans, "insert into B %r", &b);
          sample_sleep(nap_duration2);
      }
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
  }catch (McoSqlException &err) {
      mem_write("ERROR: ");
      mem_write(err.getMessage()->cstr());
      trans->rollback();
      trans->release();
      return;
  }
#endif
  trans->commit();
  trans->release();
}    

void do_test(MCO_TRANS_ISOLATION_LEVEL test_isolation_level) 
{ 
  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open(db_name,  // database name
    transdb_get_dictionary(),  // database dictionary
    DATABASE_SIZE,  // database size
    DB_PAGE_SIZE,  // page size
    MAP_ADDRESS); // mapping address for shared memory mode

  isolation_level = (McoSql::Transaction::IsolationLevel)test_isolation_level;
  printf("\tIsolation level %s\n\n", isolation_level_name[isolation_level]); 

  sample_start_connected_task( &tasks[0], task1, db_name, (void*)&engine );
  sample_start_connected_task( &tasks[1], task2, db_name, (void*)&engine );
  sample_join_task(&tasks[0]);
  sample_join_task(&tasks[1]);

  engine.close();
}

int main(int argc, char* argv[]) 
{
  mco_runtime_info_t rt_info;

  mco_get_runtime_info(&rt_info);
  if (! rt_info.concurrent_write_transactions) {
    printf("This sample requires MVCC transaction manager\n");
    dbg_exit(0);
  }

  sample_os_initialize(DEFAULT);

  buf_mtx = mco_create_mutex();

  sample_header(sample_descr);
  printf(explanation);

  /* get command line args if any: arg1=n_iterations, arg2=nap_duration1, arg3=nap_duration2 */
  if ( argc > 1 ) {
    n_iterations = atoi(argv[1]);
    if ( MAX_BUF_LENGTH < n_iterations * MAX_TAG_LENGTH )
      n_iterations = MAX_BUF_LENGTH / ( MAX_TAG_LENGTH + 1 );
  }
  if ( argc > 2 ) {
    nap_duration1 = atoi(argv[2]);
  }
  if ( argc > 3 ) {
    nap_duration2 = atoi(argv[3]);
  }

  printf( "\n\tRunning with n_iterations=%d, nap_duration1=%d, nap_duration2=%d\n", n_iterations, nap_duration1, nap_duration2 );
  sample_pause("\nPress any key to continue . . .\n");
  do_test(MCO_READ_COMMITTED);
  printf(buf);

  buf[0] = 0;   /* rewind buffer */
  sample_pause("\n\nPress any key to continue . . .\n");
  do_test(MCO_REPEATABLE_READ);
  printf(buf);

  buf[0] = 0;   /* rewind buffer */
  sample_pause("\n\nPress any key to continue . . .\n");
  do_test(MCO_SERIALIZABLE);
  printf(buf);

  sample_pause_end("\n\nPress any key to continue . . . ");

  buf_mtx->destroy(buf_mtx);
  sample_os_shutdown();
  return 0;
}
