require ('exdb')

is_debug = false
is_disk = false
is_shm = false

tmgr = 'mursiw'

function rmfile( fname )
    local f = io.open(fname, "r")
    if f then
        f:close()
        os.remove(fname)
        print("File deleted:", fname)
    end
end

rmfile('opendb.dbs')
rmfile('opendb.log')

print ("debug=", is_debug)
initRuntime{disk=is_disk,debug=is_debug,shm=is_shm,tmgr=tmgr}

print ("exdb initialized")
-- devices are a table list hence {{ ...}}. Possible options see in exdb.lua
devices = {{  type='conv',
              assignment= 'database', 
              size = 64*1024*1024  
           }}
-- db params is also a table
dbparams = {ddl_dict_size=64*1024, max_classes=100, max_indexes=100}

-- for persistent databases additinal devices are necessary
if is_disk then
    table.insert(devices, {type='conv', assignment='cache', size=64*1024*1024})
    table.insert(devices, {type='file', assignment='persistent', name='opendb.dbs'})
    table.insert(devices, {type='file', assignment='log', name='opendb.log'})

-- and addition paparemets
    dbparams.disk_page_size = 4096
end
-- finally create a database
db = create("luadb", devices, dbparams)


print ("database created", db, devices[1].ptr)

-- connect to it
c = connect{db=db}

--  c is a database connection and now can be used to perform SQL statements
print (c)

print ("Creating table")

c:statement("create table t(i int)")

print ("Inserting value")
c:statement("insert into t values([1,2,3])")

print ("Selecting count, db = ", db)

-- quering, cursor is a resut of thequery
curs = c:query("select count(*) as nrecs from t")

--print ("Select performed, curs = ", curs, "count=", curs:next().nrecs)
    
--
-- another query
--
print ("Selecting, db = ", db)
curs = c:query("select i from t")
print ("Select performed, curs = ", curs)
for elem in curs:rows() do
    print ("i = ", elem.i)
end

print ("Done. Closing database")
c:close()

close(db)
close('luadb')

print ("Free resources")
-- Free database memory with LUA GC
devices[1].ptr = nil


print ("Done")
