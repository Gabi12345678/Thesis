import exdb

schema = '''
#define int1      signed<1>
#define int2      signed<2>
#define int4      signed<4>
#define int8      signed<8>
#define uint8     unsigned<8>
#define uint4     unsigned<4>
#define uint2     unsigned<2>
#define uint1     unsigned<1>

#define CHARS_LEN  16
#define ARRAY_LEN   5

declare database types;

struct NestedStruct { 
    int4    key;
    string str;
};

class Record 
{
// Simple types
    int1  i1;
    int2  i2; 
    int4  i4; 
    int8  i8; 

    float  flt; 
    double dbl; 

    boolean bool;

// string types
    string str;

    char<CHARS_LEN> ch;
    
// nested struct
    NestedStruct nested;

    optional NestedStruct opt_nested;

// arrays
    signed<4> arr_i4[ARRAY_LEN];

    signed<1> arr_i1[ARRAY_LEN];

    string arr_str[ARRAY_LEN];

    NestedStruct arr_nested[ARRAY_LEN];

//vectors
    vector<signed<4>>    vec_i4;
    vector<signed<1>>    vec_i1;
    vector<string>       vec_str;
    vector<NestedStruct> vec_nested;

    blob b;
    
    list;
    autoid[1000];
};

'''

is_disk = False
tmgr = 'mursiw'
is_shm = False
is_debug = False

ARRAY_LEN = 5
N_RECORDS = 5

def createRecord(conn, i):
    rec = conn.new("Record")
        
    rec.i1 = i
    rec.i2 = i
    rec.i4 = i
    rec.i8 = i

    rec.bool = (i % 2 == 0)

    rec.flt = i
    rec.dbl = i

    rec.str = "string %d" % i
    rec.ch  = "chars %d" % i

    rec.arr_i1  = [i + j for j in xrange(ARRAY_LEN)]
    rec.arr_i4  = [i + j for j in xrange(ARRAY_LEN)]
    rec.arr_str = ["arr_string %s %s" % (i,j) for j in xrange(ARRAY_LEN)]

    rec.vec_i4     = [i + j for j in xrange(i)] 
    rec.vec_i1     = [i + j for j in xrange(i)]
    rec.vec_str    = ["arr_string %s %s" % (i,j) for j in xrange(i)]
    rec.vec_nested = []
    
    print 'record ', rec
    return rec;

# print out object
def printRecord(rec):
    print("Record ", rec.i4)
    print("-------------------------")
    print("i1 = ", rec.i1)
    print("i2 = ", rec.i2)
    print("i4 = ", rec.i4)
    print("i8 = ", rec.i8)
    print("dbl = ", rec.dbl)
    print("flt = ", rec.flt)
    print("bool = ", rec.bool)
    print("str = \"", rec.str, "\"")
    print("ch = \"", rec.ch, "\"")
    
    print("nested = {", rec.nested, "}")
    if rec.opt_nested is None:
        print("opt_nested = None")
    else:        
        print("opt_nested = {", rec.nested, "}")

    print("rec.arr_i1 = ", rec.arr_i1);

    print("rec.arr_i4 = ", rec.arr_i4)

    print("rec.arr_str = ", rec.arr_str);
    
    print("rec.arr_nested = ", rec.arr_nested);

    print("rec.vec_i1 = ", rec.vec_i1)

    print("rec.vec_i4 = ", rec.vec_i4)

    print("rec.vec_str = ", rec.vec_str)

    print("rec.vec_nested = ", rec.vec_nested);

    print("rec.b : Blob length = ", len(rec.b));

    print("\n\n");


#Load Runtime configuration specified by parameters
exdb.init_runtime(is_disk, tmgr, is_shm, is_debug)            

# Parse schema, load dictionary and create python dictionary object
dbdict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)
    
db = exdb.open_database(dbname='opendb', dictionary=dbdict, is_disk=is_disk, db_segment_size=128*1024*1024);
    
print 'Database created'

#Connect to the database 
con = None
try:
    con = db.connect()
    print 'Database connected. Connection object is', con

    # 
    # Insert data into the database
    # 
    con.startTransaction(exdb.Transaction.MCO_READ_WRITE)
    
    for i in xrange(N_RECORDS):
        createRecord(con, i)
        
    con.commit();

    #
    # Read data from the database
    # 
    con.startTransaction(exdb.Transaction.MCO_READ_ONLY)
    # open cursor by autoid index
    cursor = con.cursor("Record")
    for rec in cursor:
        printRecord(rec)
    
    cursor.close();
    con.commit();

finally:
    if not con is None:
        con.close()            

print 'Closing database'
db.close()
print 'Done'    


