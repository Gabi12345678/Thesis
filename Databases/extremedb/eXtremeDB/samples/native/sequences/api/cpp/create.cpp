#include "decls.h"

const char* dbName = "stockdb";

#define DATABASE_SEGMENT_SIZE (64*1024*1024)
#define MEM_PAGE_SIZE 4096

#define SWAP(x,y) { double temp = x; x = y; y = temp; }

unsigned random_integer(unsigned mod)
{
    static mco_int8 seed = 2013;
    return (unsigned)(seed = (seed * 3141592621LL + 2718281829LL) % 1000000007LL) % mod;
}

double random_double() { 
    return (double)random_integer(10000) / 100;
}

struct Tick { 
    char symbol[MAX_SYMBOL_LEN];
    mco_date day;
    float low;
    float high;
    float open;
    float close;
    uint4 volume;
};

static void generate_random_quote(Tick& tick)
{
    static mco_date day = 0;
    sprintf(tick.symbol, "SYM%d", random_integer(N_SYMBOLS));
    tick.low  = random_double();
    tick.high = random_double();
    tick.open = random_double();
    tick.close = random_double();
    if (tick.high < tick.low) SWAP(tick.high, tick.low); 
    if (tick.high < tick.open) SWAP(tick.high, tick.open); 
    if (tick.high < tick.close) SWAP(tick.high, tick.close); 
    if (tick.low > tick.open) SWAP(tick.low, tick.open); 
    if (tick.low > tick.close) SWAP(tick.low, tick.close); 
    tick.volume = random_integer(1000);
    /* date is represeting using integer in YYYYMMDD format, for example 20130830 */
    tick.day = DMY(1 + day%30, 1 + day/30%12, 2013 + day/30/12);
    day += 1;
}

mco_db_h create_database()
{
    mco_db_h db;
    mco_device_t dev;
    mco_db_params_t db_params;

    CHECK(mco_runtime_start());
    mco_error_set_handler(sample_errhandler);
    mco_db_params_init(&db_params);

    dev.type       = MCO_MEMORY_CONV;
    dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;
    dev.size       = DATABASE_SEGMENT_SIZE;
    dev.dev.conv.ptr = malloc(DATABASE_SEGMENT_SIZE);
    db_params.mem_page_size = MEM_PAGE_SIZE;
    
    CHECK(mco_db_open_dev(dbName, stockdb_get_dictionary(), &dev, 1, &db_params));
    CHECK(mco_db_connect(dbName, &db));
    
    return db;
}

void populate_database(mco_db_h db) 
{ 
    mco_trans_h trans;
    Quote quote;
    Tick tick;
    MCO_RET rc;
    char buff[15];
    int i;

    for (i = 0; i < N_QUOTES; i++) { 
        CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trans));

        generate_random_quote(tick);
        rc = Quote::by_sym::find(trans, tick.symbol, strlen(tick.symbol), quote); 
        if (rc == MCO_S_NOTFOUND) { 
            CHECK(quote.create(trans));
            quote.symbol = tick.symbol;
        } else { 
            CHECK(rc);
        }
        CHECK(quote.day_append(&tick.day, 1));
        CHECK(quote.open_append(&tick.open, 1));
        CHECK(quote.close_append(&tick.close, 1));
        CHECK(quote.high_append(&tick.high, 1));
        CHECK(quote.low_append(&tick.low, 1));
        CHECK(quote.volume_append(&tick.volume, 1));
        sprintf(buff, "Day:%d", tick.day);
        CHECK(quote.day_str_append(buff, 1));

        CHECK(mco_trans_commit(trans));
    }
} 

void close_database(mco_db_h db)
{
    CHECK(mco_db_disconnect(db));
    CHECK(mco_db_close(dbName));
}
