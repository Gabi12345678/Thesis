require ('exdb')
ffi = require('ffi')

function fmt_sec( sec )
    str = ''
    local s = seqiter(sec):materialize()
    for i,p in pairs(s) do
        str = str .. ' ' .. p
    end       
    return str
end

is_debug = false
is_disk = false
is_shm = false

tmgr = 'mursiw'

initRuntime{disk=is_disk,debug=is_debug,shm=is_shm,tmgr=tmgr}

print ("exdb initialized")

c = connect({dbname='testdb', pagesize=1024, dbsize=1024*1024*1024})

print ('Connection is:', c)

print ("Creating tables")
c:statement("create table tbl(i int, si sequence(int asc), sf sequence(float), su1 sequence(unsigned(1)), sc sequence(char(10)))")
c:statement("create table tr(i int, iv int null, fv float null, u1v unsigned(1) null, cv char(10) null)")

print ("Inserting value")
c:statement("insert into tbl values(1, [1,2,3], [1.1,2.2,3.3], [2,3,4], ['hello', 'world', '!'])")

print ("Selecting count(*)")
for t in c:query("select count(*) from tbl"):tuples() do
    print (t)
    for key,value in pairs(t) do print(key,value) end
end

print ("Inserting array")

function pad(s, len)
   return s .. string.rep(' ', len - #s)
end

arr =  {'hello ', 'world', '!\0!'}
padded_arr = {}
for k,s in ipairs(arr) do
   table.insert(padded_arr, pad(s,10))
end

c:statement("insert into tbl values(1, %i4, %f, %u, %s)", 
    ffi.new('int', 2), 
    0.5, 
    ffi.new('unsigned char', 45),
    '10')
    

c:statement("insert into tbl values(1, %I4, %F4, %U1, %A)", 
    ffi.new('size_t', 3), ffi.new('int[4]', {1,2,3}), 
    ffi.new('size_t', 3), ffi.new('float[4]', {1.1,2.2,3.3}),
    ffi.new('size_t', 3), ffi.new('unsigned char[4]', {1,2,3}),
    ffi.new('size_t', 3), ffi.new('size_t', 10), ffi.new('char [?][10]', #padded_arr, padded_arr)
    )

print ("Selecting")
for t in c:query("select * from tbl"):tuples() do
    print ("t=", t)
    print ("si", fmt_sec(t.si))
    print ("sf", fmt_sec(t.sf))
    print ("su1", fmt_sec(t.su1))
    print ("sc", fmt_sec(t.sc))
end

print ("session = ", c.session)
print ("exdbCreateValue(c, 1, exdbInt), ", c.session, 1, exdbInt)


c:statement("insert into tr values(1, %v, %v, %v, %v)", 
    exdbCreateValue(c.session, exdbInt, 1), 
    exdbCreateValue(c.session, exdbReal, nil), 
    exdbCreateValue(c.session, exdbInt, 14), 
    exdbCreateValue(c.session, exdbString, nil)
)

c:statement("insert into tr values(2, %v, %v, %v, %v)", 
    exdbCreateValue(c.session, exdbInt, nil),
    exdbCreateValue(c.session, exdbReal, 1), 
    exdbCreateValue(c.session, exdbInt, nil), 
    exdbCreateValue(c.session, exdbString, 'hello world')
)

for tr in c:query("select * from tr"):tuples() do
    print ("tr:", tr.i, tr.iv, tr.fv, tr.u1v, tr.cv)
end
