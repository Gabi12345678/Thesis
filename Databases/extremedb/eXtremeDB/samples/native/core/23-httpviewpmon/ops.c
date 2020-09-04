#include <mco.h>
#include <common.h>
#include "perf.h"

#include <stdio.h>
#include <time.h>

const unsigned int nRecords = 1000000;

void err_in_trn(uint4 trn_no, MCO_RET rc, char* file, int line)
{
  printf("%s@%d: error in transaction. Trans. #%d, rc==%d(%s)\n", file, line, trn_no, rc, mco_ret_string(rc, 0));
  dbg_exit(rc);
}

void perform_ops(sample_task_t * descriptor)
{
	MCO_RET rc = MCO_S_OK;
	time_t start_time;
	unsigned int i;
	uint4 key = 1999;
	mco_trans_h t;
	Record rec;
	mco_cursor_t c;
	long n = 0;
	mco_db_h db = descriptor->db_connection;

    uint4 *exit_flag = (uint4 *)descriptor->param;

    while (*exit_flag != 1)
    {
        /* insert Records, don't create the tree index yet */
        printf("\n\tInsert:\n\t");
        start_time = sample_msec();

        /* insert records; one object per transaction. */
        for (i = 0; (i < nRecords && MCO_S_OK == rc && (*exit_flag != 1)); i++) {
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
        if (*exit_flag == 1)
            return;

        printf("\n\t%d objects: %d milliseconds,(%d microsecs/object)\n", (int)i,
                (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / i));

        /* hash search */
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
        if (*exit_flag == 1)
            return;
        printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", nRecords,
                (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / nRecords));


        /* tree search */
        printf("\n\tTree search:\n\t");
        start_time = sample_msec();

        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {

          rc = Record_tkey_index_cursor(t, &c);
          if ( MCO_S_OK == rc ) {

            for (i = 0; i < nRecords*2 && (*exit_flag != 1); i++)
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
        if (*exit_flag == 1)
            return;
        printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", nRecords,
                (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / nRecords));


        /* cursor movements */
        printf("\n\tSequential:\n\t");
        start_time = sample_msec();
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {

          rc = Record_tkey_index_cursor(t, &c);
          if ( MCO_S_OK == rc ) {
            i = 0;
            for (n = 0, rc = mco_cursor_first(t, &c); rc == MCO_S_OK && (*exit_flag != 1); rc = mco_cursor_next(t, &c), i++)
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
        if (*exit_flag == 1)
            return;

        printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", (int)n,
                (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / n));

        /* Search using hash index and delete all objects  */
        printf("\n\tSearch/delete:\n\t");
        start_time = sample_msec();
        for (i = 0; i < nRecords* 2 && (*exit_flag != 1); i++)
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
        /*  Return if exit flag is 1 - manual exit or 2 - run this loop
            just once in evaluation package
        */
        if (*exit_flag)
        {
            if (*exit_flag == 2)
                printf("\n\nStop the database operations, not to exceed the limitation "
                        "of the evaluation version.\nHit ENTER to stop HTTP server...\n");
            return;
        }
    }
}
