import exdb

#define eXtremeDB schema - exactly like in .mco file
schema = '''
/*****************************************************************
 *                                                               *
 * Copyright(c) 2001 - 2003 McObject,LLC. All Right Reserved.    *
 *                                                               *
 *****************************************************************/

#define int1      signed<1>
#define int2      signed<2>
#define int4      signed<4>
#define uint8   unsigned<8>
#define uint4   unsigned<4>
#define uint2   unsigned<2>
#define uint1   unsigned<1>

declare database timeseries;

class Quote {
    char<8> symbol;
    sequence<time asc> timestamp;
    sequence<float> low;
    sequence<float> high;
    sequence<float> open;
    sequence<float> close;
    sequence<uint4> volume;
    hash<symbol> by_sym[1000];
};

class temp_float_iters {
    sequence<float> it;
};

class temp_time_iters {
    sequence<time> it;
};

class temp_uint4_iters {
    sequence<uint4> it;
};
'''

is_disk = False
tmgr = 'mursiw'
is_shm = False
is_debug = False
    
N_QUOTES       = 5000000
BUF_SIZE       = 100

# Set borders for extract operations
FROM = 10
TILL = N_QUOTES - 10
    
def ASUM(x, y):
    return (x + y)*(y - x + 1)/2    
    
def insert_quotes(con):    
    con.startTransaction(exdb.Transaction.MCO_READ_WRITE)
    
#
#   To work with sequences we need 'connected' object, i.e. loaded from database or created with Connection.new() method.
#   Sequence iterators are already created as object fields and are in 'initial' state
#   
    quote = con.new("Quote")
    quote.symbol = 'AAA'

    i = 0
        
    while i < N_QUOTES:
        j = 0
        # Compose lists as appropriate python type for storing sets of values to insert chunks into sequences
        timestamps = []
        opens = []
        closes = []
        highs = []
        lows = []
        volumes = []
        while i < N_QUOTES and j < BUF_SIZE:
            timestamps.append(i+1)
            opens.append(2.0)
            closes.append(3.0)
            highs.append(4.0)
            lows.append(1.0)
            volumes.append(i)
            j += 1
            i += 1

        # Chunks are ready, append them to iterators

        quote.timestamp.append(timestamps)
        quote.open.append(opens)
        quote.close.append(closes)
        quote.high.append(highs)
        quote.low.append(lows)
        quote.volume.append(volumes)

# Need to save object to do actual store
    con.commit()
    
def traverse_quotes(conn):
    
    timestamp_sum = 0.0
    open_sum      = 0.0
    close_sum     = 0.0
    high_sum      = 0.0
    low_sum       = 0.0
    volume_sum    = 0.0
        
    conn.startTransaction()
    
    # Find the record in the database
    cursor = conn.cursor()
    quote = cursor.find("Quote", "by_sym", "AAA")
    
    print "Found quote:", quote
    
    # Perform search operation on one of sequences in class
    tsit = quote.timestamp.search(FROM, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_EXCLUSIVE, TILL, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_INCLUSIVE)        

    # Project search result to other sequences in the class. That sequences becomes 'dependent' and iteration over 'main' will result in getting
    # tuple for all dependent sequences
    tsit.project('open', 'close', 'high', 'low', 'volume')

    count = 0        
    # Iterate over a set of values
    for (ts, open, close, high, low, volume) in tsit:
        timestamp_sum += ts
        open_sum += open
        close_sum += close
        high_sum += high
        low_sum += low
        volume_sum += volume
        count += 1
            
    if timestamp_sum != ASUM(FROM+1, TILL) or open_sum != (TILL-FROM)*2.0 or close_sum != (TILL-FROM)*3.0 or high_sum != (TILL-FROM)*4.0 or low_sum != (TILL-FROM)*1.0 or volume_sum != ASUM(FROM, TILL-1):            
        raise Exception ("Incorrect results of traversal")

    cursor.close()
    conn.rollback()
    return

# Show some sequence aggregates 
def aggregate_quotes(conn):
    
    conn.startTransaction()        
    cursor = conn.cursor()
    quote = cursor.find("Quote", "by_sym", "AAA")
           
    quote.timestamp.search(FROM, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_EXCLUSIVE, TILL, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_INCLUSIVE)        
    quote.timestamp.project('open', 'close', 'high', 'low', 'volume')

    print "Getting aggregates..."        
    tsum = quote.timestamp.agg_sum().next()
    osum = quote.open.agg_sum().next()
    clsum = quote.close.agg_sum().next()
    hsum = quote.high.agg_sum().next()
    lsum = quote.low.agg_sum().next()
    vsum = quote.volume.agg_sum().next()

    print "Aggregates are:", tsum, osum, clsum, hsum, lsum, vsum

    cursor.close()
    conn.rollback()
    return

    
#Load Runtime configuration specified by parameters
exdb.init_runtime(is_disk, tmgr, is_shm, is_debug)            

# Parse schema, load dictionary and create python dictionary object
dict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)
    
db = exdb.open_database(dbname='opendb', dictionary=dict, is_disk=is_disk, db_segment_size=512*1024*1024);
    
print 'Database created'

#Connect to the database 
conn = None
try:
    conn = db.connect()
    print 'Database connected. Connection object is', conn

    conn.startTransaction()
    uit = conn.new('temp_uint4_iters')
    uit.it.append([1,2,3,4,5,6])
        
    print 'iterator created'
    
    uit.it.reset()
    print 'Reset done. Iterator=', uit.it
    
    for i in uit.it:
        print i
    conn.rollback()

    insert_quotes(conn)
    traverse_quotes(conn)
    aggregate_quotes(conn)   
    
    print 'Closing connection'
    conn.close()
    conn = None
finally:
    if not conn is None:
        conn.close()            

print 'Closing database'
db.close()
print 'Done'    
