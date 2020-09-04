/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mco.h>
#include <common.h>
#include "general.h"

const char* sample_descr = "Sample 'general' demonstrates eXtremeDB performance for basic operations.\n";
const char* db_name = "general";

/* If you change the number of objects inserted, make sure that you
 * first have enough memory (DATABASE_SIZE), and also declare the hash table
 * size appropriately (hkey[estimated_number_of_entries] in general.mco
 */
#ifdef _INTEGRITY
const uint4 database_size = (2 * 1024 * 1024 * MEMORY_SIZE_X64_MULT + MEMORY_SIZE_WCHAR_ADD);
uint4 n_records = 3000;
#else
const uint4 database_size = (128 * 1024 * 1024 * MEMORY_SIZE_X64_MULT + MEMORY_SIZE_WCHAR_ADD);
uint4 n_records = 500000;
#endif

const uint4 n_traversals = 1;
const uint4 n_updates = 10;
const uint4 trans_size = 1000;
const uint4 random_seed = 1013;

#define STR_KEY_BUF_SIZE 64

#define ASSERT(cond) do { if (!(cond)) { printf("%s:%d: assertion %s failed\n", __FILE__, __LINE__, #cond); sample_os_shutdown(); exit(1); } } while (0);

typedef uint4(*sequencer_t)(uint4 val);

void insert(mco_db_h db, uint4 n_records, uint4 trans_size, uint4 init_value, sequencer_t sequencer)
{
    uint4 i;
    mco_trans_h t;
    FixedSizeRecord fix_rec;
    VaryingSizeRecord var_rec;
    time_t start = sample_msec();
    uint4 key = init_value;
    uint4 key_len;
    uint4 value_len;
    char key_buf[STR_KEY_BUF_SIZE];
    char value_buf[STR_KEY_BUF_SIZE];

    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
    for (i = 0; i < n_records; i++) 
    { 
        key = sequencer(key);
        key_len = sprintf(key_buf, "%010d", key);
        value_len = sprintf(value_buf, "%d", i);

        CHECK(FixedSizeRecord_new(t, &fix_rec));
        CHECK(FixedSizeRecord_key4_put(&fix_rec, key));
        CHECK(FixedSizeRecord_key8_put(&fix_rec, key));
        CHECK(FixedSizeRecord_key10_put(&fix_rec, key_buf, key_len));
        CHECK(FixedSizeRecord_value_put(&fix_rec, i));
        
        CHECK(VaryingSizeRecord_new(t, &var_rec));
        CHECK(VaryingSizeRecord_key_put(&var_rec, key_buf, key_len));
        CHECK(VaryingSizeRecord_value_put(&var_rec, value_buf, value_len));
        
        if ((i+1) % trans_size == 0) {
            CHECK(mco_trans_commit(t));
            CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
        }
    }
    CHECK(mco_trans_commit(t));
    printf("Elapsed time for insert %d records: %d milliseconds\n", n_records*2, (int)(sample_msec()- start));
}

    
void search(mco_db_h db, uint4 n_records, uint4 init_value, sequencer_t sequencer)
{
    uint4 i;
    mco_trans_h t;
    FixedSizeRecord fix_rec;
    VaryingSizeRecord var_rec;
    time_t start = sample_msec();
    uint4 key = init_value;
    uint4 value;
    uint4 key_len;
    uint2 str_len;
    char key_buf[STR_KEY_BUF_SIZE];
    char value_buf[STR_KEY_BUF_SIZE];
    mco_cursor_t c[4];
         
    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t));

    CHECK(FixedSizeRecord_tkey4_index_cursor(t, &c[0]));
    CHECK(FixedSizeRecord_tkey8_index_cursor(t, &c[1]));
    CHECK(FixedSizeRecord_tkey10_index_cursor(t, &c[2]));
    CHECK(VaryingSizeRecord_tkey_index_cursor(t, &c[3]));
    
    for (i = 0; i < n_records; i++)
    { 
        key = sequencer(key);
        key_len = sprintf(key_buf, "%010d", key);

        CHECK(FixedSizeRecord_hkey4_find(t, key, &fix_rec));
        CHECK(FixedSizeRecord_value_get(&fix_rec, &value));
        ASSERT(value == i);

        CHECK(FixedSizeRecord_hkey8_find(t, key, &fix_rec));
        CHECK(FixedSizeRecord_value_get(&fix_rec, &value));
        ASSERT(value == i);

        CHECK(FixedSizeRecord_hkey10_find(t, key_buf, key_len, &fix_rec));
        CHECK(FixedSizeRecord_value_get(&fix_rec, &value));
        ASSERT(value == i);

        CHECK(FixedSizeRecord_tkey4_search(t, &c[0], MCO_EQ, key)); 
        CHECK(FixedSizeRecord_from_cursor(t, &c[0], &fix_rec));
        CHECK(FixedSizeRecord_value_get(&fix_rec, &value));
        ASSERT(value == i);
       
        CHECK(FixedSizeRecord_tkey8_search(t, &c[1], MCO_EQ, key)); 
        CHECK(FixedSizeRecord_from_cursor(t, &c[1], &fix_rec));
        CHECK(FixedSizeRecord_value_get(&fix_rec, &value));
        ASSERT(value == i);
       
        CHECK(FixedSizeRecord_tkey10_search(t, &c[2], MCO_EQ, key_buf, key_len)); 
        CHECK(FixedSizeRecord_from_cursor(t, &c[2], &fix_rec));
        CHECK(FixedSizeRecord_value_get(&fix_rec, &value));
        ASSERT(value == i);
        
        CHECK(VaryingSizeRecord_hkey_find(t, key_buf, key_len, &var_rec));
        CHECK(VaryingSizeRecord_value_get(&var_rec, value_buf, STR_KEY_BUF_SIZE, &str_len));
        ASSERT((uint4)atoi(value_buf) == i);

        CHECK(VaryingSizeRecord_tkey_search(t, &c[3], MCO_EQ, key_buf, key_len)); 
        CHECK(VaryingSizeRecord_from_cursor(t, &c[3], &var_rec));
        CHECK(VaryingSizeRecord_value_get(&var_rec, value_buf, STR_KEY_BUF_SIZE, &str_len));
        ASSERT((uint4)atoi(value_buf) == i);
    }
    CHECK(mco_trans_commit(t));
    printf("Elapsed time for %d index searches: %d milliseconds\n", n_records*8, (int)(sample_msec()- start));
}
    

