#include "decls.h"

/**
 * Find correlation between two sequences
 */
void correlation(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- CORRELATION ---------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);

        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));
          
        /* Execute query */
        Char<MAX_SYMBOL_LEN> symbol = quote.symbol;
        printf("%s: {%.3f}\n", (char*)symbol, quote.low_iterator().corr(quote.high_iterator()));
    }
    CHECK(mco_trans_commit(trans));
}
          
   
