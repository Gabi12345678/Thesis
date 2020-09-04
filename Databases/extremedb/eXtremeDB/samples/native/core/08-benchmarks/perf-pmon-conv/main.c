/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mco.h>
#include <common.h>
#include <mcoperfmon.h>
#include <mcoperfmon_db.h>
#include "perf.h"


char sample_descr[] = "Sample 'perf' demonstrates eXtremeDB performance for basic operations.\n";
char explanation[] =  "\n\tFirst N objects are inserted into a class, a hash index is created\n"
                      "\ton insertion, then used for searches.  Then a tree index is built\n"
                      "\tand searches are performed by key, then sequentially.\n"
                      "\tFinally, the tree index is removed and all objects are deleted\n"
                      "\tone-by-one.  Each insert and delete is done in a separate\n"
                      "\ttransaction, so that the commit time is included in measurements.\n\n";
const char * db_name = "perf.mfd";

#undef DATABASE_SIZE
/* If you change the number of objects inserted, make sure that you
 * first have enough memory (DATABASE_SIZE), and also declare the hash table
 * size appropriately (hkey[estimated_number_of_entries] in perf.mco
 */
#ifdef _INTEGRITY
#define  DATABASE_SIZE        (2 * 1024 * 1024 * MEMORY_SIZE_X64_MULT + MEMORY_SIZE_WCHAR_ADD)
#define  PERF_DATABASE_SEGMENT_SIZE ( 2 * 1024 * 1024 )
const unsigned int nRecords = 30000;
#else
#define  DATABASE_SIZE        (64 * 1024 * 1024 * MEMORY_SIZE_X64_MULT + MEMORY_SIZE_WCHAR_ADD)
#define  PERF_DATABASE_SEGMENT_SIZE ( 8 * 1024 * 1024 )
const unsigned int nRecords = 1000000;
#endif

void err_in_trn(uint4 trn_no, MCO_RET rc, char* file, int line)
{
  printf("%s@%d: error in transaction. Trans. #%d, rc==%d(%s)\n", file, line, trn_no, rc, mco_ret_string(rc, 0));
  dbg_exit(rc);
};

void DisplayPerfColumns(mco_trans_h t, mco_bool counters)
{
	MCO_RET rc;
	mco_cursor_t c;
	PerfCounter cnt;
	uint2 no;
	char name[64];
	uint2 namelen;

	CHECK(PerfCounter_no_idx_index_cursor(t, &c));
	for (rc = mco_cursor_first(t, &c); rc == MCO_S_OK; rc = mco_cursor_next(t, &c)) {
		CHECK(PerfCounter_from_cursor(t, &c, &cnt));

		CHECK(PerfCounter_no_get(&cnt, &no));
		CHECK(PerfCounter_name_get(&cnt, name, 64, &namelen));

		if ((counters && no < MCO_PERF_TIMERS_START) || (!counters && no >= MCO_PERF_TIMERS_START)) {
			printf (",%s", name);
		}
	}
}

void DisplayPerfReadings(mco_trans_h t, uint2 db_no)
{
    MCO_RET rc;
    int i;
    mco_cursor_t c;
    PerfReading reading;
    uint8 stamp;
    uint2 db;
    uint2 nvals;
    uint4 *values = NULL;

    printf ("Readings:\nStamp(ms)");
    DisplayPerfColumns(t, 1);

    CHECK(PerfReading_stamp_idx_index_cursor(t, &c));
    for (rc = mco_cursor_first(t, &c); rc == MCO_S_OK; rc = mco_cursor_next(t, &c)) {
        CHECK(PerfReading_from_cursor(t, &c, &reading));

        CHECK(PerfReading_db_get(&reading, &db));
        if (db != db_no) {
            continue;
        }
        CHECK(PerfReading_stamp_get(&reading, &stamp));
        CHECK(PerfReading_values_size(&reading, &nvals));

        if(values == NULL) {
            values = (uint4 *)malloc(sizeof(uint4) * nvals);
        }
        CHECK(PerfReading_values_get_range(&reading, 0, nvals, values));
        printf("\n%lld", stamp);
        for (i =0; i < nvals; i++) {
        	printf (",%d", values[i]);
        }
    }

    if (values != NULL) {
        free(values);
    }

    printf ("\n");
}

