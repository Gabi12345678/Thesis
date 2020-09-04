#include "decls.h"

int print_sequence_of_float(Quote* quote, mco_seq_iterator_h iter)
{
    int i;
    float val;
    char symbol[MAX_SYMBOL_LEN];
    CHECK(Quote_symbol_get(quote, symbol, sizeof symbol));
    printf("%s: {", symbol);
    for (i = 0; mco_seq_next_float(iter, &val) == MCO_S_OK; i++) { 
        if (i != 0) printf(",");
        printf("%.3f", val);
    }
    printf("}\n");
    return i;
}

int print_sequence_of_double(Quote* quote, mco_seq_iterator_h iter)
{
    int i;
    double val;
    char symbol[MAX_SYMBOL_LEN];
    CHECK(Quote_symbol_get(quote, symbol, sizeof symbol));
    printf("%s: {", symbol);
    for (i = 0; mco_seq_next_double(iter, &val) == MCO_S_OK; i++) { 
        if (i != 0) printf(",");
        printf("%.3f", val);
    }
    printf("}\n");
    return i;
}

int print_sequence_of_int(Quote* quote, mco_seq_iterator_h iter)
{
    int i;
    uint4 val;
    char symbol[MAX_SYMBOL_LEN];
    CHECK(Quote_symbol_get(quote, symbol, sizeof symbol));
    printf("%s: {", symbol);
    for (i = 0; mco_seq_next_uint4(iter, &val) == MCO_S_OK; i++) { 
        if (i != 0) printf(",");
        printf("%u", val);
    }
    printf("}\n");
    return i;
}

int print_sequence_of_long(Quote* quote, mco_seq_iterator_h iter)
{
    int i;
    uint8 val;
    char symbol[MAX_SYMBOL_LEN];
    CHECK(Quote_symbol_get(quote, symbol, sizeof symbol));
    printf("%s: {", symbol);
    for (i = 0; mco_seq_next_uint8(iter, &val) == MCO_S_OK; i++) { 
        if (i != 0) printf(",");
        printf("%llu", val);
    }
    printf("}\n");
    return i;
}

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

    join(db);
    close_database(db);
    
    return 0;
}
