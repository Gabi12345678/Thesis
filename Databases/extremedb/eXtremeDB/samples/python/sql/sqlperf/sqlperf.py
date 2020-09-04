'''
 Sample 'Perf' demonstrates eXtremeDB Python performance for basic operations.
 First N objects are inserted into a class, a hash index is created
 on insertion, then used for searches.  Then a tree index is built
 and searches are performed by key then sequentially.
 Finally, the tree index is removed and all objects are deleted
 one-by-one.  Each insert and delete is done in a separate
 transaction, so that the commit time is included in measurements.
'''

PAGE_SIZE = 128
DISK_PAGE_SIZE = 4096
DISK_CACHE_SIZE = 8*1024*1024
DATABASE_SIZE = 32*1024*1024

N_RECORDS = 10000

dbname = "perfdb"
is_disk = False
is_debug = False
is_shm = False
tmgr   = 'mursiw'

import sys
import exdb
import datetime

class Perf(object):
    def insert(self, con):
        for i in xrange(N_RECORDS):
            con.startTransaction()
            c = con.cursor()
            c.execute("INSERT INTO Record(hkey,tkey) VALUES (?,?)", (i*2, i*2))
            c.close()
            con.commit()

    def hash_search(self, con):
        con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
        hc = con.cursor()
        for i in xrange(N_RECORDS*2):
            hc.execute("SELECT hkey,tkey FROM Record WHERE hkey = ?", (i,))
            rec = hc.fetchone()
            if i % 2 == 0:
                assert not rec is None and rec[0] == i and rec[1] == i
            else:
                assert rec is None
        hc.close()
        con.commit();

    def tree_search(self, con):
        con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
        tc = con.cursor()
        for i in xrange(N_RECORDS*2):
            tc.execute("SELECT hkey,tkey FROM Record WHERE tkey = ?", (i,))
            rec = tc.fetchone()
            if i % 2 == 0:
                assert not rec is None and rec[0] == i and rec[1] == i
            else:
                assert rec is None
        tc.close()
        con.commit();

    def traverse(self, con):
        con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
        c = con.cursor()
        c.execute("SELECT hkey, tkey FROM Record ORDER BY hkey")
        i = 0
        for rec in c:
            assert rec[0] == i*2 and rec[1] == i*2
            i += 1;
        assert i == N_RECORDS
        c.close()
        con.commit()

    def delete(self, con):
        for i in xrange(0, N_RECORDS*2, 2):
            con.startTransaction()
            c = con.cursor()
            c.execute("DELETE FROM Record WHERE hkey=?", (i,))
            c.close()
            con.commit()


class ExtractPerf(Perf):
    def hash_search(self, con):
        con.startTransaction(exdb.Transaction.MCO_READ_ONLY);
        hc = con.cursor()
        buf = " " *8
        for i in xrange(N_RECORDS*2):
            hc.execute("SELECT hkey,tkey FROM Record WHERE hkey = ?", (i,))
            rec = hc.extractone("ii", buf)
            if i % 2 == 0:
                assert not rec is None and rec[0] == i and rec[1] == i
            else:
                assert rec is None
        hc.close()
        con.commit();

    def tree_search(self, con):
        con.startTransaction(exdb.Transaction.MCO_READ_ONLY);
        tc = con.cursor()
        buf = " " *8
        for i in xrange(N_RECORDS*2):
            tc.execute("SELECT hkey,tkey FROM Record WHERE tkey = ?", (i,))
            rec = tc.extractone("ii",buf)
            if i % 2 == 0:
                assert not rec is None and rec[0] == i and rec[1] == i
            else:
                assert rec is None
        tc.close()
        con.commit();

    def traverse(self, con):
        con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
        c = con.cursor()
        c.execute("SELECT hkey, tkey FROM Record ORDER BY hkey")
        i = 0
        buf = " " *8
        rec = c.extractone("ii", buf)
        while not rec is None:
            assert rec[0] == i*2 and rec[1] == i*2
            i += 1
            rec = c.extractone("ii", buf)
        assert i == N_RECORDS
        c.close()
        con.commit()


def main():
    global is_disk
    ins = Perf()

    for s in sys.argv:
        if s == 'disk':
            is_disk = True
        elif s == 'extract':
            ins = ExtractPerf()

    exdb.init_runtime(disk=is_disk, tmgr=tmgr, shm=is_shm, debug=is_debug)
    with open("perf.mco") as sp:
        dict = exdb.load_dictionary(sp.read(), persistent=is_disk, debug=is_debug)

    db = exdb.open_database(dbname, dictionary=dict, is_disk=is_disk,
                            db_segment_size=DATABASE_SIZE, cache_segment_size=DISK_CACHE_SIZE,
                            mem_page_size=PAGE_SIZE, disk_page_size=DISK_PAGE_SIZE)


    con = exdb.connect(dbname)

    if is_disk:
        con.setCommitPolicy(exdb.Database.CommitPolicy.NoSync);
    '''
        ////////////////////////////////////////////////////// 
        // Insert some data in the database
        //////////////////////////////////////////////////////
    '''

    start = datetime.datetime.now()

    ins.insert(con)

    dt = datetime.datetime.now()-start
    ms = dt.total_seconds() * 1000 + dt.microseconds/1000
    print "Elapsed time for inserting %s records: %s milliseconds" % (N_RECORDS,ms);

    '''
        ////////////////////////////////////////////////////// 
        // Hash search
        //////////////////////////////////////////////////////
    '''
    start = datetime.datetime.now()
    ins.hash_search(con)
    dt = datetime.datetime.now()-start
    ms = dt.total_seconds() * 1000 + dt.microseconds/1000
    print "Elapsed time for %s hash searches: %s milliseconds" % (N_RECORDS*2, ms);

    '''
        //////////////////////////////////////////////////////
        // Tree search
        //////////////////////////////////////////////////////
    '''
    start = datetime.datetime.now()
    ins.tree_search(con)
    dt = datetime.datetime.now()-start
    ms = dt.total_seconds() * 1000 + dt.microseconds/1000
    print "Elapsed time for %s tree searches: %s milliseconds" % (N_RECORDS*2, ms)

    '''
        ////////////////////////////////////////////////////// 
        // cursor movements 
        //////////////////////////////////////////////////////
    '''
    start = datetime.datetime.now()
    ins.traverse(con)
    dt = datetime.datetime.now()-start
    ms = dt.total_seconds() * 1000 + dt.microseconds/1000
    print "Elapsed time for traversal %s records: %s milliseconds" % (N_RECORDS*2, ms)


    '''
         ////////////////////////////////////////////////////// 
        // Search & Delete operation
        ////////////////////////////////////////////////////// 
    '''
    start = datetime.datetime.now()
    ins.delete(con)
    dt = datetime.datetime.now()-start
    ms = dt.total_seconds() * 1000 + dt.microseconds/1000
    print "Elapsed time for removing %s records: %s milliseconds" % (N_RECORDS*2, ms)

    con.close()
    db.close()

if __name__ == "__main__":
    main()
