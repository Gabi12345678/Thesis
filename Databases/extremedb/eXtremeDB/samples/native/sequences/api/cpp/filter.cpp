#include "decls.h"

/**
 * Filter elements of sequence. Get days of last month when close > open.
 */
void filter(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_date last;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- FILTER --------------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        Sequence<uint4> day_iterator;
        CHECK(rc);

        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));

        /* Select interval */        
        CHECK(quote.day_last(&last));
        CHECK((quote.day_search(day_iterator, DMY(1, MONTH(last), YEAR(last)), MCO_SEQ_BOUNDARY_INCLUSIVE, last, MCO_SEQ_BOUNDARY_INCLUSIVE)));

        /* Execute query */
        print_sequence(quote, day_iterator.filter(quote.close_project(day_iterator) > quote.open_project(day_iterator)));
    }
    CHECK(mco_trans_commit(trans));
}
          
   
