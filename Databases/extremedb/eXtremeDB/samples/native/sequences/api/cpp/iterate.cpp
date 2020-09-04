#include "decls.h"

/**
 * Simple iteration through two related sequences.
 * Sequences belongs to the same timeseries, so then have same number of elements
 */
void iterate(mco_db_h db)
{
    mco_trans_h trans;
    mco_date day;
    mco_cursor_t quote_cursor;
    Quote quote;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    printf("--- ITERATE -------------------\n");

    /* Iterate through all objects */
    CHECK(Quote::by_sym::cursor(trans, &quote_cursor));
    for (rc = mco_cursor_first(trans, &quote_cursor); rc != MCO_S_CURSOR_END; rc = mco_cursor_next(trans, &quote_cursor)) { 
        CHECK(rc);
 
        /* Get current object */
        CHECK(quote.from_cursor(trans, &quote_cursor));

        /* Initialize iterators */
        Sequence<uint4> day_iterator = quote.day_iterator();
        Sequence<float> high_iterator = quote.high_iterator();
        Sequence< Char<15> > day_str_iterator = quote.day_str_iterator();

        /* Execute query */
        while (day_iterator.next(day)) {
            Char<MAX_SYMBOL_LEN> symbol = quote.symbol;
            Char<15> day_str = ++day_str_iterator;
            printf("%s[%u(%s)]: %f\n", (char*)symbol, day, (char*)day_str, ++high_iterator);
        }
    }
    CHECK(mco_trans_commit(trans));
}
