#include "decls.h"

/**
 * Simple iteration through two related sequences.
 * Sequences belongs to the same timeseries, so then have same number of elements
 */
void iterate(mco_db_h db)
{
    mco_trans_h trans;
    mco_seq_iterator_t high_iterator, day_iterator, day_str_iterator;
    mco_date day;
    mco_cursor_t quote_cursor;
    Quote quote;
    float high;    
    MCO_RET rc;
    char buff[15];
    char symbol[MAX_SYMBOL_LEN];

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- ITERATE -------------------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);
 
        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));

        /* Get symbol name */
        CHECK(Quote_symbol_get(&quote, symbol, sizeof symbol));

        /* Initialize iterators */
        CHECK(Quote_high_iterator(&quote, &high_iterator));
        CHECK(Quote_day_iterator(&quote, &day_iterator));
        CHECK(Quote_day_str_iterator(&quote, &day_str_iterator));

        /* Execute query */
        while ((rc = mco_seq_next_uint4(&day_iterator, &day)) != MCO_S_CURSOR_END) { 
            CHECK(rc);
            CHECK(mco_seq_next_float(&high_iterator, &high));
            CHECK(mco_seq_next_char(&day_str_iterator, buff));
            printf("%s[%u(%s)]: %f\n", symbol, day, buff, high);
        }
    }    
    CHECK(mco_trans_commit(trans));
}
          
   
