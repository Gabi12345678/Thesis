#include "decls.h"

/**
 * Sort sequence elements. This example output close prices and volumes in volume descending order
 */
void sort(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_seq_iterator_t close_iterator, volume_iterator;
    MCO_RET rc;
    mco_seq_no_t count;
    mco_size_t i, len;
    uint4* volumes;
    float* prices;
    mco_seq_no_t* permutation;
    char symbol[MAX_SYMBOL_LEN];

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- SORT ----------------------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);
 
        /* Get current object */ 
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));
       
        /* Get symbol name */
        CHECK(Quote_symbol_get(&quote, symbol, sizeof symbol));

        /* Initialize iterators */
        CHECK(Quote_close_iterator(&quote, &close_iterator));
        CHECK(Quote_volume_iterator(&quote, &volume_iterator));
        
        /* Extract data into array */
        CHECK(Quote_volume_count(&quote, &count)); /* get number of elements */
        len = (mco_size_t)count;
        volumes = (uint4*)malloc(len*sizeof(uint4));
        prices = (float*)malloc(len*sizeof(float));
        permutation = (mco_seq_no_t*)malloc(len*sizeof(mco_seq_no_t));
        CHECK(mco_seq_get_uint4(&volume_iterator, volumes, &len));
        CHECK(mco_seq_get_float(&close_iterator, prices, &len));

        /* Perform sort */
        CHECK(mco_seq_sort_uint4(volumes, permutation, len, MCO_SEQ_DESC_ORDER));
                                                      
        /* Print result */
        for (i = 0; i < len; i++) { 
            mco_size_t j = (mco_size_t)permutation[i];
            printf("%s: volume=%u, price=%.3f\n", symbol, volumes[j], prices[j]);
        }

        /* Free array's memory */
        free(prices);
        free(volumes);
        free(permutation);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
