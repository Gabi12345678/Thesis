#include "decls.h"

/**
 * Calculate window (moving) aggregates. Find out dates the 20-day moving average crosses over the 5-days moving average.
 */
void window_aggregate(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_seq_iterator_t close_iterator[2], avg5_iterator, avg20_iterator, sub_iterator, day_iterator, cross_iterator;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- WINDOW AGGREGATE ----------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));
          
        /* Initialize iterators */
        CHECK(Quote_close_iterator(&quote, &close_iterator[0]));
        CHECK(Quote_close_iterator(&quote, &close_iterator[1]));

        /* Construct operator's pipeline */
        CHECK(mco_seq_window_agg_avg_float(&avg5_iterator, &close_iterator[0], 5));
        CHECK(mco_seq_window_agg_avg_float(&avg20_iterator, &close_iterator[1], 20));
        CHECK(mco_seq_sub_double(&sub_iterator, &avg20_iterator, &avg5_iterator));
        CHECK(mco_seq_cross_double(&cross_iterator, &sub_iterator, 1));
        CHECK(Quote_day_map(&quote, &day_iterator, &cross_iterator));
                
        /* Execute query */
        print_sequence_of_int(&quote, &day_iterator);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
