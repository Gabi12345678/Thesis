/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mco.h>
#include <common.h>
#include "perf.h"
char sample_descr[] = "Sample 'perf' demonstrates eXtremeDB performance for basic operations.\n";
char explanation[] =  "\n\tFirst N objects are inserted into a class, a hash index is created\n"
                      "\ton insertion, then used for searches.  Then a tree index is built\n"
                      "\tand searches are performed by key, then sequentially.\n"
                      "\tFinally, the tree index is removed and all objects are deleted\n"
                      "\tone-by-one.  Each insert and delete is done in a separate\n"
                      "\ttransaction, so that the commit time is included in measurements.\n\n";
const char * db_name = "perf";

#undef DATABASE_SIZE
/* If you change the number of objects inserted, make sure that you
 * first have enough memory (DATABASE_SIZE), and also declare the hash table
 * size appropriately (hkey[estimated_number_of_entries] in perf.mco
 */
#ifdef _INTEGRITY
#define  DATABASE_SIZE        (2 * 1024 * 1024 * MEMORY_SIZE_X64_MULT + MEMORY_SIZE_WCHAR_ADD)
const unsigned int nRecords = 30000;
#else
#define  DATABASE_SIZE        (24 * 1024 * 1024 * MEMORY_SIZE_X64_MULT + MEMORY_SIZE_WCHAR_ADD)
unsigned int nRecords       = 500000;
#endif

#undef CACHE_SIZE
#define CACHE_SIZE             (64 * 1024 * 1024)
const unsigned int nRecordsDisk = 1000;

void err_in_trn(uint4 trn_no, MCO_RET rc, char* file, int line)
{
  printf("%s@%d: error in transaction. Trans. #%d, rc==%d\n", file, line, trn_no, rc);
}

void report_mem(mco_db_h db)
{
  uint2 psize;
  mco_size32_t fpages, tpages;

  mco_db_free_pages(db, &fpages);
  mco_db_total_pages(db, &tpages);
  mco_db_page_size(db, &psize);

  printf("Memory usage %d from %d bytes\n", (tpages - fpages)*psize, tpages*psize);
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
  mco_runtime_info_t info;
  mco_trans_h t;
  time_t start_time;
  unsigned int i;
  long n = 0;
  Record rec;
  mco_cursor_t c;
  uint4 key = 1999;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);
  printf(explanation);

  /* get runtime info */
  mco_get_runtime_info(&info);

  if (info.disk_supported) {
      nRecords = nRecordsDisk;
      unlink("perf.log");
      unlink("perf.dbs");
  }

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open databases */
  rc = sample_open_database( db_name, perf_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      report_mem(db);

      /* Insert Records, don't create the tree index yet */
      printf("\tInsert:\n\t");
      start_time = sample_msec();

      /* Insert records; one object per transaction. */
      for (i = 0; i < nRecords && MCO_S_OK == rc; i++) {
        key = i * 2;

        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          rc = Record_new(t, &rec);
          if ( MCO_S_OK == rc ) {
            rc = Record_key_put(&rec, key);
          }
          if (i % (nRecords / 10) == 0)
          {
            printf(".");
          }
          rc = mco_trans_commit(t);
        }
      }
      printf("\n\t%d objects: %d milliseconds,(%d microsecs/object)\n", (int)i, 
              (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / i));

      report_mem(db);

      /* Hash search */
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        printf("\n\tHash search:\n\t");
        start_time = sample_msec();
        for (i = 0; i < nRecords * 2; i++) {
          Record obj;
          uint4 ui4;

          key = i;
          rc = Record_hkey_find(t, key, &obj);
          if (i % 2 == 0)
          {
            if ( MCO_S_OK == rc ) {
              rc = Record_key_get(&obj, &ui4);
              if (i != ui4) {
                err_in_trn(i, rc, __FILE__, __LINE__);
              }
            } else {
              err_in_trn(i, rc, __FILE__, __LINE__);
            }
          }
          else
          {
            if (MCO_S_OK == rc)
            {
              err_in_trn(i, rc, __FILE__, __LINE__);
            }
          }

          if (i % (nRecords / 10) == 0)
          {
            printf(".");
          }
        }
      }

      report_mem(db);

      mco_trans_commit(t);
      printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", (nRecords * 2), 
              (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / (nRecords * 2)));

      /* Tree search */
      printf("\n\tTree search:\n\t");
      start_time = sample_msec();

      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {

        rc = Record_tkey_index_cursor(t, &c);
        if ( MCO_S_OK == rc ) {

          for (i = 0; i < nRecords* 2; i++)
          {
            key = i;
            rc = Record_tkey_search(t, &c, MCO_EQ, key);

            if (i % 2 == 0)
            {
              Record obj;
              uint4 ui4;

              if ( MCO_S_OK == rc ) {
                rc = Record_from_cursor(t, &c, &obj);
                if ( MCO_S_OK == rc ) {
                  rc = Record_key_get(&obj, &ui4);
                  if (i != ui4)
                  {
                    err_in_trn(i, rc, __FILE__, __LINE__);
                  }
                }
              } else {
                err_in_trn(i, rc, __FILE__, __LINE__);
              }


            }
            else
            {
              if (MCO_S_OK == rc)
              {
                err_in_trn(i, rc, __FILE__, __LINE__);
              }
            }

            if (i % (nRecords / 10) == 0)
            {
              printf(".");
            }
          }
        }
        mco_trans_commit(t);
      }
      printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", (nRecords * 2), 
              (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / (nRecords * 2)));

      report_mem(db);

      /* Cursor movements */
      printf("\n\tSequential:\n\t");
      start_time = sample_msec();
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {

        rc = Record_tkey_index_cursor(t, &c);
        if ( MCO_S_OK == rc ) {
          i = 0;
          for (n = 0, rc = mco_cursor_first(t, &c); rc == MCO_S_OK; rc = mco_cursor_next(t, &c), i++)
          {
            Record obj;
            uint4 ui4;

            rc = Record_from_cursor(t, &c, &obj);
            if ( MCO_S_OK == rc ) {
              rc = Record_key_get(&obj, &ui4);
              if (i* 2 != ui4)
              {
                err_in_trn(i, rc, __FILE__, __LINE__);
              }
            }
            if (n % (nRecords / 10) == 0)
            {
              printf(".");
            }
            n++;
          }
          if (MCO_S_CURSOR_END != rc)
          {
            err_in_trn(n, rc, __FILE__, __LINE__);
          }
        }
        mco_trans_commit(t);
      }

      printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", (int)n, 
              (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / n));

      report_mem(db);

      /* Search using hash index and delete all objects  */
      printf("\n\tSearch/delete:\n\t");
      start_time = sample_msec();
      for (i = 0; i < nRecords* 2; i++)
      {
        key = i;

        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {

          rc = Record_hkey_find(t, key, &rec);
          if (i % 2 == 0)
          {
            uint4 ui4;

            if ( MCO_S_OK == rc ) {
              rc = Record_key_get(&rec, &ui4);
              if (i != ui4)
              {
                err_in_trn(i, rc, __FILE__, __LINE__);
              }
              rc = Record_delete(&rec);
            } else {
              err_in_trn(i, rc, __FILE__, __LINE__);
            }

          } else {
            if ( MCO_S_OK == rc )
            {
              err_in_trn(i, rc, __FILE__, __LINE__);
            }
          }
          rc = mco_trans_commit(t);

          if (i % (nRecords / 10) == 0)
          {
            printf(".");
          }
        }
      }

      printf("\n\t%d objects: %d milliseconds (%d microsecs/object)\n", (unsigned)i, 
              (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / i));

      report_mem(db);

      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
