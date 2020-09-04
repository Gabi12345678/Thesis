#include "decls.h"

/**
 * Get top-N sequence elements. This example calculates top-10 close proces and correspondent volumes
 */
void top(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_seq_iterator_t close_iterator, close_pos_iterator, volume_pos_iterator;
    mco_seq_iterator_t top_iterator, volume_top_iterator, close_top_iterator;
    MCO_RET rc;
    uint4 volume;
    float close;
    char symbol[MAX_SYMBOL_LEN];

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- TOP -----------------------\n");

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
        
        /* Get positions of top-10 highest prices */
        CHECK(mco_seq_top_pos_max_float(&top_iterator, &close_iterator, 10));
        
        /* Fork top_iterator to be able to use it for two sequences */
        CHECK(mco_seq_tee(&close_pos_iterator, &volume_pos_iterator, &top_iterator));

        /* Get elements by top positions */
        CHECK(Quote_volume_map(&quote, &volume_top_iterator, &volume_pos_iterator));
        CHECK(Quote_close_map(&quote, &close_top_iterator, &close_pos_iterator));
        
        /* Execute query */
        while ((rc = mco_seq_next_float(&close_top_iterator, &close)) != MCO_S_CURSOR_END) { 
            CHECK(rc);
            CHECK(mco_seq_next_uint4(&volume_top_iterator, &volume));
            printf("%s: price=%.3f, volume=%u\n", symbol, close, volume);
        }
    }
    CHECK(mco_trans_commit(trans));
}
          
   
