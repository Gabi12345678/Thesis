/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "characterdb.h"

#undef MEMORY_PAGE_SIZE
#define MEMORY_PAGE_SIZE 192

char sample_descr[] = {
  "Sample 'character' demonstrates eXtremeDB patricia tree match functions\n"
  "using a string key.\n"
};
const char * db_name = "characterdb";

MCO_RET createDatabase(mco_db_h db);
MCO_RET doListing(mco_db_h db);
MCO_RET doGoogle(mco_db_h db);

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
  rc = sample_open_database( db_name, characterdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      rc = createDatabase(db);
      if ( MCO_S_OK == rc ) {
        doListing(db);
        doGoogle(db);
      }

      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
