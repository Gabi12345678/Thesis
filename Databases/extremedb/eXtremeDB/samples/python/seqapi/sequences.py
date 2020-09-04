import exdb

dbName = "stockdb"
is_debug = False

DATABASE_SEGMENT_SIZE = 64*1024*1024

N_QUOTES = 1000
N_SYMBOLS = 10

def print_sequence(quote, seqit):
    print "%s: {%s}" % (quote.symbol, ",".join(["%.3f" % s for s in seqit]))    

seed = 2013L

def log_rand():
    with open ("rand.log", "a") as rl:
        rl.write ("%s\n" % str(seed))

def random_integer(mod):
    global seed
    seed = (seed * 3141592621L + 2718281829L) % 1000000007L
    log_rand()
    return seed % mod
    
def random_double():
    return random_integer(10000) / 100.0

def DMY(day, month, year):    
      return year*10000 + month*100 + day

def DAY(dmy):
    return dmy % 100;
    
def MONTH(dmy):  
    return dmy/100%100

def YEAR(dmy):
    return dmy / 10000;

def create_database():
    
    #Load Runtime configuration specified by parameters
    exdb.init_runtime(False, 'mursiw', False, is_debug)            
    with open('schema.mco') as fp:
        schema = fp.read    ()

    # Parse schema, load dictionary and create python dictionary object
    dbdict = exdb.load_dictionary(schema, persistent=False, debug=is_debug)    
    db = exdb.open_database(dbname=dbName, dictionary=dbdict, is_disk=False, db_segment_size=DATABASE_SEGMENT_SIZE);
    conn = db.connect()    
    return db,conn
    
class Tick(object):
    pass

day = 0

def print_tick(tick):
    with open("tick.log", "a") as f:
        f.write("%s %s %s %s %s %s %s\n" % (tick.symbol, tick.day, tick.low, tick.high, tick.open, tick.close, tick.volume))
    
def generate_random_quote():
    global day
    tick = Tick()
    tick.symbol = "SYM%s" % random_integer(N_SYMBOLS)
    tick.low  = random_double()
    tick.high = random_double()
    tick.open = random_double()
    tick.close = random_double()

    if tick.high < tick.low:
        tmp = tick.high
        tick.high = tick.low
        tick.low = tmp
        
    if tick.high < tick.open:
        tick.high, tick.open = tick.open, tick.high
    
    if tick.high < tick.close:
        tick.high, tick.close = tick.close, tick.high

    if tick.low > tick.open:
        tick.low, tick.open = tick.open, tick.low

    if tick.low > tick.close:
        tick.low, tick.close = tick.close, tick.low
    
    tick.volume = random_integer(1000)
    tick.day = (1 + day%30) + 100*(1 + day/30%12) + 10000*(2013 + day/30/12);
    day += 1

    print_tick(tick)

    return tick        
    
def populate_database(con):
    for i in xrange(N_QUOTES):
        con.startTransaction()
        tick = generate_random_quote()
        
        cursor = con.cursor()
        quote = cursor.find("Quote", "by_sym", tick.symbol)
        if quote is None:
            quote = con.new("Quote")
            quote.symbol = tick.symbol
        
        quote.day.append(tick.day)
        quote.open.append(tick.open)
        quote.close.append(tick.close)
        quote.high.append(tick.high)
        quote.low.append(tick.low)
        quote.volume.append(tick.volume)
        cursor.close()
        con.commit()

def iterate(con):
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- ITERATE -------------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        # Bind iterators together
        quote.day.project('high')
        for (day,high) in quote.day:
            print "%s[%s]: %f" % (quote.symbol, day, high)
    cursor.close()
    con.rollback()


def interval(con):
    '''
    Extract data belonging to specified interval.
    Ordered sequence (day) is used to select interval and then this interval is projects to other (unordered) sequences
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- INTERVAL ------------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        # Bind iterators together
        dayit = quote.day.search(20130101, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_INCLUSIVE, 20130401, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_INCLUSIVE)
        
        dayit.project('open', 'close')
        for (day,open,close) in dayit:
            print "%s[%s]: %f..%f" % (quote.symbol, day, open, close)
    cursor.close()
    con.commit()

def apply(con):    
    '''
    Evaluate expression for sequences. This example caclulates average of high and low prices.
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- APPLY ---------------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        sum = quote.low.add(quote.high)
        it2 = exdb.SequenceIterator.const(2, exdb.Database.MCO_DB_FT_SEQUENCE_FLOAT)
        avgit = sum.div(it2)        
        print_sequence(quote, avgit)
    cursor.close()
    con.commit()
    
def filter(con):
    '''
    Filter elements of sequence. Get days of last month when close > open.
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- FILTER --------------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        # Select interval
        last = quote.day.last();
        dayit = quote.day.search(DMY(1,MONTH(last),YEAR(last)), exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_INCLUSIVE, last, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_INCLUSIVE)
        openit,closeit = dayit.project('open', 'close')

        #Construct operator's pipeline
        gtit = closeit.gt(openit)
        #        print "dayit=", [str(s) for s in dayit]
        filtit = gtit.filter(dayit)
        
        #Execute query
        print "%s: {%s}" % (quote.symbol, ",".join([str(s) for s in filtit]))    
    cursor.close()
    con.commit()

def correlation(con):  
    '''
    Find correlation between two sequences
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- CORRELATION ---------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        corrit = quote.low.corr(quote.high)
        print_sequence(quote, corrit)
    cursor.close()
    con.commit()
    
