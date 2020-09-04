/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.    *
 *                                                              *
 ****************************************************************/

#include <mcohv.h>
#include <mcohvperf.h>
#include <mcoperfmon.h>
#include <common.h>
#include "perf.h"

#define METADICT_ENTRY_NUM      2


#ifdef DATABASE_SIZE
#undef DATABASE_SIZE
#endif

#define DATABASE_SIZE (128<<20)
#define  PERF_DATABASE_SEGMENT_SIZE ( 1<<20 )

void perform_ops(sample_task_t * descriptor);

char sample_descr[] = "Sample 'httpview' demonstrates database browser via HTTP protocol.\n";
const char *dbName  = "hvddb";

static void errhandler(MCO_RET n)
{
  printf("\neXtremeDB fatal error: %d", n);
  getchar();
  dbg_exit( - 1);
}

uint4 exit_flag = 0;

int main(int argc, char** argv)
{
  MCO_RET rc;
  mco_runtime_info_t info;
  mcohv_p hv = 0;
  mco_db_h db;
  sample_memory_t sample_dev;
  mco_db_params_t db_params;
  void *perf_memory;
  mco_metadict_header_t *metadict;  /* Meta-dictionary header. Application must allocate memory for it. */
  unsigned int	metadict_size;
  sample_task_t worker_task;

  sample_os_initialize(DEFAULT);

  mco_get_runtime_info(&info);
  if (info.evaluation_version)
  {
    /* Signal worker thread to stop operations and exit */
    exit_flag = 2;
  }

  sample_header(sample_descr);

  /* Set fatal error handler */
  mco_error_set_handler(&errhandler);

  /* Start db engine */
  if (mco_runtime_start() != MCO_S_OK)
  {
    printf("\nUnable to start database engine\n");
    getchar();
    sample_os_shutdown();
    return(-1);
  }

  /* Initialize MCOHV */
  mcohv_initialize();
  /* Initialize perfmon support for MCOHV */
  mcohv_perf_initialize();

  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size for persistent storage */

  rc = sample_open_database(dbName, perf_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 10, &sample_dev);
  sample_rc_check("Open database", rc);
  CHECK(rc);
  printf("\n");

  perf_memory = malloc(PERF_DATABASE_SEGMENT_SIZE);
  if (perf_memory == NULL) {
	  printf ("Not enough memory to create performance monitor database\n");
	  dbg_exit(1);
  }

  CHECK( mco_perfmon_init(perf_memory, PERF_DATABASE_SEGMENT_SIZE, NULL));
  CHECK( mco_perfmon_attach(dbName));

  rc = mco_db_connect(dbName, &db);
  if (MCO_S_OK != rc) {
      printf ("\nUnable to connect to database. Error %d (%s).", rc, mco_ret_string(rc, NULL));
      sample_os_shutdown();
      dbg_exit(rc);
  }

  mco_metadict_size(METADICT_ENTRY_NUM, &metadict_size);
  metadict = (mco_metadict_header_t *) (malloc(metadict_size));

  rc = mco_metadict_init (metadict, metadict_size, 0);
  if ( MCO_S_OK != rc ) {
      // Handle this error
      printf ("\nUnable to initialize metadict. Error %d (%s).", rc, mco_ret_string(rc, NULL));
      free(metadict);
      getchar();
      sample_os_shutdown();
      return(rc);
  }

  /* Start the HTTP server */
  rc = mcohv_start(&hv, metadict, 0, 0);
  printf("mcohv_start(...)==%d\n", rc);

  /* Perform operations on a database */
  sample_start_connected_task(&worker_task, perform_ops, dbName, &exit_flag); /* Start worker thread*/

  /* Simulate the program's event- or data-loop */
  sample_pause("\n\nVisit page http://localhost:8082/\nHit ENTER to stop HTTP server...\n");

  /* Signal worker thread to stop operations and exit */
  exit_flag = 1;

  /* Wait for listener thread */
  sample_join_task(&worker_task);

  /* Stop the server */
  printf("\nmcohv_stop(...)==");
  fflush(stdout);
  printf("%d\n", mcohv_stop(hv));

  /* Final cleanup */
  printf("mcohv_shutdown(...)==");
  fflush(stdout);
  printf("%d\n", mcohv_shutdown());

  free (perf_memory);

  /* Close the database */
  sample_close_database( dbName, &sample_dev);

  /* Shut down database engine */
  mco_runtime_stop();
  sample_os_shutdown();
  return 0;
}
