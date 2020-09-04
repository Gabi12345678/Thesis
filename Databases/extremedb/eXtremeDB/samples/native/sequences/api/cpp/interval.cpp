#include "decls.h"

/**
 * Extract data belonging to specified interval.
 * Ordered sequence (day) is used to select interval and then this interval is projects to other (unordered) sequences
 */
void interval(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_date day;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- INTERVAL ------------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        Sequence<uint4> day_iterator;
        CHECK(rc);

        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));

        /* Select interval */
        CHECK(quote.day_search(day_iterator, DMY(1,1,2013), MCO_SEQ_BOUNDARY_INCLUSIVE, DMY(1,4,2013), MCO_SEQ_BOUNDARY_INCLUSIVE));
        Sequence<float> open_iterator = quote.open_project(day_iterator);
        Sequence<float> close_iterator = quote.close_project(day_iterator);
    
        /* Execute query */
        while (day_iterator.next(day)) { 
            Char<MAX_SYMBOL_LEN> symbol = quote.symbol;
            printf("%s[%u]: %f..%f\n", (char*)symbol, day, ++open_iterator, ++close_iterator);
        }
    }
    CHECK(mco_trans_commit(trans));
}
          
   
