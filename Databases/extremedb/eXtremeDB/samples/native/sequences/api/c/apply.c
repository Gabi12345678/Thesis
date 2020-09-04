#include "decls.h"

/**
 * Evaluate expression for sequences. This example caclulates average of high and low prices.
 */
void apply(mco_db_h db)
{
    mco_trans_h trans;
    mco_seq_iterator_t high_iterator, low_iterator, add_iterator, div_iterator, const_iterator;
    mco_cursor_t quote_cursor;
    Quote quote;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- APPLY ---------------------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);
 
        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));
        
        /* Initialize iterators */
        CHECK(Quote_high_iterator(&quote, &high_iterator));
        CHECK(Quote_low_iterator(&quote, &low_iterator));
 
        /* Construct operator's pipeline */
        CHECK(mco_seq_add_float(&add_iterator, &low_iterator, &high_iterator));
        CHECK(mco_seq_const_float(&const_iterator, 2));
        CHECK(mco_seq_div_float(&div_iterator, &add_iterator, &const_iterator));

        /* Execute query */
        print_sequence_of_float(&quote, &div_iterator);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
