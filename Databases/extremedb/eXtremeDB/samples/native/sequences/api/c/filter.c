#include "decls.h"

/**
 * Filter elements of sequence. Get days of last month when close > open.
 */
void filter(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_seq_iterator_t open_iterator, close_iterator, gt_iterator, day_iterator, filter_iterator;
    mco_date last;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- FILTER --------------------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));

        /* Select interval */        
        CHECK(Quote_day_last(&quote, &last));
        CHECK((Quote_day_search(&quote, &day_iterator, DMY(1, MONTH(last), YEAR(last)), MCO_SEQ_BOUNDARY_INCLUSIVE, last, MCO_SEQ_BOUNDARY_INCLUSIVE)));
        CHECK(Quote_open_project(&quote, &open_iterator, &day_iterator));
        CHECK(Quote_close_project(&quote, &close_iterator, &day_iterator));

        /* Construct operator's pipeline */
        CHECK(mco_seq_gt_float(&gt_iterator, &close_iterator, &open_iterator));
        CHECK(mco_seq_filter_uint4(&filter_iterator, &gt_iterator, &day_iterator));

        /* Execute query */
        print_sequence_of_int(&quote, &filter_iterator);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
