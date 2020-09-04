/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <math.h>
#include <time.h>
#include <common.h>
#include "errordb.h"

char sample_descr[] = {
  "Sample 'nonfatalerr' demonstrates handling of eXtremeDB non-fatal error codes.\n"
};
const char * db_name = "errordb";

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
   
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, errordb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, 0, 1, &dbmem );
  sample_rc_check("\tOpen database", rc );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    sample_rc_check("\tConnect to database", rc );
    if ( MCO_S_OK == rc ) {

      printf("\n\n\tTry to close without disconnecting...");
      rc = mco_db_close(db_name);
      sample_rc_check("\n\tClose database", rc );

      rc = mco_db_disconnect(db);
      sample_rc_check("\n\tDisconnect from database", rc );
    }

    rc = sample_close_database(db_name, &dbmem);
    sample_rc_check("\n\tClose database", rc );
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}
