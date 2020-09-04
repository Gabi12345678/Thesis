/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <ctype.h>
#include <common.h>
#include "searchdb.h"

char sample_descr[] = { "Sample 'search' demonstrates full text search operations with tree indexes\n" };
const char * db_name = "search";

char const* values[] = { 
    "one two three one",
    "abc xyz cba zyx",
    "one more abc",
    "two three",
    "xyz",
    "cba abc one"
};

void split_in_words(anObject* obj, char const* text)
{
    char const* p = text;
    char const* q = text;
    int n_words = 0;
    while (*p != '\0') { 
        if (isspace(*p & 0xFF)) {
            if (p != q) { 
                n_words += 1;
            }
            q = p += 1;
        } else { 
            p += 1;
        }
    }
    n_words += p != q;

    p = q = text;
    anObject_keywords_alloc(obj, n_words);
    n_words = 0;
    while (*p != '\0') { 
        if (isspace(*p & 0xFF)) {
            if (p != q) { 
                anObject_keywords_put(obj, n_words++, q, p-q);
            }
            q = p += 1;
        } else { 
            p += 1;
        }
    }
    if (p != q) { 
        anObject_keywords_put(obj, n_words, q, p-q);
    }
}        


int main(int argc, char* argv[])
{

  MCO_RET rc;
  mco_db_h db = 0;
  mco_trans_h t;
  sample_memory_t dbmem;
  unsigned int i;
  mco_cursor_t csr;
  anObject obj;
  char text[64];
  uint2 len;
  uint4 id;
  mco_search_kwd_t query[2];

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, searchdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Fill database */
      for (i = 0; i < sizeof(values) / sizeof(values[0]) && MCO_S_OK == rc; ++i) {
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc )  {
          anObject_new(t, &obj);
          anObject_id_put(&obj, i);
          anObject_text_put(&obj, values[i], strlen(values[i]));
          split_in_words(&obj, values[i]);
          rc = mco_trans_commit(t);
        }
        if ( MCO_S_OK != rc ) {
          printf("\tUnable insert object #%d: %s\n", i, mco_ret_string(rc, 0));
        }
      }

      query[0].kwd_ptr = "abc";
      query[0].kwd_val.ptr_size = 3;
      query[1].kwd_ptr = "one";
      query[1].kwd_val.ptr_size = 3;            

      /* Perform AND full text search */
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc )  {
        rc = anObject_inverse_index_cursor(t, &csr);    
        if ( MCO_S_OK == rc ) {
          printf("\nQuery: abc AND one\n");
          for (rc = anObject_inverse_search_all(t, &csr, 2, query); MCO_S_OK == rc; rc = mco_cursor_next(t, &csr)) {
            anObject_from_cursor(t, &csr, &obj);
            anObject_text_get(&obj, text, sizeof text, &len);
            anObject_id_get(&obj, &id);
            printf("%d: %s\n", id, text);
          }
          printf("------------------------\n");
        }    
        mco_trans_rollback(t);
      }

      /* Perform OR full text search */
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc )  {
        rc = anObject_inverse_index_cursor(t, &csr);    
        if ( MCO_S_OK == rc ) {
          printf("\nQuery: abc OR one\n");
          for (rc = anObject_inverse_search_any(t, &csr, 2, query); MCO_S_OK == rc; rc = mco_cursor_next(t, &csr)) {
            anObject_from_cursor(t, &csr, &obj);
            anObject_text_get(&obj, text, sizeof text, &len);
            anObject_id_get(&obj, &id);
            printf("%d: %s\n", id, text);
          }
          printf("------------------------\n");
        }    
        mco_trans_rollback(t);
      }
      sample_close_database(db_name, &dbmem);
    }
  }

  mco_runtime_stop();

  sample_pause_end("\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc || MCO_S_CURSOR_END == rc ? 0 : 1 );
}
