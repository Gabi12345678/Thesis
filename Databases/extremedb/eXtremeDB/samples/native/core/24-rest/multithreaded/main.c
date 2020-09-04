/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.    *
 *                                                              *
 ****************************************************************/

#include "common.h"
#include "mcorest.h"
#include "restmtdb.h"

#define REST_CHECK(func)  { MCO_RET rc_ = func; if (rc_ != MCO_S_OK) { printf ("\nCall \"" #func "\" at %s:%d failed , code = %d\n", __FILE__, __LINE__, rc_); dbg_exit (1); } }

char sample_descr[] = "Sample 'rest-multithreaded' demonstrates embedded REST Web server in automatic multithreaded mode.\n";
const char *dbName  = "restmtdb";

static const char *rest_host = "127.0.0.1";
static const unsigned short rest_port = 8083;

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
  mcorest_h rest = NULL;
  mcorest_interface_h intf = NULL;
  mco_db_h db;
  unsigned int size;
  mco_device_t dev[1];
  unsigned int n_dev;
  mco_db_params_t db_params;

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

  /* Initialize mcorest */
  REST_CHECK(mcorest_initialize());

  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size for persistent storage */

  /* Set total number of connections to the database */
  /* 1 connection is from this sample application and second from mcorest internals */
  db_params.db_max_connections = 10;

#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

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

  /* Open database */
  rc = mco_db_open_dev(dbName, restmtdb_get_dictionary(), dev, 1, &db_params );
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

  /* Create REST server */
  REST_CHECK(mcorest_create(dbName, &rest));

  /* Add interface */
  REST_CHECK(mcorest_add_interface(rest, rest_host, rest_port, NULL, &intf));

  /* Register REST DB service */
  REST_CHECK(mcorest_svc_db_init(rest));

  /* Start the server */
  REST_CHECK(mcorest_start(rest));

  printf("REST web server address: %s:%u\n", rest_host, rest_port);

  /* Simulate the program's event- or data-loop */
  sample_pause("\nUse the restclient.py script to connect to the REST server and retrieve data.\nHit ENTER to stop HTTP server...");

  /* Stop the server */
  REST_CHECK(mcorest_stop(rest));

  /* Close the interface */
  REST_CHECK(mcorest_interface_close(intf));

  /* Destroy the server */
  REST_CHECK(mcorest_destroy(rest));

  /* Close the database */
  rc = mco_db_close(dbName);
  if (rc != MCO_S_OK) {
    printf("mco_db_close(...) failed %d(%s)\n", rc, mco_ret_string(rc, NULL));
  }

  /* Shut down the REST module */
  REST_CHECK(mcorest_shutdown());

  /* Shut down database engine */
  mco_runtime_stop();

  if (!info.mco_shm_supported)
    free(dev[0].dev.conv.ptr);

  sample_os_shutdown();
  return 0;
}
