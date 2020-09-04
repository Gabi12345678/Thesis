/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "trigramdb.h"

char sample_descr[] = { "Sample 'trigram' demonstrates trigram index pattern searches.\n" };
const char * db_name = "trigramdb";

#define MAX_OBJ_COUNT 4

int main(int argc, char* argv[])
{
  MCO_RET           rc;
  mco_db_h          db = 0;
  sample_memory_t   dbmem;
  mco_trans_h       t;
  mco_cursor_t      c;
  
  uint4 result = 0;
  char dest[64] = "";
  char src[4][16] = { " 77a474ko", " 77u768cc", " 69a314pi", "177x278ee" };
  const char * search_pattern[6][16] = { "768", "4pi", " 77", "8cc", "7u7", " 77a474ko" };

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, trigramdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if (MCO_S_OK == rc) {
      int i;

      /* Fill db */
      printf("Filling DB with the following id's and patterns:\n");
      CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
      for (i = 0; i < MAX_OBJ_COUNT; i++)
      {
        Obj x;

        Obj_new(t, &x);
        Obj_id_put(&x, (uint4)(i));
        Obj_carid_put(&x, &src[i][0], strlen(&src[i][0]));
        printf("\t%u: %s\n", i, &src[i][0]);
      }
      CHECK(mco_trans_commit(t));

      /* perform trigram searches on data */
      printf("\nPerforming Trigram searches on database:\n");
      CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t));
      rc = Obj_by_carid_index_cursor(t, &c);
      if (MCO_S_OK == rc)
      {
        int j;

        for (j = 0; j < MAX_OBJ_COUNT; j++)
        {
          do 
          {
            rc = Obj_by_carid_search(t, &c, MCO_CONTAIN, search_pattern[0][j], strlen(search_pattern[0][j]));
            printf("\nSearching for pattern \"%s\"\n", search_pattern[0][j]);
            while (MCO_S_OK == rc)
            {
               Obj a;

               rc = Obj_from_cursor(t, &c, &a);
               if (MCO_S_OK == rc)
               {
                 Obj_id_get(&a, &result);
                 Obj_carid_get(&a, dest, sizeof(dest));
                 printf("\t%u: %s\n", result, dest);
               }
              rc = mco_cursor_next(t, &c);
            }
            break;
          } while (MCO_S_OK == rc);
        }
      }
      CHECK(mco_trans_rollback(t));
      rc = mco_db_disconnect(db);
    }
    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
