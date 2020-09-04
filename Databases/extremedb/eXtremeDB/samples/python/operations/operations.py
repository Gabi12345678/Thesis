import exdb

#define eXtremeDB schema - exactly like in .mco file
schema = '''
#define uint4     unsigned<4>

declare database operations;

class Record  {
    uint4 i4;
    string str;

    unique tree<i4> by_i4;
};
'''

OBJECT_NUM = 5

is_disk = False
tmgr = 'mursiw'
is_shm = False
is_debug = False

#print out database content 
def print_database(conn, header):
    print header
    conn.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    cursor = conn.cursor("Record", "by_i4");
    for rec in cursor:
        print 'i4=%s,str="%s"' % (rec.i4, rec.str)
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
        rec = conn.new("Record") # create python object

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
    # Perform simple index search: locate Record by id
    cursor = conn.cursor()
    # find record to update 
    rec = cursor.find("Record", "by_i4", 2)
    # update object
    rec.str = "Updated string"

    cursor.close() #release cursor
    conn.commit() # commit changes

    print_database(conn, "Database content after updating record with id == 2 :");

    #
    # Search operation
    #
    conn.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    cursor = conn.cursor()
    ret = cursor.search("Record", "by_i4", exdb.Operation.GreaterOrEquals, 2)

    if ret:
        rec = cursor.next()
        print "Object found: i4=%s,str=%s" % (rec.i4, rec.str)

        rec = cursor.next()
        print "Next object is: i4=%s,str=%s" % (rec.i4, rec.str)
    else:
        print "Object not found"
    print ""

    cursor.close()
    conn.commit()

    # 
    # Delete
    # 
    conn.startTransaction(exdb.Transaction.MCO_READ_WRITE)
    # Perform simple index search: locate Record by id
    cursor = conn.cursor()
    # find record with id == 3
    # Class may be referred either as database type or name (str)
    rec = cursor.find("Record", "by_i4", exdb.Operation.Equals)
    conn.remove(rec) # remove current object (pointed by cursor) from eXtremeDB database
    cursor.close() #release cursor
    conn.commit() # commit changes

    print_database(conn, "Database content after removing record with id == 3 :");

    # 
    # Delete all
    # 
    conn.startTransaction(exdb.Transaction.MCO_READ_WRITE)
    conn.deleteAll("Record")
    conn.commit()

    print_database(conn, "Database content after removing all records :");

    
    print 'Closing connection'
    conn.close()
    conn = None
finally:
    if not conn is None:
        conn.close()            

print 'Closing database'
db.close()
print 'Done'    
