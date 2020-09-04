#include "decls.h"


/**
 * Calculate aggregate for each group. A group is determined by ordered sequence elements with the same value.
 * Find out total volume for each month of this year.
 */
void group_by_aggregate(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- GROUP BY AGGREGATE --------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        Sequence<uint4> day_iterator;
        CHECK(rc);

        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));
          
        /* Select interval */
        CHECK((quote.day_search(day_iterator, DMY(1,1,2013), MCO_SEQ_BOUNDARY_INCLUSIVE, DMY(1,1,2014), MCO_SEQ_BOUNDARY_EXCLUSIVE)));

        /* Execute query */
        print_sequence(quote, quote.volume_project(day_iterator).groupAggSum<uint8>(day_iterator/Sequence<uint4>(100)));
    }
    CHECK(mco_trans_commit(trans));
}
          
   
