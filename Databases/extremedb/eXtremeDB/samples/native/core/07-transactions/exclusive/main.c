/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <common.h>
#include <transdb.h>

char sample_descr[] = "Sample 'exclusive' demonstrates the Exclusive transaction manager.\n";
const char * db_name = "exclusive";
char explanation[] =  "\n\tTwo threads are started to run in parallel.  The first thread writes\n"
                      "\tto record A and the second to record B.  As they write the database \n"
                      "\trecords, output is written also to a memory buffer which is unspooled\n"
                      "\tafterwards to show the actual order of operations.\n"
                      "\tThe thread interaction can be affected by three command line arguments:\n"
                      "\t   n_iterations - number of records written to output (default=10) \n"
                      "\t   nap_duration1 - sleep time for task 1 (default=10 msec)\n"
                      "\t   nap_duration2 - sleep time for task 2 (default=5 msec)\n\n";

#define N_THREADS 2 
#define N_ITERATIONS 10 
#define NAP_DURATION_1 10 
#define NAP_DURATION_2 5 

/* Thread parameters */
static sample_task_t tasks[N_THREADS];
unsigned int n_iterations = N_ITERATIONS;
int nap_duration1 = NAP_DURATION_1;
int nap_duration2 = NAP_DURATION_2;
char buf[4096];

/* Write tag to memory buffer */
void mem_write( char * tag) {
  strcat(buf, tag);
}

void task1( sample_task_t * descriptor ) 
{
  MCO_RET rc;
  uint4 i;

  for (i = 0; i < n_iterations; i++) 
  { 
    mco_trans_h t;
    A a;
    char tag[64];

    rc = mco_trans_start(descriptor->db_connection, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t); 
    if ( MCO_S_OK == rc ) {       
      sprintf(tag, "Stream 1, rec %u\n", i);
      mem_write(tag);
      rc = A_new(t, &a);
      rc = A_ui4_put(&a, i);
      rc = A_tag_put(&a, tag, strlen(tag)); 
      sample_sleep(nap_duration1);
      rc = mco_trans_commit(t);
    }
  }
}

void task2( sample_task_t * descriptor ) 
{
  MCO_RET rc;
  uint4 i;

  for (i = 0; i < n_iterations; i++) 
  { 
    mco_trans_h t;
    B b;
    char tag[64];

    rc = mco_trans_start(descriptor->db_connection, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t); 
    if ( MCO_S_OK == rc ) {       
      sprintf(tag, "Stream 2, rec %u\n", i);
      mem_write(tag);
      rc = B_new(t, &b);
      rc = B_ui4_put(&b, i);
      rc = B_tag_put(&b, tag, strlen(tag));   
      sample_sleep(nap_duration2);
      rc = mco_trans_commit(t);
    }
  }
}    

MCO_RET do_test() 
{ 
  MCO_RET rc;
  sample_memory_t dbmem;
  rc = sample_open_database( db_name, transdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, N_THREADS, &dbmem );
  if ( MCO_S_OK == rc ) {

    sample_start_connected_task(&tasks[0], task1, db_name, 0);
    sample_start_connected_task(&tasks[1], task2, db_name, 0);
    sample_join_task(&tasks[0]);
    sample_join_task(&tasks[1]);
    
    sample_close_database(db_name, &dbmem);
  }
  
  return rc;
}                      

int main(int argc, char* argv[]) 
{    
  MCO_RET rc;

  sample_os_initialize(DEFAULT);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  sample_header(sample_descr);
  printf(explanation);

  /* Get command line args if any: arg1=n_iterations, arg2=nap_duration1, arg3=nap_duration2 */
  if ( argc > 1 ) {
    n_iterations = atoi(argv[1]);
  }
  if ( argc > 2 ) {
    nap_duration1 = atoi(argv[2]);
  }
  if ( argc > 3 ) {
    nap_duration2 = atoi(argv[3]);
  }

  rc = do_test();
  printf(buf);

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
