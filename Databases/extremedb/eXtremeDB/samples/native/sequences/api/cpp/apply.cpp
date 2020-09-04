#include "decls.h"

/**
 * Evaluate expression for sequences. This example caclulates average of high and low prices.
 */
void apply(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- APPLY ---------------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);
 
        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));
        
        /* Execute query */
        print_sequence(quote, (quote.high_iterator() + quote.low_iterator())/2.0f);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