void scan(mco_db_h db, uint4 n_records, uint4 n_traversals)
{
    uint4 i, j, n;
    mco_trans_h t;
    FixedSizeRecord fix_rec;
    VaryingSizeRecord var_rec;
    time_t start = sample_msec();
    uint4 key;
    uint2 str_len;
    char key_buf[STR_KEY_BUF_SIZE];
    mco_cursor_t c[8];
    MCO_RET rc;
         
    for (i = 0; i < n_traversals; i++) 
    { 
        CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t));

        CHECK(FixedSizeRecord_tkey4_index_cursor(t, &c[0]));
        CHECK(FixedSizeRecord_hkey4_index_cursor(t, &c[1]));
        CHECK(FixedSizeRecord_tkey8_index_cursor(t, &c[2]));
        CHECK(FixedSizeRecord_hkey8_index_cursor(t, &c[3]));
        CHECK(FixedSizeRecord_tkey10_index_cursor(t, &c[4]));
        CHECK(FixedSizeRecord_hkey10_index_cursor(t, &c[5]));
        CHECK(VaryingSizeRecord_tkey_index_cursor(t, &c[6]));
        CHECK(VaryingSizeRecord_hkey_index_cursor(t, &c[7]));
        
        for (j = 0; j < 6; j++) 
        { 
            uint4 prev_key = 0;
            for (n = 0, rc = mco_cursor_first(t, &c[j]); rc == MCO_S_OK; rc = mco_cursor_next(t, &c[j]), n++)
            {
                CHECK(FixedSizeRecord_from_cursor(t, &c[j], &fix_rec));
                CHECK(FixedSizeRecord_key4_get(&fix_rec, &key));
                if ((j & 1) == 0) { /* tree index */
                    ASSERT(key > prev_key);
                    prev_key = key;
                }
            }
        }
        for (j = 6; j < 8; j++) 
        { 
            uint4 prev_key = 0;
            for (n = 0, rc = mco_cursor_first(t, &c[j]); rc == MCO_S_OK; rc = mco_cursor_next(t, &c[j]), n++)
            {
                CHECK(VaryingSizeRecord_from_cursor(t, &c[j], &var_rec));
                CHECK(VaryingSizeRecord_key_get(&var_rec, key_buf, STR_KEY_BUF_SIZE, &str_len));
                if ((j & 1) == 0) { /* tree index */
                    key = atoi(key_buf);
                    ASSERT(key > prev_key);
                    prev_key = key;
                }
            }
        }
        CHECK(mco_trans_commit(t));
    }
    printf("Elapsed time for %d table traversals: %d milliseconds\n", n_traversals*8, (int)(sample_msec()- start));
}
    
