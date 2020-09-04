/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.    *
 *                                                              *
 ****************************************************************/

#include <common.h>
#include <mcouda.h>
#include <mcohv.h>
#include "hvddb.h"

#define METADICT_ENTRY_NUM      1

char sample_descr[] = "Sample 'httpview' demonstrates database browser via HTTP protocol.\n";
const char *dbName  = "hvddb";

static void errhandler(MCO_RET n)
{
  printf("\neXtremeDB fatal error: %d", n);
  getchar();
  dbg_exit( - 1);
}

MCO_RET LoadData(mco_db_h db);

int main(int argc, char** argv)
{
  MCO_RET rc;
  mco_runtime_info_t info;
  mcohv_p hv = 0;
  mco_db_h db;
  unsigned int size;
  mco_device_t dev[1];
  unsigned int n_dev;
  mco_db_params_t db_params;
  mco_metadict_header_t *metadict;  /* Meta-dictionary header. Application must allocate memory for it. */

  sample_os_initialize(DEFAULT);

  mco_get_runtime_info(&info);

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

  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size for persistent storage */

  /* Set total number of connections to the database */
  /* 1 connection is from this sample application and second from hv internals */
  db_params.db_max_connections = 10;

#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

  /* Determine size of meta-dictionary (in bytes) and allocate memory */
  mco_metadict_size(METADICT_ENTRY_NUM, &size);
  metadict = (mco_metadict_header_t *) (malloc(size));

  rc = mco_metadict_init (metadict, size, 0);

  if ( MCO_S_OK != rc ) 
  {
    // Handle this error
    printf ("\nUnable to initialize metadict. Error %d (%s).", rc, mco_ret_string(rc, NULL));
    free(metadict);
    getchar();
    sample_os_shutdown();
    return(rc);
  }

  /* Initialize memory device(s) */
  dev[0].assignment  = MCO_MEMORY_ASSIGN_DATABASE;      /* Assign the device as a main database memory */
  dev[0].size        = DATABASE_SIZE;                   /* Set the device size */
  if (info.mco_shm_supported) {
    dev[0].type      = MCO_MEMORY_NAMED;                /* Set the device as a shared named memory device */
    sprintf( dev[0].dev.named.name, "%s-db", dbName );  /* Set memory name */
    dev[0].dev.named.flags = 0;                         /* Zero flags */
    dev[0].dev.named.hint  = 0;                         /* Set mapping address or null it */
  } else {
    dev[0].type       = MCO_MEMORY_CONV;                /* Set the device as a conventional memory device */
    dev[0].dev.conv.ptr  = (void*)malloc( DATABASE_SIZE ); /* Allocate memory and set device pointer */
  }
  n_dev = 1;

  mco_metadict_register(metadict, dbName, hvddb_get_dictionary(), NULL);

  /* Open database */
  rc = mco_uda_db_open(metadict,      /* Meta-dictionary header - must be initialized */
                     0,               /* Dictionary number */
                     dev,             /* Memory devices */
                     n_dev,           /* Num of memory devices */
                     &db_params);     /* db parameters */

  if (MCO_S_OK != rc) {
    printf ("\nUnable to open database. Error %d. (%s)", rc, mco_ret_string(rc, NULL));
    sample_os_shutdown();
    dbg_exit(rc);
  }

  // Connect needed to load data
  rc = mco_db_connect(dbName, &db);
  if (MCO_S_OK != rc) {
      printf ("\nUnable to connect to database. Error %d (%s).", rc, mco_ret_string(rc, NULL));
      sample_os_shutdown();
      dbg_exit(rc);
  }
  /* Load data to the database */
  rc = LoadData(db);
  if (MCO_S_OK != rc) {
      printf ("\nError loading data into database. Error %d (%s).", rc, mco_ret_string(rc, NULL));
      sample_os_shutdown();
      dbg_exit(rc);
  }

  mco_db_disconnect(db);

  /* Start the HTTP server */
  printf("mcohv_start(...)==%d\n", mcohv_start(&hv, metadict, 0, 0));

  /* Simulate the program's event- or data-loop */
  sample_pause("\n\nVisit page http://localhost:8082/\nHit ENTER to stop HTTP server...");

  /* Stop the server */
  printf("mcohv_stop(...)==");
  fflush(stdout);
  printf("%d\n", mcohv_stop(hv));

  /* Destroys the memory manager */
  rc = mco_uda_db_close(metadict, 0);
  if (rc != MCO_S_OK)
  {
    printf("mco_db_close(...) failed %d(%s)\n", rc, mco_ret_string(rc, NULL));
  }

  /* Final cleanup */
  printf("mcohv_shutdown(...)==");
  fflush(stdout);
  printf("%d\n", mcohv_shutdown());

  /* Shut down database engine */
  mco_runtime_stop();

  if (!info.mco_shm_supported)
    free(dev[0].dev.conv.ptr);

  sample_os_shutdown();
  return 0;
}
