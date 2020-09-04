#include <stdio.h>
#include <stdlib.h>
#include "mcoseq.h"
#include "stockdb.h"
#include "common.h"

#define N_QUOTES 1000
#define N_SYMBOLS  10
#define MAX_SYMBOL_LEN 21

extern int print_sequence_of_float(Quote* quote, mco_seq_iterator_h iter);
extern int print_sequence_of_double(Quote* quote, mco_seq_iterator_h iter);
extern int print_sequence_of_int(Quote* quote, mco_seq_iterator_h iter);
extern int print_sequence_of_long(Quote* quote, mco_seq_iterator_h iter);
extern mco_db_h create_database();
extern void populate_database(mco_db_h dba);

extern void iterate(mco_db_h db);
extern void interval(mco_db_h db);
extern void apply(mco_db_h db);
extern void filter(mco_db_h db);
extern void correlation(mco_db_h db);
extern void grand_aggregate(mco_db_h db);
extern void window_aggregate(mco_db_h db);
extern void grid_aggregate(mco_db_h db);
extern void group_by_aggregate(mco_db_h db);
extern void hash_aggregate(mco_db_h db);
extern void sort(mco_db_h db);
extern void top(mco_db_h db);
extern void VWAP(mco_db_h db);
extern void join(mco_db_h db);

extern void close_database(mco_db_h db);
 
