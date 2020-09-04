#include "decls.h"

/**
 * Find correlation between two sequences
 */
void correlation(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_seq_iterator_t high_iterator, low_iterator, corr_iterator;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- CORRELATION ---------------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));
          
        /* Initialize iterators */
        CHECK(Quote_high_iterator(&quote, &high_iterator));
        CHECK(Quote_low_iterator(&quote, &low_iterator));


        /* Construct operator's pipeline */
        CHECK(mco_seq_corr_float(&corr_iterator, &low_iterator, &high_iterator));

        /* Execute query */
        print_sequence_of_double(&quote, &corr_iterator);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