def grand_aggregate(con):    
    '''
    Calculate grand (sequnece-wide) aggregates. Get maximal close price for each symbol.
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- GRAND AGGREGATE -----------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        maxit = quote.close.agg_max()
        print_sequence(quote, maxit)
    con.commit()
    cursor.close()

def window_aggregate(con):    
    '''
    Calculate window (moving) aggregates. Find out dates the 20-day moving average crosses over the 5-days moving average.
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- WINDOW AGGREGATE ----------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        # Construct operator's pipeline
        cl1,cl2 = quote.close.tee() 
        avg5_it = cl1.window_agg_avg(5)
        avg20_it = cl2.window_agg_avg(20)

        sub_it = avg20_it.sub(avg5_it)
        cross_it = sub_it.cross(1)
        agg_it = quote.day.map(cross_it)

        # Execute query
        print "%s: {%s}" % (quote.symbol, ",".join([str(s) for s in agg_it]))
    cursor.close()
    con.commit()

def grid_aggregate(con):    
    '''
    Calculate grid aggregates. Determine maximal close price for each week
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- GRID AGGREGATE ------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        # Construct operator's pipeline
        maxit = quote.close.grid_agg_max(7)
        print_sequence(quote, maxit)
    cursor.close()
    con.commit()

    
def group_by_aggregate(con):
    '''
    Calculate aggregate for each group. A group is determined by ordered sequence elements with the same value.
    Find out total volume for each month of this year.
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- GROUP BY AGGREGATE --------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        # Construct operator's pipeline
        dayit = quote.day.search(20130101, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_INCLUSIVE, 20140101, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_EXCLUSIVE)  
        volit, = dayit.project('volume')

        const_it = exdb.SequenceIterator.const(100, exdb.Database.MCO_DB_FT_SEQUENCE_UINT4) # date is represetned as YYYYMMDD, so to group it by moth we should divide date by 100
        month_it = dayit.div(const_it)
        sum_it = volit.group_agg_sum(month_it)
        
        print "%s: {%s}" % (quote.symbol, ",".join([str(s) for s in sum_it]))
    cursor.close()
    con.commit()
    
    
def hash_aggregate(con):
    '''
    Calculate aggregate for each group. Groups are determined by arbitrary (not necessarily sorted) sequence 
    using hash table.
    Find out average price for volume groups 0..10, 11..20, 21..30...
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- HASH AGGREGATE ------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        
        # Construct operator's pipeline        
        const_it = exdb.SequenceIterator.const(10, exdb.Database.MCO_DB_FT_SEQUENCE_UINT4)
        vol_div_10_it = quote.volume.div(const_it)
        avg_it,group_it = quote.close.hash_agg_avg(vol_div_10_it, 0) 
        
        # Execute query
        print_sequence(quote, avg_it)
    cursor.close()
    con.commit()
        
    
def sort(con):
    '''
    Sort sequence elements. This example output close prices and volumes in volume descending order
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- SORT ----------------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        #Extract data into array
        
        volumes = quote.volume.get()
        prices = quote.close.get()
        
        permutation = quote.volume.sort(volumes, exdb.SortOrder.Descending)
        
        for j in permutation:
            print "%s: volume=%u, price=%.3f" % (quote.symbol, volumes[j], prices[j])
    cursor.close()
    con.commit()
    
def top(con):
    '''
    Get top-N sequence elements. This example calculates top-10 close proces and correspondent volumes
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- TOP -----------------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        #Get positions of top-10 highest prices
        top_it = quote.close.top_pos_max(10)
        cl_pos_it, vol_pos_it = top_it.tee()

        top_close_it = quote.close.map(cl_pos_it)
        top_vol_it = quote.volume.map(vol_pos_it)

        for close, vol in zip(top_close_it, top_vol_it):
            print "%s: price=%.3f, volume=%u" % (quote.symbol, close, vol)
    cursor.close()
    con.commit()

    
def VWAP(con):
    '''
    Volume weighted average prices for last week: vwap = sum(close*volume)/sum(volume)
    '''
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    print '--- VWAP ----------------------'
    # Iterate through all objects
    cursor = con.cursor("Quote", "by_sym")
    for quote in cursor:
        #Select interval
        last = quote.day.last()
        dayit = quote.day.search(last-7, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_EXCLUSIVE, last, exdb.SeqIteratorBoundary.MCO_SEQ_BOUNDARY_INCLUSIVE)
        closeit, volit1, volit2 = dayit.project('close', 'volume', 'volume')        
                
        #Construct operator's pipeline
        v1 = volit1.cast(exdb.Database.MCO_DB_FT_FLOAT)
        mul_it = closeit.mul(v1)
        sum1 = mul_it.agg_sum()
        sum2 = volit2.agg_sum()        
        #        print 'sum1 = ' , sum1, "data =",  sum1.get()
        div_it = sum1.div(sum2.cast(exdb.Database.MCO_DB_FT_DOUBLE))

        #        print "div = ", div_it, "data=", div_it.get()
        
        #Execute query
        print "%s: %f" % (quote.symbol, div_it.next())
    cursor.close()
    con.commit()
    
     
def close_database(db,con):
    con.close()
    db.close()
    
def main():
    db,con = create_database()
    populate_database(con)

    iterate(con)
    interval(con)
    apply(con)
    filter(con)
    correlation(con)
    grand_aggregate(con)
    window_aggregate(con)
    grid_aggregate(con)
    group_by_aggregate(con)
    hash_aggregate(con)
    sort(con)
    top(con)
    VWAP(con)

    close_database(db,con)

if __name__ == "__main__":
    main()
