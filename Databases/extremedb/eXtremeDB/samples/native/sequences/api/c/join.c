
#include "decls.h"

typedef struct quote_low_dsc_t
{
	mco_trans_h trans;
	mco_cursor_t quote_cursor;
	Quote quote;
} quote_low_dsc;


MCO_RET join_low_next(mco_seq_meta_iterator_h meta_iterator)
{
	quote_low_dsc *ud = (quote_low_dsc *)meta_iterator->user_data;

 	MCO_RET rc = mco_cursor_next(ud->trans, &ud->quote_cursor);
 	if (rc != MCO_S_OK)
 		return rc;

 	CHECK(Quote_from_cursor(ud->trans, &ud->quote_cursor, &ud->quote));

 	CHECK(Quote_low_iterator(&ud->quote, &meta_iterator->iterator));
 	return MCO_S_OK;
}

MCO_RET join_low_first(mco_seq_meta_iterator_h meta_iterator)
{
	quote_low_dsc *ud = (quote_low_dsc *)meta_iterator->user_data;

 	MCO_RET rc = mco_cursor_first(ud->trans, &ud->quote_cursor);

 	if (rc != MCO_S_OK)
 		return rc;
 	CHECK(Quote_from_cursor(ud->trans, &ud->quote_cursor, &ud->quote));

 	CHECK(Quote_low_iterator(&ud->quote, &meta_iterator->iterator));
 	return MCO_S_OK;
}

/**
 * Filter elements of sequence. Get days of last month when close > open.
 */
void join(mco_db_h db)
{
    mco_seq_iterator_t all_low_it;
    mco_seq_iterator_t min_low_it;
    mco_seq_iterator_t count_low_it; 
    mco_seq_meta_iterator_t low_meta_it;

    mco_date last;
    MCO_RET rc;
    float min_low;
    uint8 count;

    quote_low_dsc user_data;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &user_data.trans));
    printf("--- JOIN --------------------\n");


    low_meta_it.next = join_low_next;
    low_meta_it.first = join_low_first;
    low_meta_it.user_data = &user_data;

    /* Iterate through all objects */
    CHECK(Quote_by_sym_index_cursor(user_data.trans, &user_data.quote_cursor));

    CHECK(mco_seq_join_float(&all_low_it, &low_meta_it));

    /* now all_low_it - sequence which consists of all sequences low for all object Quote which we selected */
	
    CHECK(mco_seq_agg_max_float(&min_low_it, &all_low_it));
    CHECK(mco_seq_next_float(&min_low_it, &min_low));
    
    printf("Maximum of lows: %g\n", min_low);
        
    CHECK(mco_trans_commit(user_data.trans));
}
          
   
