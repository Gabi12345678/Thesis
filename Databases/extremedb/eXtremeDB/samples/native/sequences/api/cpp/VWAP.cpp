#include "decls.h"

/**
 * Volume weighted average prices for last week: vwap = sum(close*volume)/sum(volume)
 */
void VWAP(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_date last;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- VWAP ----------------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        Sequence<mco_date> slice;
        CHECK(rc);

        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));
        
        /* Select interval */
        CHECK(quote.day_last(&last));
        CHECK((quote.day_search(slice, last-7, MCO_SEQ_BOUNDARY_EXCLUSIVE, last, MCO_SEQ_BOUNDARY_INCLUSIVE)));

        /* Execute query */
        Char<MAX_SYMBOL_LEN> symbol = quote.symbol;
        printf("%s: %f\n", (char*)symbol, ((Sequence<float>)quote.volume_project(slice)*quote.close_project(slice)).aggSum<double>()/quote.volume_project(slice).aggSum<uint8>());
    }
    CHECK(mco_trans_commit(trans));
}
          
   
