import exdb
import array # Python's array.array which can be used for fast sequence append
import datetime
import traceback
import sys


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
'''

is_disk = False
tmgr = 'mursiw'
is_shm = False
is_debug = False
    
N_QUOTES       = 5000000
BUF_SIZE       = 1000

# Set borders for extract operations
FROM = 10
TILL = N_QUOTES - 10
    
def ASUM(x, y):
    return (x + y)*(y - x + 1)/2    
    
def timing(fn):
    def wrapped(conn):
        start  = datetime.datetime.now()
        ret = fn(conn)
        finish = datetime.datetime.now()
        dt = finish - start
        mcs  = dt.microseconds + (dt.seconds + dt.days * 24 * 3600)*10**6
        print "Function %s took %s ms" % (fn.__name__, mcs/1000)
        return ret
    return wrapped    

@timing
def insert_quotes_list(con):    
    con.startTransaction(exdb.Transaction.MCO_READ_WRITE)
    
#
#   To work with sequences we need 'connected' object, i.e. loaded from database or created with Connection.new() method.
#   Sequence iterators are already created as object fields and are in 'initial' state
#   

    quote = con.new("Quote")
    quote.symbol = 'AAA'

    op = array.array('f', [2.0] * BUF_SIZE)
    cl = array.array('f', [3.0] * BUF_SIZE)
    hg = array.array('f', [4.0] * BUF_SIZE)
    lw = array.array('f', [1.0] * BUF_SIZE)

    i = 0
        
    while i < N_QUOTES:
        quote.timestamp.append( [i+j+1 for j in xrange(BUF_SIZE)]) 
        quote.open.append(op)
        quote.close.append(cl)
        quote.high.append(hg)
        quote.low.append(lw)
        quote.volume.append( [i+j for j in xrange(BUF_SIZE)] )
        i += BUF_SIZE
    con.commit()

@timing
def insert_quotes_array(con):    
    con.startTransaction(exdb.Transaction.MCO_READ_WRITE)
    
#
#   To work with sequences we need 'connected' object, i.e. loaded from database or created with Connection.new() method.
#   Sequence iterators are already created as object fields and are in 'initial' state
#   
    quote = con.new("Quote")
    quote.symbol = 'AAA'

    i = 0
    
    # prepare buffers
    op = array.array('f', [2.0] * BUF_SIZE)
    cl = array.array('f', [3.0] * BUF_SIZE)
    hg = array.array('f', [4.0] * BUF_SIZE)
    lw = array.array('f', [1.0] * BUF_SIZE)

    while i < N_QUOTES:
        quote.timestamp.append(array.array('I', [i+j+1 for j in xrange(BUF_SIZE)]))
        quote.open.append(op)
        quote.close.append(cl)
        quote.high.append(hg)
        quote.low.append(lw)
        quote.volume.append( array.array('I', [i+j for j in xrange(BUF_SIZE)] ) )
        i += BUF_SIZE

        
# Need to save object to do actual store of all fields but sequences
    con.insert(quote)       
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
    
def calc(conn):
    conn.startTransaction()        
    cursor = conn.cursor()
    quote = cursor.find("Quote", "by_sym", "AAA")
           
    clthin = quote.close.thin (10, 1000)
        
    len = quote.timestamp.count()
    thninlen = clthin.agg_count().next()

    print 'thihlen = ', thninlen 

    cursor.close()
    conn.rollback()    


def unget():
    arrit = exdb.SequenceIterator.unget((1,2,3,4,5), exdb.Database.MCO_DB_FT_UINT4)    
    print "arrit = ", arrit
    for a in arrit:
        print " ", a

    
#Load Runtime configuration specified by parameters
exdb.init_runtime(is_disk, tmgr, is_shm, is_debug)            

unget()

# Parse schema, load dictionary and create python dictionary object
dbdict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)
    
db = exdb.open_database(dbname='opendb', dictionary=dbdict, is_disk=is_disk, db_segment_size=512*1024*1024);
    
print 'Database created'

#Connect to the database 
conn = None
try:
    with db.connect() as conn:
        print 'Database connected. Connection object is', conn
        insert_quotes_list(conn)
        traverse_quotes(conn)
        aggregate_quotes(conn)   
        calc(conn)

except:
    print 'Error happened:'
    traceback.print_exc()
    sys.exit(1)
finally:
    print 'Closing database'
    db.close()
    print 'Done'    
