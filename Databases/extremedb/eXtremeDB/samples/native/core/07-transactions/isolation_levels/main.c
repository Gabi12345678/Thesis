/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <common.h>

#include "isolationdb.h"

char sample_descr[] = "Sample 'isolation_levels' demonstrates the difference in behavior\n"
                      "using different isolation levels with the MVCC transaction manager.\n";
const char * db_name = "isolationdb";
char explanation[] =  "\n\tTwo Account records are created and initialized as follows:\n"
                      "\t\tAccount Id=1, balance=1000\n"
                      "\t\tAccount Id=2, balance=10000\n"
                      "\n\tThen for each isolation level two threads are spawned to update\n"
                      "\tthe Account records in parallel transactions as follows:\n"
                      "\t\t1) Deposit 100 to Account 1 in transaction 1\n"
                      "\t\t2) Display the resulting balances in transaction 2.\n"
                      "\t\t3) Get the resulting balance of Account 1.\n"
                      "\t\t4) Deposit the balance of Account 2 to Account 1.\n"
                      "\t\t5) Withdraw the previous balance of Account 1 from Account 2.\n"
                      "\t\t6) Display the resulting balances.\n";

#define DELAY        100 /* Small delay in milliseconds to start threads */
#define N_THREADS    2 
static sample_task_t tasks[N_THREADS];

void error_handler(MCO_RET n) 
{
  fprintf(stderr, "eXtremeDB runtime fatal error: %d\n", n);
  sample_os_shutdown();
  dbg_exit(-1);
}

mco_bool withdraw(mco_trans_h t, int4 id, uint4 amount)
{
  Account account;
  uint4 balance;
  mco_bool ret = MCO_YES;
  
  MCO_RET rc = Account_by_id_find(t, id, &account);

  if ( MCO_S_OK == rc ) {
    Account_balance_get(&account, &balance);
    if (balance >= amount) {
      balance -= amount;
      Account_balance_put(&account, balance);
      ret = MCO_YES;
    }
    ret = MCO_NO;
  } else {
    printf("Account not found for id=%d", id);
    ret = MCO_NO;
  }
  return ret;
}

void deposit(mco_trans_h t, int4 id, uint4 amount)
{
  Account account;
  uint4 balance;
  
  MCO_RET rc = Account_by_id_find(t, id, &account);

  if ( MCO_S_OK == rc ) {
    Account_balance_get(&account, &balance);
    balance += amount;
    Account_balance_put(&account, balance);
  } else {
    printf("Account not found for id=%d", id);
  }
}

uint4 get_balance(mco_trans_h t, int4 id)
{
  Account account;
  uint4 balance = 0;

  MCO_RET rc = Account_by_id_find(t, id, &account);

  if ( MCO_S_OK == rc ) {
    Account_balance_get(&account, &balance);
  } else {
    printf("Account not found for id=%d", id);
  }
  return balance;
}

void create_account(mco_trans_h t, int4 id, uint4 amount) 
{
  Account account;

  MCO_RET rc = Account_new(t, &account);
  
  if ( MCO_S_OK == rc ) {
    Account_id_put(&account, id);
    Account_balance_put(&account, amount);
  } else {
    printf("Account not found for id=%d", id);
  }
}

MCO_RET initialize_database(mco_db_h db) 
{
  mco_trans_h t;

  MCO_RET rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);    
  
  if ( MCO_S_OK == rc ) {
    create_account(t, 1, 1000);
    create_account(t, 2, 10000);
    rc = mco_trans_commit(t);
  }
  return rc;
}

MCO_RET dump_database(mco_trans_h t) 
{ 
  mco_cursor_t c;
  
  MCO_RET rc = Account_by_id_index_cursor(t, &c);

  if ( MCO_S_OK == rc ) {
  
    printf("\tID\tBalance\n");
    for (rc = mco_cursor_first(t, &c); MCO_S_OK == rc; rc = mco_cursor_next(t, &c))
	  {
      Account account;
      uint4 balance;
      int4  id;
      Account_from_cursor(t, &c, &account);
      Account_id_get(&account, &id);
      Account_balance_get(&account, &balance);
      printf("\t%d\t%u\n", id, balance);
    }
    printf("\t-----------------------\n");
  }
  return rc;
}
    
