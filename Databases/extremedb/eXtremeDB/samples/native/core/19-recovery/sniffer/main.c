/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <common.h>
#include <snifferdb.h>


char sample_descr[] = "Sample 'sniffer' demonstrates recovery using sniffer to detect dead connections.\n";
const char * db_name = "sniffer";
#ifndef _INTEGRITY
char explanation[] =  "\n\tStart sniffer without command line arguments to use the default\n"
                      "\tvalue of n_iterations=10000.  Then start a second instance of\n"
                      "\tsniffer with the command line argument '10' (n_iterations=10).\n"
                      "\tThe second process will cause a segmentation fault on the 10th\n"
                      "\titeration.  The sniffer loop in the first process will detect\n"
                      "\tand report the 'dead' connection in the second process.\n";
#else
char explanation[] =  "\n\tStart sniffer process from address space 1 to use the default\n"
                      "\tvalue of n_iterations=10000.  Then start a second instance of\n"
                      "\tsniffer from address space 2 for n_iterations=10.\n"
                      "\tThe second process will cause a segmentation fault on the 10th\n"
                      "\titeration.  The sniffer loop in the first process will detect\n"
                      "\tand report the 'dead' connection in the second process.\n";
#endif
#define  DATABASE_SEGMENT_SIZE ( 600 * 1024 )

/* Thread parameters */
#define N_CONNECTIONS 4  
#define N_THREADS 1 
static sample_task_t tasks[N_THREADS];
#define INIT_BALANCE  1000
#define SNIFFER_INTERRVAL 100
static int n_accounts = 300;
#ifndef _INTEGRITY
static int n_iterations = 1000;
#else
extern int n_iterations;
extern int crash;
#endif
volatile int n_conflicts;
volatile int repair = 0;

MCO_RET sniffer_callback(mco_db_h db, void* context, mco_trans_counter_t trans_no)
{
    SAMPLE_OS_TASK_ID pid;

    if ( !context ) {
        /* no context. the connection was maid by call of mco_db_connect() or mco_db_connect_ctx() with null context data */
        return MCO_S_OK;
    }

    pid = *(SAMPLE_OS_TASK_ID *)context;

    if ( sample_os_task_id_check( pid ) == 0 ) {
        return MCO_S_OK;
    }

    repair ++;
    printf("Process %d is crashed\n", (int)pid);
    return MCO_S_DEAD_CONNECTION;
}

void sniffer_loop( sample_task_t * descriptor ) 
{
  mco_db_h db;
  SAMPLE_OS_TASK_ID pid = sample_os_task_id_get();
  /* Connect using mco_db_connect_ctx() and pass &pid as parameter */
  MCO_RET rc = mco_db_connect_ctx(db_name, &pid, &db);
  
  if ( MCO_S_OK == rc ) {
    /* Descriptor->stopped flag is set to 1 by sample_stop_task() in main thread */
    while ( MCO_S_OK == rc && descriptor->stopped == 0 ) {
      rc = mco_db_sniffer(db, sniffer_callback, MCO_SNIFFER_INSPECT_ACTIVE_CONNECTIONS);
      sample_sleep(SNIFFER_INTERRVAL);
    }
    mco_db_disconnect(db);
  }
} 


MCO_RET check_consistency(mco_db_h db, int n_accounts)
{
  mco_trans_h t;    
  int i = 0;
  uint8 total = 0;
  mco_cursor_t c;

  MCO_RET rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);    

  if ( MCO_S_OK == rc ) {       
    rc = Account_list_cursor(t, &c);
    if ( MCO_S_OK == rc ) {

      for (i = 0, rc = mco_cursor_first(t, &c); MCO_S_OK == rc; i++, rc = mco_cursor_next(t, &c))
      {
        Account account;
        uint8 balance;
        rc = Account_from_cursor(t, &c, &account);
        if ( MCO_S_OK == rc ) {
          rc = Account_balance_get(&account, &balance);
          if ( MCO_S_OK == rc ) {
            total += balance;
          }
        }
      }

      if ( i != n_accounts) {
        printf("\n\tLoop counter i=%d different from n_accounts=%d\n", i, n_accounts);
        rc = MCO_S_NOTFOUND;
      }

      if ( total != (uint8)(n_accounts * INIT_BALANCE) ) {
        printf("\n\tTotal=%d different from (n_accounts * INIT_BALANCE)=%d\n", (int)total, n_accounts * INIT_BALANCE);
        rc = MCO_S_NOTFOUND;
      }
    }

    rc = mco_trans_commit(t);
  }

  return rc;
}

MCO_RET transfer_money(mco_db_h db, int dst_account, int src_account, mco_bool crash)
{
  mco_trans_h t;
  uint8 balance;
  Account from;
  Account to;
  MCO_RET rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t); 

  if ( MCO_S_OK == rc ) {       
    rc = Account_idx_find(t, src_account, &from);
    if ( MCO_S_OK == rc ) {       
      rc = Account_idx_find(t, dst_account, &to);
      if ( MCO_S_OK == rc ) {       
        rc = Account_balance_get(&from, &balance);
        if ( MCO_S_OK == rc ) {     
          if (balance == 0) { 
            rc = MCO_S_BUSY;
          } else { 
            rc = Account_balance_put(&from, balance - 1);
            if ( MCO_S_OK == rc ) {       
              rc = Account_balance_get(&to, &balance);
              if ( MCO_S_OK == rc ) {       
                if (crash) { 
                  /* Cause the "process fatal error" - segmentation fault */
                  sample_os_shutdown();
#ifdef _WIN32
                  TerminateProcess(GetCurrentProcess(),-1);
#else
                  *(int*)1 = 0;
#endif
                }
                rc = Account_balance_put(&to, balance + 1);
              }
            }
          }
        }
      }
    }
    rc = mco_trans_commit(t);
    if ( MCO_S_OK != rc ) { 
      /* If a concurrent access conflict occurred increment conflict counter */
      if ( MCO_E_CONFLICT == rc );
        n_conflicts += 1;
    }
  }

  return rc;
}    
                    
