import exdb
import os
import sys
import traceback
import datetime
    
port = 5556

N_QUOTES = 10

schema = '''
#define int1      signed<1>
#define int2      signed<2>
#define int4      signed<4>
#define uint8   unsigned<8>
#define uint4   unsigned<4>
#define uint2   unsigned<2>
#define uint1   unsigned<1>

declare database quotes;

class Quote {
    uint8    ikey;
    char<8> symbol;
    time    stamp;
    float low;
    float high;
    float open;
    float close;
    uint4 volume;
    
    unique tree<ikey> iidx;
    unique tree<symbol> by_sym;
};
'''

def insert_quotes(conn):
    cursor = conn.cursor()
    sql = "INSERT INTO Quote(ikey, symbol, stamp, low, high, open, close, volume) VALUES (?,?,?,?,?,?,?,?)"

    params = []
    for i in xrange(N_QUOTES / 2, N_QUOTES):
        params.append((i, 'AA%s' % i, datetime.datetime(2017, 8, 16, 9, 30+i), 1.0, 4.0, 2.0, 3.0, i*1000))
    cursor.execute_many(sql, params)
        
    for i in xrange(0, N_QUOTES / 2):
        cursor.execute(sql, (i, 'AA%s' % i, datetime.datetime(2017, 8, 16, 9, 30+i), 1.0, 4.0, 2.0, 3.0, i*1000))
    cursor.close()
    print 'Insert done'
    return
    
def traverse_quotes(conn):
    print "Starting traverse quotes"
    cursor = conn.cursor()
    sql = "SELECT ikey, symbol, stamp, low, high, open, close, volume FROM Quote ORDER BY ikey"
    cursor.execute(sql)
        
    count = 0
    while True:
        row=cursor.fetchone()
        if row is None:
            break
        print "Row read:", row
        count +=  1
        
    # Do the same in other way   
    sql = "SELECT * FROM Quote ORDER BY ikey"
    cursor.execute(sql)
#        self.assertEqual(N_QUOTES, cursor.rowcount, "Invalid rowcount returned")

    count = 0        
    rows = cursor.fetchall()
    for row in rows:
        count +=  1                        

    # Test parameters
    sql = "SELECT * FROM Quote WHERE ikey > ? AND ikey <= ? ORDER BY ikey"
    cursor.execute(sql, (2, 7))
        
    cursor.arraysize = 2
        
    ret = []
    while True:
        rows = cursor.fetchmany()
        print "fetchmany returned ", rows
        if rows is None:
            break
            
#            self.assertEqual(2, len(rows), "Invalid return array returned")
        for row in rows:
            ret.append(row)
            
    return
    
def update_quotes(conn):
    print "Starting update quotes"
        
    sql = "UPDATE Quote SET high=?, volume=? WHERE ikey = ?"
    cursor = conn.cursor()
    cursor.execute(sql, (100, 110, 5))    
    
        # Check it
    cursor = conn.cursor()
    sql = "SELECT ikey, high, volume FROM Quote WHERE ikey = ?"
    cursor.execute(sql, (5,))
        
    row = cursor.fetchone()
    print row
    cursor.close()
    return
    
def delete_quotes(conn):
    print "Starting delete quotes"        
    cursor = conn.cursor()
    sql = "DELETE FROM Quote WHERE ikey = ?"
    cursor.execute(sql, (3,))
        
        # Check number of records and that only deleted is missing
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM Quote ORDER BY ikey")

    expected_ids = range(0, N_QUOTES)
    del expected_ids[3]
        
    rows = cursor.fetchall()        
    idx = 0
    for row in rows:
        idx += 1
        
    cursor.close()
        
    #delete all        
    cursor = conn.cursor()
    cursor.execute("DELETE FROM Quote")
    print 'cursor.rowcount = %s ' % cursor.rowcount
               
    cursor = conn.cursor()
    cursor.execute("SELECT COUNT(*) FROM Quote")
    result = cursor.fetchall()
        
    print "fetchall for 'SELECT COUNT(*) FROM Quote' returned %s" % result
        
    cursor.close()
    return
    
def run():

    is_disk = False
    tmgr = 'mursiw'
    is_shm = False
    is_debug = False
        
    #Load Runtime configuration specified by parameters
    exdb.init_runtime(is_disk, tmgr, is_shm, is_debug)            
        
    #Start SQL server
    dbdict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)        

    params = exdb.SqlOpenParameters(databaseName = "testdb", dictionary = dbdict, 
                                        mainMemoryDatabaseSize = 64*1024*1024, 
                                        mainMemoryPageSize = 128)

    if is_disk:
            params.diskDatabaseFile = "testdb.dbs"
            params.diskDatabaseLogFile = "testdb.log"
            params.diskCacheSize = 32 * 1024 * 1024
            
    engine = exdb.SqlEngine()
    engine.open(params)
    server = exdb.SqlServer(engine, port)
    server.start()
    
    try:
            conn = exdb.connect('localhost', port)   
            insert_quotes(conn)
            traverse_quotes(conn)
            update_quotes(conn)      
            delete_quotes(conn)      
            conn.close()
    except:
            traceback.print_exc(None, sys.stdout)
            print "Error happened: %s, %s, %s " % sys.exc_info() 
            sys.exit(1)                
    finally:
            server.stop()
    return

if __name__ == '__main__':
    run()
