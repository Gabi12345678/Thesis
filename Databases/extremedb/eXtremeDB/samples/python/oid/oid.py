import exdb
import struct

is_disk = False
tmgr = 'mursiw'
is_shm = False
is_debug = False

#define eXtremeDB schema - exactly like in .mco file
schema = '''
#define uint2     unsigned<2>
#define uint4     unsigned<4>

declare database operations;

struct struct_oid {
    uint4 p1;
    uint2 p2;
};

declare oid struct_oid[10000];

class Record  {
    uint4 i4;
    string str;

    oid;
    unique tree<i4> by_i4;
};
'''

OBJECT_NUM = 5


#print out database content 
def print_database(conn, header):
    print header
    conn.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    cursor = conn.cursor("Record", "by_i4");
    for rec in cursor:
        print 'i4=%s,str="%s, oid=%s"' % (rec.i4, rec.str, struct.unpack("IH", rec.oid))
    cursor.close(); # close cursor
    conn.rollback(); # end transaction
    print ""
    
#Load Runtime configuration specified by parameters
exdb.init_runtime(is_disk, tmgr, is_shm, is_debug)

# Parse schema, load dictionary and create python dictionary object
dict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)
    
db = exdb.open_database(dbname='opendb', dictionary=dict, is_disk=is_disk, db_segment_size=128*1024*1024);
    
print 'Database created'
#Connect to the database 
conn = None
try:
    conn = db.connect()
    print 'Database connected. Connection object is', conn

    # 
    # Insert objects 
    # 
    i = 0
    while i < OBJECT_NUM:
        conn.startTransaction(exdb.Transaction.MCO_READ_WRITE) # start RW transaction
        rec = conn.new("Record", struct.pack("IH", 10, i)) # create python object

        # fill data
        rec.i4 = i;
        rec.str = "String %s" % i;
    
        conn.commit() # commit changes
        i += 1

    print_database(conn, "Database content after insert :")

    # 
    # Update
    # 
    conn.startTransaction(exdb.Transaction.MCO_READ_WRITE)

    # find record to update
    
    rec = conn.oidFind("Record", struct.pack("IH", 10, 3))
    # update object
    rec.str = "Updated string"
    conn.commit() # commit changes

    print_database(conn, "Database content after updating record with id == 2 :");

    # 
    # Delete
    # 
    conn.startTransaction(exdb.Transaction.MCO_READ_WRITE)

    # find and delete record with oid == (10,2)
    rec = conn.deleteObject(struct.pack("IH",10,2))
    conn.commit() # commit changes

    print_database(conn, "Database content after removing record with id == 3 :");

    print 'Closing connection'
    conn.close()
    conn = None
finally:
    if not conn is None:
        conn.close()            

print 'Closing database'
db.close()
print 'Done'    
