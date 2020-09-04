#include <stdio.h>
#include <stdlib.h>
#include "stockdb.hpp"
#include "common.h"
#include <iostream>
#include <iomanip>
#include <vector>

#define N_QUOTES 1000
#define N_SYMBOLS  10

#define MAX_SYMBOL_LEN 21

using namespace std;
using namespace stockdb;

template<class T>
int print_sequence(Quote& quote, Sequence<T> iter)
{
    int i;
    T val;
    Char<MAX_SYMBOL_LEN> symbol = quote.symbol;
    cout << (char*)symbol << ": {";
    for (i = 0; iter.next(val); i++) { 
        if (i != 0) cout << ",";
        cout << fixed << std::setprecision(3) << val;
    }
    cout << "}\n";
    return i;
}
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

extern void close_database(mco_db_h db);
 