int main(int argc, char* argv[])
{
  MCO_RET rc;
  int i;
  mco_db_h db;
  mco_trans_h t;
  mco_cursor_t c;
  mco_device_t dev[1];
  mco_db_params_t db_params;
  int src_account;
  int dst_account;
  time_t start;
  unsigned long key;
  SAMPLE_OS_TASK_ID pid;
  int do_crash = 0;

  sample_os_initialize(DEFAULT);

  key = (unsigned long)time(0);
  pid = sample_os_task_id_get();
  /* Get command line args if any: arg1=n_iterations, arg2=n_accounts */
#ifndef _INTEGRITY
  if ( argc > 1 ) {
    n_iterations = atoi(argv[1]);
    do_crash = 1;
  }
  if ( argc > 2 ) {
    n_accounts = atoi(argv[2]);
  }
#else
  do_crash = crash;
  mco_sync_first_semaphore_index = 14;
  mco_sync_last_semaphore_index  = 18;
  mco_registry_object_index      = 12; 
#endif

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  sample_header(sample_descr);
  printf(explanation);

  if (do_crash) sample_pause("\n\nPress any key to continue . . .\n");

  dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;
  dev[0].size       = DATABASE_SEGMENT_SIZE;
#ifndef _INTEGRITY
  dev[0].type       = MCO_MEMORY_NAMED;
  sprintf( dev[0].dev.named.name, "%s-db", db_name );
  dev[0].dev.named.flags = 0;
#ifdef __LYNX
  dev[0].dev.named.hint = (void*) 0x0;
#else   /* !__LYNX */
  dev[0].dev.named.hint = DATABASE_MAP_ADDRESS;
#endif  /* !__LYNX */
#else   /* _INTEGRITY */
  dev[0].type       = MCO_MEMORY_INT_DESC;
  dev[0].dev.idesc.handle  = 13;
#endif  /* _INTEGRITY */

  /* Customize the database params */
  mco_db_params_init ( &db_params );
  db_params.mem_page_size            = MEMORY_PAGE_SIZE;
  db_params.disk_page_size           = 0;            /* Pure in-memory database */
  /* Allow up to 10 connections - 2 per process */
  db_params.db_max_connections       = N_CONNECTIONS;
  db_params.connection_context_size  = sizeof(SAMPLE_OS_TASK_ID);
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key              = EXTREMEDB_LICENSE_KEY;
#endif

  rc = mco_db_open_dev(db_name, snifferdb_get_dictionary(), dev, 1, &db_params);

  /* Allow duplicate instances because all processes connect to the same db */
  if ( MCO_S_OK != rc && MCO_E_INSTANCE_DUPLICATE != rc )
  {
      CHECK(rc);
  }

  /* Connect using mco_db_connect_ctx() and pass &pid as parameter */
  CHECK(mco_db_connect_ctx(db_name, &pid, &db));
  CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
  if (Account_list_cursor(t, &c) == MCO_S_CURSOR_EMPTY)
  {
    start = time(0);
    for (i = 0; i < n_accounts; i++) { 
        char buf [64];
        Account account;
        CHECK(Account_new(t, &account));
        sprintf(buf, "Owner %d", i);
        CHECK(Account_owner_put(&account, buf, (uint2)strlen(buf)));
        CHECK(Account_id_put(&account, i));
        CHECK(Account_balance_put(&account, INIT_BALANCE));
    }
    CHECK(mco_trans_commit(t));
    printf("Elapsed time for inserting %d records: %d\n", n_accounts, (int)(time(0) - start));
  }

  n_conflicts = 0;

  if (!do_crash) sample_start_task(&tasks[0], sniffer_loop, 0);

  do {
    start = time(0);
    for (i = 0; i < n_iterations; i++) { 
        check_consistency(db, n_accounts);
        /* Use (Pi * key + e) % 1000000007L to generate a random key value */
        key = ((3141592621L * key + 2718281829L) % 1000000007L);
        src_account = key % n_accounts; 
        key = ((3141592621L * key + 2718281829L) % 1000000007L);
        dst_account = key % n_accounts; 
        while (1) { 
            rc = transfer_money(db, dst_account, src_account, (i == n_iterations - 1) && do_crash);
            if (rc == MCO_S_BUSY) { 
                src_account = (src_account + 1) % n_accounts;
            } else if (rc != MCO_S_OK && rc != MCO_E_CONFLICT) {
                printf("Transaction error %d\n", rc);
                sample_os_shutdown();
                return 1;
            } else {
                break;
            } 
        } 
    } 
    printf("Elapsed time for performing %d transactions: %d, number of conflicts: %d\n", 
            n_iterations, (int)(time(0) - start), n_conflicts);
  } while(!do_crash && !repair);

  sample_stop_task(&tasks[0]);
  sample_join_task(&tasks[0]);

  CHECK(mco_db_disconnect(db));
  CHECK(mco_db_close(db_name));

  CHECK(mco_runtime_stop());

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
