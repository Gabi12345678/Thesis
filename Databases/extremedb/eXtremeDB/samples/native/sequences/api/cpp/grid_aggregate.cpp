#include "decls.h"

/**
 * Calculate grid aggregates. Determine maximal close proce for each week 
 */
void grid_aggregate(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- GRID AGGREGATE ------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));
          
        /* Execute query */
        print_sequence(quote, quote.close_iterator().gridAggMax(7));
    }
    CHECK(mco_trans_commit(trans));
}
          
   
