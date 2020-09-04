#include "decls.h"

/**
 * Volume weighted average prices for last week: vwap = sum(close*volume)/sum(volume)
 */
void VWAP(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    mco_seq_iterator_t close_iterator, day_iterator, mul_iterator, div_iterator;
    mco_seq_iterator_t volume_iterator[2], cast_iterator[2], sum_iterator[2];
    mco_date last;
    char symbol[MAX_SYMBOL_LEN];
    double vwap;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- VWAP ----------------------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));

        /* Get symbol name */
        CHECK(Quote_symbol_get(&quote, symbol, sizeof symbol));
        
        /* Select interval */
        CHECK(Quote_day_last(&quote, &last));
        CHECK((Quote_day_search(&quote, &day_iterator, last-7, MCO_SEQ_BOUNDARY_EXCLUSIVE, last, MCO_SEQ_BOUNDARY_INCLUSIVE)));
        CHECK((Quote_close_project(&quote, &close_iterator, &day_iterator)));
        CHECK((Quote_volume_project(&quote, &volume_iterator[0], &day_iterator)));
        CHECK((Quote_volume_project(&quote, &volume_iterator[1], &day_iterator)));

        /* Construct operator's pipeline */
        CHECK(mco_seq_uint4_to_float(&cast_iterator[0], &volume_iterator[0]));
        CHECK(mco_seq_mul_float(&mul_iterator, &close_iterator, &cast_iterator[0]));
        CHECK(mco_seq_agg_sum_float(&sum_iterator[0], &mul_iterator));
        CHECK(mco_seq_agg_sum_uint4(&sum_iterator[1], &volume_iterator[1]));
        CHECK(mco_seq_uint8_to_double(&cast_iterator[1], &sum_iterator[1]));\
        CHECK(mco_seq_div_double(&div_iterator, &sum_iterator[0], &cast_iterator[1]));
        
        /* Execute query */
        CHECK(mco_seq_next_double(&div_iterator, &vwap));
        printf("%s: %f\n", symbol, vwap);
    }
    CHECK(mco_trans_commit(trans));
}
          
   
