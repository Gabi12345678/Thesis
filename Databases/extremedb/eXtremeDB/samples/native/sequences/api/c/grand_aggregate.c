#include "decls.h"

/**
 * Calculate grand (sequnece-wide) aggregates. Get maximal close price for each symbol.
 */
void grand_aggregate(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_seq_iterator_t close_iterator, max_iterator;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- GRAND AGGREGATE -----------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));
          
        /* Initialize iterators */
        CHECK(Quote_close_iterator(&quote, &close_iterator));

        /* Construct operator's pipeline */
        CHECK(mco_seq_agg_max_float(&max_iterator, &close_iterator));

        /* Execute query */
        print_sequence_of_float(&quote, &max_iterator);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
