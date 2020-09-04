import os
import sys
import exdb

#define eXtremeDB schema - exactly like in .mco file
schema = '''
#define uint4     unsigned<4>

declare database opendb;

class Class1  {
    uint4 i4;
};
'''

is_disk = False
tmgr = 'mursiw'
is_shm = False
is_debug = False

#Load Runtime configuration specified by parameters
exdb.init_runtime(is_disk, tmgr, is_shm, debug=is_debug)

# Parse schema, load dictionary and create python dictionary object
dict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)

print 'List structures in the loaded dictionary'
for s in dict.structs:
    print 'type ', type(dict.structs[s])
    print "%s=>%s - %s" % (s, dict.structs[s], dict.structs[s].__dict__)
    for fld in dict.structs[s].__dict__:
        f = dict.structs[s].__dict__[fld]
        if issubclass(type(f), exdb.eXtremeDBField):
            print "fld %s=>%s" % (fld, f.__dict__)

print 'List classes in the loaded dictionary'
for s in dict.classes:
    cls = dict.classes[s]
    print "%s=>%s - %s" % (s, cls, cls.struct.__dict__)
    for fld in cls.struct.__dict__:
        f = cls.struct.__dict__[fld]
        if issubclass(type(f), exdb.eXtremeDBField):
            print "fld %s=>%s" % (fld, f.__dict__)

# Open (create) database with default parameters

db = exdb.open_database(dbname='opendb', dictionary=dict, is_disk=is_disk, db_segment_size=128*1024*1024)
    
print 'Database created'
#Connect to the database 
conn = None
with db.connect() as conn:

    print 'Database connected. Connection object is', conn

# Open transaction. By default transaction is opened in Read-Write mode
    conn.startTransaction()

# Create an object of type specified in the schema definition
    o = conn.new("Class1")

# Print field(s) of created object with fields filled with default values
    print o   
    
# Fields in object are filled with default values, so we can now save object in database
    conn.commit() 

# Connection will automatically close when leaving scope of with statement.
# It may be handled manually if use:
# conn = db.connect()
# ....
# conn.close()    

print 'Closing database'
db.close()
print 'Done'
