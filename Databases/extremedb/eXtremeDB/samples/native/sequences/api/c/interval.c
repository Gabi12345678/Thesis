#include "decls.h"

/**
 * Extract data belonging to specified interval.
 * Ordered sequence (day) is used to select interval and then this interval is projects to other (unordered) sequences
 */
void interval(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    mco_seq_iterator_t open_iterator, close_iterator, day_iterator;
    Quote quote;
    mco_date day;
    float open, close;
    char symbol[MAX_SYMBOL_LEN];
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- INTERVAL ------------------\n");

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(Quote_from_cursor(trans, &quote_cursor, &quote));

        /* Get symbol name */
        CHECK(Quote_symbol_get(&quote, symbol, sizeof symbol));

        /* Select interval */
        CHECK((Quote_day_search(&quote, &day_iterator, DMY(1,1,2013), MCO_SEQ_BOUNDARY_INCLUSIVE, DMY(1,4,2013), MCO_SEQ_BOUNDARY_INCLUSIVE)));
        CHECK((Quote_open_project(&quote, &open_iterator, &day_iterator)));
        CHECK((Quote_close_project(&quote, &close_iterator, &day_iterator)));
    
        /* Execute query */
        while ((rc = mco_seq_next_uint4(&day_iterator, &day)) != MCO_S_CURSOR_END) { 
            CHECK(rc);
            CHECK(mco_seq_next_float(&open_iterator, &open));
            CHECK(mco_seq_next_float(&close_iterator, &close));
            printf("%s[%u]: %f..%f\n", symbol, day, open, close);
        }
    }
    CHECK(mco_trans_commit(trans));
}
          
   