void update(mco_db_h db, uint4 n_records, uint4 trans_size, uint4 init_value, sequencer_t sequencer, uint4 n_updates)
{
    uint4 i, n;
    mco_trans_h t;
    FixedSizeRecord fix_rec;
    VaryingSizeRecord var_rec;
    time_t start = sample_msec();
    uint4 value;
    uint4 value_len;
    uint4 key_len;
    uint2 str_len;
    char key_buf[STR_KEY_BUF_SIZE];
    char value_buf[STR_KEY_BUF_SIZE];
         
    for (n = 0; n < n_updates; n++) 
    { 
        uint4 key = init_value;
        CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
   
        for (i = 0; i < n_records; i++)
        { 
            key = sequencer(key);
            key_len = sprintf(key_buf, "%010d", key);

            CHECK(FixedSizeRecord_hkey4_find(t, key, &fix_rec));            
            CHECK(FixedSizeRecord_value_get(&fix_rec, &value));
            ASSERT(value == i + n);
            CHECK(FixedSizeRecord_value_put(&fix_rec, value + 1));
            
            CHECK(VaryingSizeRecord_hkey_find(t, key_buf, key_len, &var_rec));
            CHECK(VaryingSizeRecord_value_get(&var_rec, value_buf, STR_KEY_BUF_SIZE, &str_len));
            if (n & 1) { 
                ASSERT(str_len == 0);
                value_len = sprintf(value_buf, "%d", i);
                CHECK(VaryingSizeRecord_value_put(&var_rec, value_buf, value_len));
            } else {
                ASSERT((uint4)atoi(value_buf) == i);
                CHECK(VaryingSizeRecord_value_put(&var_rec, 0, 0));
            }
                        
            if ((i+1) % trans_size == 0) {
                CHECK(mco_trans_commit(t));
                CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
            }
        }
        CHECK(mco_trans_commit(t));
    }

    printf("Elapsed time for %d updates: %d milliseconds\n", n_updates*n_records*2, (int)(sample_msec()- start));
}
    

void delete(mco_db_h db, uint4 n_records, uint4 trans_size, uint4 init_value, sequencer_t sequencer)
{
    uint4 i;
    mco_trans_h t;
    FixedSizeRecord fix_rec;
    VaryingSizeRecord var_rec;
    time_t start = sample_msec();
    uint4 key_len;
    char key_buf[STR_KEY_BUF_SIZE];
    uint4 key = init_value;
    
    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
   
    for (i = 0; i < n_records; i++)
    { 
        key = sequencer(key);
        key_len = sprintf(key_buf, "%010d", key);

        CHECK(FixedSizeRecord_hkey4_find(t, key, &fix_rec));            
        CHECK(FixedSizeRecord_delete(&fix_rec));

        CHECK(VaryingSizeRecord_hkey_find(t, key_buf, key_len, &var_rec));
        CHECK(VaryingSizeRecord_delete(&var_rec));
        
        if ((i+1) % trans_size == 0) {
            CHECK(mco_trans_commit(t));
            CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
        }        
    }
    CHECK(mco_trans_commit(t));

    printf("Elapsed time for removing %d records: %d milliseconds\n", n_records*2, (int)(sample_msec()- start));
}
    
    
uint4 random_sequence(uint4 key)
{
    return (3141592621u*key + 2718281829u) % 1000000007u;
}

/*
Fast generator for semi-random unique numbers. Uses global n_records variable
*/
uint4 pseudo_random_sequence(uint4 key)
{
    return (key + 90000) % n_records + 1;
}
    
uint4 monotone_sequence(uint4 key)
{
    return key + 1;
}

int main(int argc, char* argv[])
{
    MCO_RET rc;
    mco_db_h db;
    sample_memory_t db_mem;
    mco_runtime_info_t rt_info;

    sample_os_initialize(DEFAULT);

    sample_header(sample_descr);

    /* Set fatal error handler and start eXtremeDB runtime */
    mco_error_set_handler(&sample_errhandler);
    mco_runtime_start();

    mco_get_runtime_info(&rt_info);
    if (rt_info.disk_supported)
      n_records = 2000;

    /* Open databases */
    CHECK(sample_open_database(db_name, general_get_dictionary(), database_size, 
                               CACHE_SIZE, MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &db_mem));
    CHECK(mco_db_connect(db_name, &db));

    insert(db, n_records, 1, random_seed, pseudo_random_sequence);
    search(db, n_records, random_seed, pseudo_random_sequence);
    scan(db, n_records, n_traversals);
    update(db, n_records, 1, random_seed, pseudo_random_sequence, n_updates);
    delete(db, n_records, 1, random_seed, pseudo_random_sequence);

    insert(db, n_records, trans_size, 1, monotone_sequence);
    search(db, n_records, 1, monotone_sequence);
    scan(db, n_records, n_traversals);
    update(db, n_records, trans_size, 1, monotone_sequence, n_updates);
    delete(db, n_records, trans_size, 1, monotone_sequence);

    CHECK(mco_db_disconnect(db));
    sample_close_database(db_name, &db_mem);
    mco_runtime_stop();

    sample_pause_end("\n\nPress any key to continue . . . ");
    sample_os_shutdown();

    return 0;
}
