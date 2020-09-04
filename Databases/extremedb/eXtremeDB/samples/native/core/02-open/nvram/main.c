/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <common.h>
#include <nvramdb.h>


char sample_descr[] = "Sample 'nvram' demonstrates recovery of database in non-volatile memory.\n";
const char * db_name = "nvram";
#define  DATABASE_SEGMENT_SIZE ( 600 * 1024 )

#define INIT_BALANCE  1000

int n_accounts = 300;
int n_iterations = 100;
void* memory;
mco_runtime_info_t rtinfo;

void check_consistency(mco_db_h db, int n_accounts)
{
    mco_trans_h t;    
    int i = 0;
    uint8 total = 0;
    mco_cursor_t c;
    MCO_RET rc;
    
    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t));    
    
    CHECK(Account_list_cursor(t, &c));
    for (i = 0, rc = mco_cursor_first(t, &c); MCO_S_OK == rc; i++, rc = mco_cursor_next(t, &c))
    {
        Account account;
        uint8 balance;
        CHECK(Account_from_cursor(t, &c, &account));
        CHECK(Account_balance_get(&account, &balance));
        total += balance;
    }
    if (i != n_accounts) {
        printf("\n\tLoop counter i=%d different from n_accounts=%d\n", i, n_accounts);
        sample_os_shutdown();
        dbg_exit(2);
    }
    
    if (total != (uint8)n_accounts * INIT_BALANCE) {
        printf("\n\tTotal=%d different from (n_accounts * INIT_BALANCE)=%d\n", (int)total, n_accounts * INIT_BALANCE);
        sample_os_shutdown();
        dbg_exit(2);
    }
    CHECK(mco_trans_commit(t));
}

void transfer_money(mco_db_h db, int dst_account, int src_account, mco_bool crash)
{
  mco_trans_h t;
  uint8 balance;
  Account from;
  Account to;
  CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t)); 
  CHECK(Account_idx_find(t, src_account, &from));
  CHECK(Account_idx_find(t, dst_account, &to));
  CHECK(Account_balance_get(&from, &balance));
  if (balance != 0) { 
      CHECK(Account_balance_put(&from, balance - 1));
      CHECK(Account_balance_get(&to, &balance));
      if (crash) { 
          size_t rc; 
          FILE* nvram = fopen("nvram.img", "wb");
          rc = fwrite(&memory, 1, sizeof(memory), nvram);
          rc = fwrite(memory, 1, DATABASE_SEGMENT_SIZE, nvram);
          if (rc != DATABASE_SEGMENT_SIZE) { 
              perror("Failed to save NVRAM image");
              sample_os_shutdown();
              dbg_exit(1);
          }
          fclose(nvram);
          /* Cause the "process fatal error" - segmentation fault */
#ifdef _WIN32
          TerminateProcess(GetCurrentProcess(),-1);
#else
          sample_os_shutdown();
          *(int*)1 = 0;
#endif
      }
      CHECK(Account_balance_put(&to, balance + 1));
  }
  CHECK(mco_trans_commit(t));
}    
                    
int main(int argc, char* argv[])
{
    int i;
    size_t rc;
    mco_db_h db;
    mco_trans_h t;
    mco_cursor_t c;
    unsigned long key;
    int src_account;
    int dst_account;
    FILE* nvram;
    time_t start;
    mco_device_t dev[1];
    mco_db_params_t db_params;

    sample_os_initialize(DEFAULT|DISK);
    key = (unsigned long)time(0);

    /* Get command line args if any: arg1=n_iterations, arg2=n_accounts */
    #ifndef _INTEGRITY
    if ( argc > 1 ) {
        n_iterations = atoi(argv[1]);
    }
    if ( argc > 2 ) {
        n_accounts = atoi(argv[2]);
    }
    #endif

    mco_get_runtime_info( &rtinfo );
    
    /* Set fatal error handler and start eXtremeDB runtime */
    mco_error_set_handler(&sample_errhandler);
    mco_runtime_start();
    
    sample_header(sample_descr);
    
    memory = malloc(DATABASE_SEGMENT_SIZE);
    if (memory == 0) { 
        fprintf(stderr, "Failed to allocate %d bytes of memory\n", DATABASE_SEGMENT_SIZE);
        sample_os_shutdown();
        return 1;
    }
    
    dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;
    dev[0].size       = DATABASE_SEGMENT_SIZE;
    dev[0].type       = MCO_MEMORY_CONV;
    dev[0].dev.conv.ptr = memory;
    
    mco_db_params_init(&db_params);
    db_params.mem_page_size      = MEMORY_PAGE_SIZE;
    db_params.db_max_connections = 2;
#ifdef EXTREMEDB_LICENSE_KEY
    db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
    
    nvram = fopen("nvram.img", "rb");
    if (nvram != NULL) { 
        void * test_ptr;
        rc = fread(&test_ptr, 1, sizeof(test_ptr), nvram);
        if (rtinfo.direct_pointers_supported && memory != test_ptr ) {
            printf("Direct pointer configuration of the runtime required the SAME addresses for the database\n");
            printf("The database was created at base address 0x%x, current base address is 0x%x\n", (int)test_ptr, (int)memory );
            sample_os_shutdown();
            return 0;
        }
        rc = fread(memory, 1, DATABASE_SEGMENT_SIZE, nvram);
        if (rc != DATABASE_SEGMENT_SIZE) { 
            perror("Failed to read NVRAM image");
            sample_os_shutdown();
            return 1;
        }
        db_params.mode_mask |= MCO_DB_OPEN_EXISTING;
    }        
    CHECK(mco_db_open_dev(db_name, nvramdb_get_dictionary(), dev, 1, &db_params));
    CHECK(mco_db_connect(db_name, &db));  
    
    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));    
    if (Account_list_cursor(t, &c) == MCO_S_CURSOR_EMPTY) {     
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
        printf("Elapsed time for creating %d accounts: %d\n", n_accounts, (int)(time(0) - start));
        sample_pause("\n\nPress any key to continue . . .\n");
    }
    CHECK(mco_trans_commit(t));
    
    start = time(0);
    for (i = 0; i < n_iterations; i++) { 
        check_consistency(db, n_accounts);
        /* Use (Pi * key + e) % 1000000007L to generate a random key value */
        key = ((3141592621L * key + 2718281829L) % 1000000007L);
        src_account = key % n_accounts; 
        key = ((3141592621L * key + 2718281829L) % 1000000007L);
        dst_account = key % n_accounts;
        transfer_money(db, dst_account, src_account, nvram == NULL && i+1 == n_iterations);
    }    
    printf("Elapsed time for performing %d transactions: %d\n", n_iterations, (int)(time(0) - start));
    
    mco_db_disconnect(db);
    mco_db_close(db_name);
    mco_runtime_stop();
    free(memory);
    fclose(nvram);
    
    sample_pause_end("\n\nPress any key to continue . . . ");
    sample_os_shutdown();
    
    return 0;
}
