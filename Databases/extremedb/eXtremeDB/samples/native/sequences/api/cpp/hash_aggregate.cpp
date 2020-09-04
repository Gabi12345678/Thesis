#include "decls.h"


/**
 * Calculate aggregate for each group. Groups are determined by arbitrary (not necessarily sorted) sequence 
 * using hash table.
 * Find out average price for volume groups 0..10, 11..20, 21..30...
 */
void hash_aggregate(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- HASH AGGREGATE ------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        Sequence<double> result;
        Sequence<uint4> groups;
        CHECK(rc);

        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));
          
        /* Execute query */
        quote.close_iterator().hashAggAvg(result, groups, quote.volume_iterator()/Sequence<uint4>(10));
        print_sequence(quote, result);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
