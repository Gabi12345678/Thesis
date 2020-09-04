import exdb

schema = '''
#define uint4     unsigned<4>
declare database opendb;
class myclass  {
uint4 i4;
};
'''

exdb.init_runtime(disk=False, tmgr='mursiw', shm=False, debug=False, UsePerfmon=True)

dict = exdb.load_dictionary(schema, persistent=False, debug=False)
db = exdb.open_database(dbname='mydb', dictionary=dict, is_disk=False, db_segment_size=128*1024*1024)

exdb.Perfmon.init()
exdb.Perfmon.attach(db)

hv = exdb.HV()
hv.start()

raw_input("Press Enter to stop...")

hv.stop()

exdb.Perfmon.detach(db)
exdb.Perfmon.close()