void make_deposit( sample_task_t * descriptor ) 
{
  mco_db_h db;
  mco_trans_h t;
  MCO_TRANS_ISOLATION_LEVEL isolation_level = *(MCO_TRANS_ISOLATION_LEVEL*)descriptor->param;

  /* Connect to make deposit */
  MCO_RET rc = mco_db_connect(db_name, &db);
 
  if ( MCO_S_OK == rc ) {
    mco_trans_set_default_isolation_level(db, isolation_level);

    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);    
    if ( MCO_S_OK == rc ) {
      deposit(t, 1, 100);
      rc = mco_trans_commit(t);
    }
    mco_db_disconnect(db);
  }
}

void make_transfer( sample_task_t * descriptor ) 
{
  mco_db_h db;
  mco_trans_h t;
  uint4 balance;
  MCO_TRANS_ISOLATION_LEVEL isolation_level = *(MCO_TRANS_ISOLATION_LEVEL*)descriptor->param;

  /* Connect to make balance transfer */
  MCO_RET rc = mco_db_connect(db_name, &db);

  if ( MCO_S_OK == rc ) {
    mco_trans_set_default_isolation_level(db, isolation_level);

    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);    
    if ( MCO_S_OK == rc ) {
      balance = get_balance(t, 2);    
      deposit(t, 1, balance);
      rc = mco_trans_commit(t);
    }
    mco_db_disconnect(db);
  }
}

char* isolation_level_name[] = {
  "",
  "READ COMMITTED",
  "REPEATABLE READ",
  "",
  "SERIALIZABLE"
};

MCO_RET do_test(MCO_TRANS_ISOLATION_LEVEL isolation_level) 
{ 
  mco_db_h db;
  sample_memory_t dbmem;
  mco_trans_h t;
  uint4 balance;
  
  MCO_RET rc = sample_open_database( db_name, isolationdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1 + N_THREADS, &dbmem );

  if ( MCO_S_OK == rc ) {

    /* Connect to set isolation level */
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {
      mco_trans_set_default_isolation_level(db, isolation_level);
      printf("\tIsolation level %s\n\n", isolation_level_name[isolation_level]); 
   
      rc = initialize_database(db);

      if ( MCO_S_OK == rc ) {

        /* Deposit 100 to Account 1; read balances */
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);  
        if ( MCO_S_OK == rc ) {
          printf("\tDeposit $100 to Account 1, read balances in parallel transaction:\n");
          sample_start_connected_task(&tasks[0], make_deposit, db_name, &isolation_level);
          sample_sleep(DELAY);
          dump_database(t);
          rc = mco_trans_commit(t);
          if ( MCO_S_OK == rc ) {
            sample_join_task(&tasks[0]);
          }
        }

        /* Deposit to Account 1 and Withdraw from Account 2 */
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);  
        if ( MCO_S_OK == rc ) {
          printf("\tParallel Deposit to Account 1 and Withdraw from Account 2\n");
          balance = get_balance(t, 1);
          sample_start_connected_task(&tasks[1], make_transfer, db_name, &isolation_level);
          sample_sleep(DELAY);
          withdraw(t, 2, balance);
          rc = mco_trans_commit(t);
          if ( MCO_S_OK == rc ) {
            sample_join_task(&tasks[1]);
          }
        }

        /* Start transaction to show database results */
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);  
        if ( MCO_S_OK == rc ) {
          dump_database(t);
          rc = mco_trans_commit(t);
        }
      }

      mco_db_disconnect(db);   
    }

    sample_close_database(db_name, &dbmem);
  }
  
  return rc;
}                      

MCO_RET isolation_level(MCO_TRANS_ISOLATION_LEVEL level)
{ 
  MCO_RET rc = MCO_S_OK;

  if (mco_trans_get_supported_isolation_levels() & level) { 
    rc = do_test(level);
  }
  return rc;
}
        
int main(int argc, char* argv[])
{
  MCO_RET rc;

  sample_os_initialize(DEFAULT);

  /* set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  sample_header(sample_descr);
  printf(explanation);

  sample_pause("\n\nPress any key to continue . . .\n");
  rc = isolation_level(MCO_READ_COMMITTED);

  if ( MCO_S_OK == rc ) {
    sample_pause_end("\n\nPress any key to continue . . .\n");
    rc = isolation_level(MCO_REPEATABLE_READ);
  }
  if ( MCO_S_OK == rc ) {
    sample_pause_end("\n\nPress any key to continue . . .\n");
    isolation_level(MCO_SERIALIZABLE);
  }

  mco_runtime_stop();
  
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}