void DisplayPerfTimingReadings(mco_trans_h t, uint2 db_no)
{
	int i;
    MCO_RET rc;
    mco_cursor_t c;
    PerfTimingReading reading;
    uint8 stamp;
    uint2 db;
    uint2 nvals;
    uint8 *current = NULL;

    printf ("Timing readings:\nStamp(ms)");
    DisplayPerfColumns(t, 0);

    CHECK(PerfTimingReading_stamp_idx_index_cursor(t, &c));
    for (rc = mco_cursor_first(t, &c); rc == MCO_S_OK; rc = mco_cursor_next(t, &c)) {
        CHECK(PerfTimingReading_from_cursor(t, &c, &reading));
        CHECK(PerfTimingReading_db_get(&reading, &db));
        if (db != db_no) {
            continue;
        }

        CHECK(PerfTimingReading_stamp_get(&reading, &stamp));
        CHECK(PerfTimingReading_current_size(&reading, &nvals));

        if(current == NULL) {
            current = (uint8 *)malloc(sizeof(uint8) * nvals);
        }
        CHECK(PerfTimingReading_current_get_range(&reading, 0, nvals, current));
        printf("\n%lld", stamp);
        for (i = 0; i < nvals; i++) {
        	printf(",%lld", current[i]);
        }
    }

    if (current != NULL) {
        free(current);
    }
    printf ("\n");
}


void DisplayPerfmonStats()
{
    MCO_RET rc;
    mco_db_h pdb = 0;
    mco_trans_h t;
    mco_cursor_t dbc;
    eXDBDatabase stat_db;
    uint2 db_no;
    char name[20];
    uint2 namelen;

    const char *perfmon_dbname = mco_perfmon_get_db_name();

    CHECK(rc = mco_db_connect(perfmon_dbname, &pdb));

    CHECK(mco_trans_start(pdb, MCO_READ_ONLY, MCO_TRANS_BACKGROUND, &t));
    CHECK(eXDBDatabase_no_idx_index_cursor(t, &dbc));
    for (rc = mco_cursor_first(t, &dbc); rc == MCO_S_OK; rc = mco_cursor_next(t, &dbc)) {
        CHECK(eXDBDatabase_from_cursor(t, &dbc, &stat_db));
        CHECK(eXDBDatabase_no_get(&stat_db, &db_no));
        CHECK(eXDBDatabase_db_name_get(&stat_db, name, 20, &namelen));

        printf("%d. Database %s\n", db_no, name);

        DisplayPerfReadings(t, db_no);
        DisplayPerfTimingReadings(t, db_no);
    }
    CHECK(mco_trans_rollback(t));
    mco_db_disconnect(pdb);
}


int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
  char *perf_memory;
  mco_trans_h t;
  time_t start_time;
  unsigned int i;
  long n = 0;
  Record rec;
  mco_cursor_t c;
  uint4 key = 1999;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);
  printf("%s", explanation);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open databases */
  rc = sample_open_database( db_name, perf_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 2, &dbmem );

  perf_memory = malloc(PERF_DATABASE_SEGMENT_SIZE);
  CHECK( mco_perfmon_init(perf_memory, PERF_DATABASE_SEGMENT_SIZE, NULL));
  CHECK( mco_perfmon_attach(db_name));

  CHECK( mco_db_connect(db_name, &db));

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
  if (rc != MCO_S_OK) {
      CHECK(rc);
  }
  printf("\n\t%d objects: %d milliseconds,(%d microsecs/object)\n", (int)i, 
      (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / i));

  /* Hash search */
  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
      printf("\n\tHash search:\n\t");
      start_time = sample_msec();
      for (i = 0; i < nRecords* 2; i++) {
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

  mco_trans_commit(t);
  printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", nRecords, 
      (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / nRecords));

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
  printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", nRecords, 
      (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / nRecords));

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

  mco_db_disconnect(db);


  /* Print collected database statistics */
  DisplayPerfmonStats();

  CHECK( mco_perfmon_detach(db_name));
  CHECK( mco_perfmon_close());
  sample_close_database(db_name, &dbmem);  

  mco_runtime_stop();

  free (perf_memory);

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
