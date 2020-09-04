/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mco.h>
#include <common.h>
#include "vectors.h"
char sample_descr[] = "Sample 'vectors' demonstrates the use of vectors and vector-based indexes.\n";
const char * db_name = "vecdb";
const int  n_boxes = 100;

MCO_RET create_box(mco_trans_h t, uint4 volume, const char* name);

static void show_mem(mco_db_h db)
{
  uint4 totalpg, freepg;
  uint2 pgsize;
  mco_db_free_pages(db, &freepg);
  mco_db_total_pages(db, &totalpg);
  mco_db_page_size(db, &pgsize);

  printf("\n\tMem: total pgs=%d, free pgs=%d, used and reserved %d Kb", totalpg, freepg, (totalpg - freepg)* pgsize /
         1024);
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
  int j;
  char boxname[100];
  mco_trans_h t;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  rc = sample_open_database( db_name, vectors_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Create objects filled with the various vector fields defined in vector.mco */
      printf("\tCreating %d objects with vectors declared in vector.mco...\n\t", n_boxes);
      for (j = 0; j < n_boxes && MCO_S_OK == rc; j++)
      {
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {

          sprintf(boxname, "Box %d", j);
          rc = create_box(t, 4000 + (j % 10) * 100, boxname);
          if ( MCO_S_OK == rc ) {
            rc = mco_trans_commit(t);
          } else {
            mco_trans_rollback(t);
          }
        }

        if (j % 5 == 0)
        {
          printf(".");
        }
      }

      show_mem(db);

      mco_db_disconnect(db);
    }
  
    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
