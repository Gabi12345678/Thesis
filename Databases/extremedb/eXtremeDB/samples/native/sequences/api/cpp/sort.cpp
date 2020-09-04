#include "decls.h"

/**
 * Sort sequence elements. This example output close prices and volumes in volume descending order
 */
void sort(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- SORT ----------------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        vector<uint4> volumes;
        vector<float> prices;
        vector<mco_seq_no_t> permutation;

        CHECK(rc);
 
        /* Get current object */ 
        CHECK(quote.from_cursor(trans, &quote_cursor));
        
        /* Extract data into array */
        quote.volume_iterator().get(volumes);
        quote.close_iterator().get(prices);
        /* Perform sort */
        Sequence<uint4>::sort(volumes, permutation, MCO_SEQ_DESC_ORDER);
                                                      
        /* Print result */
        for (size_t i = 0; i < permutation.size(); i++) { 
            size_t j = (size_t)permutation[i];
            Char<MAX_SYMBOL_LEN> symbol = quote.symbol;
            printf("%s: volume=%u, price=%.3f\n", (char*)symbol, volumes[j], prices[j]);
        }
    }
    CHECK(mco_trans_commit(trans));
}
          
   
