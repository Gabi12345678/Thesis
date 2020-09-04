/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include <mcowrap.h>
#include "patterndb.h"

char sample_descr[] = "Sample 'pattern' demonstrates basic eXtremeDB pattern search methods.\n";
const char * db_name = "pattern";

/* Default search patterns */
#define PSEARCH_K1 "abc_1*"
#define PSEARCH_K2 "??c_?2?"
#define PSEARCH_K3 1

#define NUMOBJECTS    20

/* Pattern search 
 * You can change '?' and '*' symbols to any ascii char
 * or you can ignore fields other than char<n> or string.
 */
MCO_RET psearch(mco_db_h db, char* pattern1, char* pattern2, int p3, mco_bool ignore_other_fields)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h trn;
  mco_cursor_t csr;
  TestClass test;
  uint2 size1;
  uint4 bsize;
  char key[64], k2[64];
  int4 k3, v1;
  void* buf;
  mco_pattern_policy_t p;

  /* Get memory for set of patterns */
  TestClass_i1_pattern_size(pattern1, (uint2)strlen(pattern1), pattern2, (uint2)strlen(pattern2), 0, &bsize);
  buf = malloc(bsize);

  /* Start read-write transaction to change pattern policy */
  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {
    mco_get_pattern_policy(trn, &p);
    p.ignore_other_fields = ignore_other_fields; /* Ignore fields other than char<n> or string */
    mco_set_pattern_policy(trn, &p);

    rc = TestClass_i1_index_cursor(trn, &csr);
    for (rc = TestClass_i1_pattern_search(trn, &csr, buf, bsize, pattern1, (uint2)strlen(pattern1), pattern2,
         (uint2)strlen(pattern2), p3); MCO_S_OK == rc; rc = TestClass_i1_pattern_next(trn, &csr, buf))
    {
      TestClass_from_cursor(trn, &csr, &test);

      TestClass_v1_get(&test, &v1);
      TestClass_k3_get(&test, &k3);
      TestClass_key_get(&test, key, (uint2)sizeof(key), &size1);
      TestClass_k2_get(&test, k2, (uint2)sizeof(k2));

      printf("\t Found: v1=%d, key=%s(%d), k2=%s(%d), k3=%d\n", v1, key, size1, k2, (int)strlen(k2), k3);
    }
    rc = MCO_S_OK;
    mco_trans_commit(trn);
  } else {
    sample_rc_check("\tUnable to start transaction", rc );
  }
  free(buf);
  return rc;
}

MCO_RET newobj(mco_db_h db, int id)
{
  MCO_RET rc;
  mco_trans_h t;
  TestClass hClass;
  char src1[64], src2[64];
  int4 v1 = (int4)(rand() &0x3FF);
  int4 k3 = (int4)(rand() % 3);

  /* Open a read_write transaction with FOREGROUND priority level */
  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    rc = TestClass_new(t, &hClass);
    TestClass_v1_put(&hClass, v1);
    TestClass_k3_put(&hClass, k3);
    sprintf(src1, "abc_%03d", rand() & 0xFF);
    TestClass_key_put(&hClass, src1, (uint2)strlen(src1));
    sprintf(src2, "abc_%03d", rand() & 0x1F);
    TestClass_k2_put(&hClass, src2, (uint2)strlen(src2));
    rc = mco_trans_commit(t);
    if ( MCO_S_OK != rc ) {
      sample_rc_check("Unable to commit transaction", rc );
    }
    printf("\n\t v1=%d, k1=%s, k2=%s k3=%d\tinserted okay", v1, src1, src2, k3);

  } else {
    sample_rc_check("\tUnable to start transaction", rc );
  }
  return rc;
}

int main(int argc, char** argv)
{
  MCO_RET rc;
  mco_db_h db = 0;
  int num = 0;
  sample_memory_t dbmem;
  char * pattern1 = PSEARCH_K1;
  char * pattern2 = PSEARCH_K2;
  int    pattern3 = PSEARCH_K3;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Convert optional command line args to pattern values */
  if ( argc > 1 ) {
    pattern1 = argv[1];
  }
  if ( argc > 2 ) {
    pattern2 = argv[2];
  }
  if ( argc > 3 ) {
    pattern3 = atoi(argv[3]);
  }
 
  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, patterndb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      srand((unsigned)time(NULL));
      printf("\n\tFilling database with random values:");

      for (num = 0; num < NUMOBJECTS && MCO_S_OK == rc; ++num)  {
        rc = newobj(db, num);
      }
      if (MCO_S_OK == rc) {

        printf("\n\n\tStart search using all patterns and other index components\n");
        rc = psearch(db, pattern1, pattern2, pattern3, MCO_NO);
        if (MCO_S_OK == rc) {
          printf("\n\tStart search using all patterns and ignore other index components\n");
          psearch(db, pattern1, pattern2, pattern3, MCO_YES);
        }
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
