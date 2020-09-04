/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <errordb.h>

char sample_descr[] = {
  "Sample 'errorhandler' demonstrates setting an eXtremeDB error handler.\n"
};
const char * db_name = "errordb";

/* Implement error handler */
static void errhandler(MCO_RET n)
{
    printf("\neXtremeDB runtime fatal error: %d", n);
    getchar();
    sample_os_shutdown();
    dbg_exit( -1 );
}

int main(int argc, char* argv[])
{
  MCO_RET   rc;
  mco_db_h  db;
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set the error handler to be called from the eXtremeDB runtime if a fatal error occurs */
  mco_error_set_handler(&errhandler);
  printf("\n\tUser-defined error handler set\n");

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  /* Open and connect to database */
  rc = sample_open_database( db_name, errordb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, 0, 1, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Do database work... */

      /* Display an explanatory message ... */
      char text[] = "\n\tIf a fatal error occurs the user-defined errorhandler will be called.\n\n"
                      "\tFatal errors are unrecoverable and cause the eXtremeDB runtime to call \n"
                      "\tthe function mco_stop() which enters an infinite loop or calls the\n"
                      "\tC-runtime assert() to keep the application running so that the runtime\n"
                      "\tstack can be debugged.  This behavior can be overridden by calling the\n"
                      "\tmco_error_set_handler() API to cause mco_stop() to instead call the\n"
                      "\tuser-defined error handler.";
      printf(text);

      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}
