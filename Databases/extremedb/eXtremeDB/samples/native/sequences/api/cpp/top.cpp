#include "decls.h"

/**
 * Get top-N sequence elements. This example calculates top-10 close proces and correspondent volumes
 */
void top(mco_db_h db)
{
    mco_trans_h trans;
    mco_cursor_t quote_cursor;
    Quote quote;
    MCO_RET rc;
    float close;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- TOP -----------------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);
 
        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));
        
        /* Initialize iterators */
        Sequence<float> close_top_iterator = quote.close_iterator().topMax(10);
        Sequence<uint4> volume_top_iterator = quote.volume_map(quote.close_iterator().topPosMax(10));

        /* Execute query */
        while (close_top_iterator.next(close)) { 
            Char<MAX_SYMBOL_LEN> symbol = quote.symbol;
            printf("%s: price=%.3f, volume=%u\n", (char*)symbol, close, ++volume_top_iterator);
        }
    }
    CHECK(mco_trans_commit(trans));
}
          
   
