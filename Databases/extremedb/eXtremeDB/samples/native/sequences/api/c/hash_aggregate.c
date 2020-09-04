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
    mco_seq_iterator_t close_iterator, volume_iterator, volume_div_10_iterator, group_iterator, const_iterator, avg_iterator;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- HASH AGGREGATE ------------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));
          
        /* Initialize iterators */
        CHECK(Quote_close_iterator(&quote, &close_iterator));
        CHECK(Quote_volume_iterator(&quote, &volume_iterator));

        /* Construct operator's pipeline */
        CHECK(mco_seq_const_uint4(&const_iterator, 10)); 
        CHECK(mco_seq_div_uint4(&volume_div_10_iterator, &volume_iterator, &const_iterator));
        CHECK(mco_seq_hash_agg_avg_float(&avg_iterator, &group_iterator, &close_iterator, &volume_div_10_iterator, 0));

        /* Execute query */
        print_sequence_of_double(&quote, &avg_iterator);

        /* Free memory used by hash table */
        mco_seq_free_hash(&avg_iterator);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
