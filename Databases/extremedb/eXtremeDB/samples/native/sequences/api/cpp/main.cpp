#include "decls.h"

int main(int argc, char* argv[])
{ 
    mco_db_h db;
    db = create_database();
    populate_database(db);
    iterate(db);
    interval(db);
    apply(db);
    filter(db);
    correlation(db);
    grand_aggregate(db);
    window_aggregate(db);
    grid_aggregate(db);
    group_by_aggregate(db);
    hash_aggregate(db);
    sort(db);
    top(db);
    VWAP(db);

    close_database(db);
    
    return 0;
}
