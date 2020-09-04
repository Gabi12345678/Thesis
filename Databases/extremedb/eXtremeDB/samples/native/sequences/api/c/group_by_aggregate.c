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
    mco_seq_iterator_t volume_iterator, day_iterator, month_iterator, const_iterator, sum_iterator;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- GROUP BY AGGREGATE --------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));
          
        /* Select interval */
        CHECK((Quote_day_search(&quote, &day_iterator, DMY(1,1,2013), MCO_SEQ_BOUNDARY_INCLUSIVE, DMY(1,1,2014), MCO_SEQ_BOUNDARY_EXCLUSIVE)));
        CHECK((Quote_volume_project(&quote, &volume_iterator, &day_iterator)));

        /* Construct operator's pipeline */
        CHECK(mco_seq_const_uint4(&const_iterator, 100)); /* date is represetned as YYYYMMDD, so to group it by moth we should divide date by 100 */
        CHECK(mco_seq_div_uint4(&month_iterator, &day_iterator, &const_iterator));
        CHECK(mco_seq_group_agg_sum_uint4(&sum_iterator, &volume_iterator, &month_iterator));

        /* Execute query */
        print_sequence_of_long(&quote, &sum_iterator);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
