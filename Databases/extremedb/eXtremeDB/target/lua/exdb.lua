local ffi = require 'ffi'

ffi.cdef[[
    typedef unsigned int uint4;
    typedef unsigned long long uint8;
 ]]

if ffi.abi("64bit") then
  ffi.cdef[[
    typedef uint8 mco_size_t;
    typedef uint8 mco_offs_t;
    typedef uint8 McoLuaSession;
]]
else
  ffi.cdef[[
    typedef uint4 mco_size_t;
    typedef uint4 mco_offs_t;
    typedef uint4 McoLuaSession;	
]]
end

ffi.cdef[[

  typedef struct McoLuaBinary {
      size_t size;
	  void*  data;
  } McoLuaBinary;

    typedef unsigned short uint2;
    typedef unsigned char uint1;

    typedef int mco_bool;
    typedef uint4          mco_counter32_t;

    typedef void* mco_db_h;

  typedef struct McoLuaDatabase   McoLuaDatabase;
  typedef struct McoLuaCursor     McoLuaCursor;
  typedef struct McoLuaDataSource McoLuaDataSource;
  typedef struct McoLuaTuple      McoLuaTuple;
  typedef struct McoLuaSequence   McoLuaSequence;
  typedef struct McoLuaValue      McoLuaValue;

    typedef struct mco_device_t_ {
	unsigned int type;       /* none, conv, named, file, raid, etc */
	unsigned int assignment; /* none, db-segment, cache-segment, db-file, log-file */
	mco_size_t   size;
	union {
	    struct {
		void * ptr;
		int flags;
	    } conv;
	    struct {
		char name[64];
		unsigned int flags;
		void * hint;
	    } named;
	    struct {
		int flags;
		char name[256];
	    } file;
	    struct {
		int flags;
		char name[64];
		mco_offs_t segment_size;
	    } multifile;
	    struct {
		int flags;
		char name[64];
		int level;
		mco_offs_t offset;
	    } raid;
	    struct {
		unsigned long handle;
	    } idesc;
	} dev;
    } mco_device_t, *mco_device_h;

    /* scheduling policy for transactions with the same priority */
    typedef enum MCO_TRANS_SCHED_POLICY_E_
    {
	MCO_SCHED_FIFO = 0,                    /* First In First Out */
	MCO_SCHED_READER_FAVOR = 1,            /* Run read-only transactions first */
	MCO_SCHED_WRITER_FAVOR = 2             /* Run read-write transactions first */
    } MCO_TRANS_SCHED_POLICY;


    typedef enum MCO_COMMIT_POLICY_E
    {
       MCO_COMMIT_SYNC_FLUSH, /* default  */
       MCO_COMMIT_BUFFERED,   /* runtime buffered transactions */
       MCO_COMMIT_DELAYED,    /* runtime buffered transactions, with commit criteria */
       MCO_COMMIT_NO_SYNC     /* changes are not synchronously written to the disk */
    } MCO_COMMIT_POLICY;

    typedef struct mco_log_params_t_
    {
	MCO_COMMIT_POLICY default_commit_policy;
	mco_offs_t        redo_log_limit;
	mco_offs_t        delayed_commit_threshold;
	mco_counter32_t   max_delayed_transactions;
	uint4             max_commit_delay; /* in milliseconds */
    } mco_log_params_t;

    typedef enum MCO_LOG_TYPE_ {
	NO_LOG, REDO_LOG, UNDO_LOG
    } MCO_LOG_TYPE;


	typedef struct mco_db_params_t_ {
	    uint2            mark;                    /**/
	    uint2            mem_page_size;           /* memory page size */
	    uint4            disk_page_size;          /* disk page size, typical 4096 bytes, set to zero for in-memory only database */
	    uint4            db_max_connections;      /* max number of connections */
	    mco_offs_t       disk_max_database_size;  /* max size of persistent database */
	    mco_offs_t       file_extension_quantum;  /* quantum of increasing size of database file (can help to reduce file fragmentation) */
	    MCO_LOG_TYPE     db_log_type;             /* database logging method */
	    uint2            connection_context_size; /* recovery connection context */

	    uint2            hash_load_factor;        /* criteria of hash reallocation in percent of used items */
	    uint2            index_optimistic_lock_threshold; /* maximum number of active write transactions when
								 optimistic locking of B-Tree indexes can be performed */
	    mco_log_params_t log_params;              /* initial log settings */

	    int              mode_mask;               /* open mode mask */
	    int              min_conn_local_pages;    /* minimal number of pages in per-connection allocator */
	    int              max_conn_local_pages;    /* maximal number of pages in per-connection allocator */

	    int              allocation_bitmap_caching_priority; /* priority of allocator bitmap pages in LRU page pool */
	    int              index_caching_priority;  /* priority of indices pages in LRU page pool */
	    int              object_caching_priority; /* priority of objects (non-blob) pages in LRU page pool */

	    struct mco_ddl_dictionary_t_* ddl_dict;
	    mco_size_t       ddl_dict_size;
	    int              ddl_dict_flags;

	    char*            cipher_key;             /* database encryption key */
	    mco_bool         dynamic_hash;           /* dynamically extend hash table when number of items exceeds load factor */
	    char*            license_key;            /* eXtremeDB license key */

	    int              max_classes;            /* used to reserve space for classes in database header to allow dynamic table creation */
	    int              max_indexes;            /* used to reserve space for indexes in database header to allow dynamic table creation */

	    mco_size_t       autocompact_threshold;  /* size of object after reaching which mco_w_compact is called during transaction commit */

	    MCO_TRANS_SCHED_POLICY trans_sched_policy; /* scheduling policy for transactions with the same priority */
	    uint8            max_trans_time;         /* maximal transaction time, 0 to disable */
	    int              max_active_pages;       /* maximal number of pages accessed by connection */
	    int              page_hash_bundles;      /* number of bundles in encryption/protection/compression page hash */
	    int              compression_level;      /* compression level: 0..9, 0 - no compression, -1: default compression level */
	    int              compression_mask;       /* bitmap of page kinds which should be compressed */
	    int              expected_compression_ratio;   /* use to allocate page map: virtual database space can not be larger than physical */
							   /* size compression ratio                                                           */
	    uint1            btree_cursor_read_ahead_size; /* number of keys taken from leaf B-Tree page at each access, can not be larger     */
							   /* than MCO_CFG_CURSOR_BUF_SIZE                                                     */
	    int              mvcc_bitmap_size;       /* MVCC bitmap size should be power of two */
	    int              additional_heap_size;   /* heap memory used by various internal DB functionalities, mostly for buffering during   */
						     /* XML export/import and database save/load operations                                    */
			mco_size_t       cow_pagemap_size;       /* Size of copy-on-write MVCC pagemap (in bytes, one byte per disk page) */

	    mco_size_t       backup_map_size;        /* size of backup counters array, bytes, power of two, ignored if disk_max_database_size set */
						     /* set to zero do disable backup feature */
	    unsigned int     backup_min_pages;       /* number of pages for last exclusive pass of backup procedure, set to zero to disable treshold */
	    unsigned int     backup_max_passes;      /* max number of passes before exclusive pass of backup procedure */
	    char             backup_map_filename[256]; /* name of a file will be used to store backup temporary data on mco_db_close() call */
						     /* eXtremeDB will use filesystem wrapper linked into the process to store the data   */
						     /* optional, set zero for default "<persistent-storage>.bmap" file locate at the same   */
						     /* location as persistent storage file  */
        uint8            iot_agent_id;
        uint2            iot_level;
		uint4	         file_backup_delay; 
} mco_db_params_t;

    typedef struct mco_ssl_params_t_ {
	const char *cipher_list;
	long max_cert_list;
	unsigned long options;
	const char *tmp_dh;
	unsigned int verify_mode;
	int verify_depth;

	const char *certificate;
	const char *private_key;
	const char *certificate_file;
	const char *private_key_file;
    } mco_ssl_params_t;

  typedef void *mco_dict_h;

  void exdbParamsInit(mco_db_params_t *dbparams);

  mco_dict_h exdbLoadDictionary(const char *schema, bool persistent, bool debug, bool nosort, bool largeDatabase, bool compact);

  McoLuaDatabase *exdbCreateDatabaseDev(const char *db_name, mco_dict_h dict, mco_device_t *dev, int ndev, mco_db_params_t *dbparams);
  int exdbDisconnect(mco_db_h dbh);
  int exdbCloseDatabase(const char *db_name);

  McoLuaDataSource* exdbExecuteQuery(McoLuaSession session, const char* query, ...);
  int exdbExecuteStatement(McoLuaSession session, const char* query, ...);
  void exdbCloseCursor(McoLuaDataSource* ds, McoLuaCursor* cursor);
  McoLuaCursor* exdbOpenCursor(McoLuaDataSource* ds);
  McoLuaCursor* exdbResetCursor(McoLuaDataSource* ds, McoLuaCursor* cursor);

  void exdbCloseSession(McoLuaSession session);
  McoLuaSession exdbOpenSession(char const* dbname, size_t dbsize, size_t pagesize);
  McoLuaSession exdbConnectSession(McoLuaDatabase *dbh);
  McoLuaSession exdbRemoteConnectSession(const char *hostname, int port, int maxConnectAttempts, bool localDomain, void* sslParameters,
					      uint8 connectTimeout, uint8 readTimeout, const char *sql_login, const char *sql_password, int compressionLevel);
  McoLuaSession exdbDistributedConnectSession(char **nodes, int nNodes, int nReplicas, int replType,
                                                     int maxConnectAttempts, int* badNode, bool localDomain, void* sslParameters,
                                                     uint8 connectTimeout, uint8 readTimeout, const char *sql_login, const char *sql_password, int compressionLevel);

  McoLuaTuple* exdbNextTuple(McoLuaCursor* cursor);
  int exdbNumberOfColumns(McoLuaDataSource* ds);

  char const* exdbColumnName(McoLuaDataSource* ds, int column);
  char const* exdbColumnType(McoLuaDataSource* ds, int column);
  bool exdbIsNull(McoLuaTuple* tuple, int column);
  bool exdbGetBool(McoLuaTuple* tuple, int column);
  int exdbGetInt(McoLuaTuple* tuple, int column);
  double  exdbGetReal(McoLuaTuple* tuple, int column);
  char const* exdbGetString(McoLuaTuple* tuple, int column);
  McoLuaBinary* exdbGetBinary(McoLuaTuple* tuple, int column);

 McoLuaSequence* exdbGetSequence(McoLuaTuple* tuple, int column);
  int  exdbSequenceCount(McoLuaSequence* seq);

  void exdbExtract(McoLuaDataSource* ds, McoLuaTuple* tuple, void* dst, size_t size);

  int loadRuntime(bool disk, const char *tmgr, bool shm, bool debug, const char *cluster, bool ha, bool disk_compression, bool posix);
  const char *exdbLastError();

  int exdb_ssl_load_verify_locations(const char *ca_file, const char *ca_path);
  int exdb_ssl_init();
  void exdb_ssl_params_init(mco_ssl_params_t *params);

  McoLuaValue *exdbIntValue(McoLuaSession engine, int val);
  McoLuaValue *exdbRealValue(McoLuaSession engine, double val);
  McoLuaValue *exdbStringValue(McoLuaSession engine, char *val);
  McoLuaValue *exdbNullValue(McoLuaSession engine);
]]

exdbNull = 0
exdbBool = 1
exdbInt1 = 2
exdbUInt1 = 3
exdbInt2 = 4
exdbUInt2 = 5
exdbInt4 = 6
exdbUInt4 = 7
exdbInt8 = 8
exdbUInt8 = 9
exdbReal4 = 10
exdbReal8 = 11
exdbDateTime = 12
exdbNumeric = 13
exdbUnicode = 14
exdbString = 15
exdbBinary = 16
exdbReference = 17
exdbArray = 18
exdbStruct = 19
exdbBlob = 20
exdbDataSource = 21
exdbList = 22
exdbSequence = 23
exdbAverage = 24
exdbNullableArray = 25
exdbAsArg1Type = 26
exdbAsArg2Type = 27
exdbAsArg3Type = 28
exdbAsArg1XType = 29
exdbAsArg1ElemType = 30
exdbAny = 31

exdbInt = exdbInt8
exdbReal = exdbReal8

local C = nil

if not (current_session == nil) then
    if ffi.os == "Windows" then
         if debug_runtime > 0 then 
           apilib = "mcoluaapi_debug.dll"
         else
           apilib = "mcoluaapi_debug.dll"
         end

        if mco_library_path then
           apilib = mco_library_path .. "\\" .. apilib
        end
        C = ffi.load(apilib)
    else
      local lib_path = ''
      local suffix   = ''
      if (mco_library_path) then
          lib_path = mco_library_path .. '/'
          suffix = '.so'
      end
      if (debug_runtime) then
         C = ffi.load(lib_path .. 'libmcoluaapi_debug' .. suffix)
      else
         C = ffi.load(lib_path .. 'libmcoluaapi' .. suffix)
       end
    end
end

function exdbError()
   return ffi.string(C.exdbLastError())
end

function initRuntime(t)

    setmetatable(t, {__index={disk=false, tmgr='mursiw', shm=false, debug=false, cluster=nil, ha=false, disk_compression=true, posix=true}})

    local disk, tmgr, shm, debug, cluster, ha, disk_compression, posix =
       t['disk'] or t.disk,
       t['tmgr'] or t.tmgr,
       t['shm'] or t.shm,
       t['debug'] or t.debug,
       t['cluster'] or t.cluster,
       t['ha'] or t.ha,
       t['disk_compression'] or t.disk_compression,
       t['posix'] or t.posix

       sizes = {
          mco_device_t = ffi.sizeof("mco_device_t"),
          mco_log_params_t = ffi.sizeof("mco_log_params_t"),
          mco_db_params_t = ffi.sizeof("mco_db_params_t"),
          mco_ssl_params_t = ffi.sizeof("mco_ssl_params_t")
       }
       
    if (ffi.os == 'Windows') then
      if (debug) then
        wrapper_name = "mcoluawrap_debug.dll"
      else
        wrapper_name = "mcoluawrap.dll"
      end
      wrapper_init_fn, err, err2 = package.loadlib(wrapper_name, "wrapper_init")
      if wrapper_init_fn == nil then
        error(string.format("Could not initialize eXtremeDB runtime with %s (%s:%s)", wrapper_name, err, err2))
      end
      wrapper_init_fn(disk, tmgr, shm, cluster, ha, disk_compression)
      C = ffi.load(wrapper_name)
      print ("C = ", C)
    else

       if debug then
          loader_name = "libmcoluaload_debug.so"
       else
          loader_name = "libmcoluaload.so"
       end
       loader = assert(package.loadlib(loader_name, "luaopen_exdblib"))

       l = loader()

       l.exdb_load_runtime(disk, tmgr, shm, debug, cluster, ha, disk_compression, posix)

       if debug then
          luaapi = 'mcoluaapi_debug.so'
       else
          luaapi = 'mcoluaapi.so'
       end
       C = ffi.load(luaapi)
    end
    exdbCheckRuntime(sizes)
end

function createClass(c, name)
    c = c or {}
    c.new = function(self, o, name)
        o = o or {}    -- create table if not provided by user
        setmetatable(o, self)
        self.__index = self
        if o.init then o:init() end
        return o
    end
    return c
end

Iter = createClass()

-- Return cursor iterator which can be used in for statement
-- Example: for t in con.query("select * from T"):tuples() do ... end
function Iter:tuples()
  return function()
    local rec = self:next()
	if not rec then
	  self:close()
    end
	return rec
  end
end

-- Materialize cursor into column-based array 
function Iter:columns()
  local row = self:next()
  if not row then 
    return {}
  end
  ret = {}
  for k,v in pairs(row) do
    ret.k = {v}
  end
  row = self:next()
  while row do
    for k,v in pairs(row) do
      table.insert(ret[k], v)
    end
    row = self:next()
  end
  return ret
end


-- Materialize and sort elements
-- Parameters:
--   by: list of order column name
-- Returns: Lua table
-- Example: iter:sort({'name', 'age'})
function Iter:sort(by)
  local tbl = self:materialize()
  table.sort(tbl, function (a,b) for i,k in ipairs(by) do if a[k] < b[k] then return true elseif a[k] > b[k] then return false end end end)
  return tbl
end

-- Extract all elements into Lua table and close iterator
-- Returns: Lua table
function Iter:materialize(columns)   
   local tbl = {}
   if columns then
      tbl._columns = true
      local row = self:next()
      if not row then
         return tbl
      end
      for k,v in row do
         tbl[k] = {v}
      end
      row = self:next()
      while row do
         for k,v in row do
            table.insert(tbl[k], v)
         end
         row = self:next()
      end
      return tbl
   else 
      while true do
         local elem = self:next()
         if not elem then
            self:close()
            return tbl
         end
         table.insert(tbl, elem)
      end
   end
end

-- Apply function for each element
-- Parameters:
--   action: function to be applied for all selected records
-- Example: cursor:foreach(function(rec) sum = sum + rec.weigth end)
function Iter:foreach(action)
  local elem = self:next()
  while elem do
    action(elem)
    elem = self:next()
  end
  self:close()
end

-- Count number of records
function Iter:count()
  local n = 0
  while true do
    local rec = self:next()
    if not rec then
      return n
    end
    n = n + 1
  end
end

-- Print records
function Iter:print()
  while true do
    local rec = self:next()
    if not rec then
	  self:close()
      break
    end
    print(table.concat(rec, ','))
  end
end

    
function Iter:next() error"NYI: Iter.next" end

function Iter:close()
  if self.source then
    self.source:close()
    self.source = nil
  end
end

function Iter:reset() error"NYI: Iter.reset" end

function Iter:rows()
    return function()
        return self:next()
    end
end 
    

-- Helper function to allow iteration over cursor like
--  for row in rows(it) do print(row) end
function rows(it)   
   return function ()
     return it:next()
   end
end

    
local Cursor = Iter:new()

-- Move cursor to next record. This function can be used instead of next()
--   if record fields are accessed using Cursor:getXXX methods.
-- Returns: true if there is next record, false otherwise.
function Cursor:moveNext()
  self.rec = C.exdbNextTuple(self.csr)
  --self.rec = exdbNextTupleS(self.csr)
  return self.rec ~= nil
end


-- Get next records
-- Returns: next record or nil if end of set is reached
function Cursor:next()
  self.rec = C.exdbNextTuple(self.csr)
  if self.rec ~= nil then return self:getRow() end
  return nil
end

-- Get result column name
-- Parameters:
--   column: zero-based column index
-- Return: column name
function Cursor:getColumnName(column)
  return ffi.string(C.exdbColumnName(self.rs, column))
end

-- Get result column type
-- Parameters:
--   column: zero-based column index
-- Return: column type ('void', 'int', 'real', 'string', 'bool', 'array', 'struct', 'binary', 'raw', 'sequence')
function Cursor:getColumnType(column)
  return ffi.string(C.exdbColumnType(self.rs, column))
end

-- Extract record in C structure (created using FFI)
-- Parameters:
--   dst: address of structure (createds using ffi.new)
--   size: size fo structure (ffi.sizeof)
function Cursor:extract(dst,size)
  C.exdbExtract(self.rs, self.rec, dst, size)
end

-- Check if column is null
-- Parameters:
--   column: zero-based column index
-- Return: true if column is null
-- Example: cursor:isNull(0)
function Cursor:isNull(column)
  return C.exdbIsNull(self.csr)
end

-- Get integer column
-- Parameters:
--   column: zero-based column index
-- Return: column value
-- Example: cursor:getInt(0)
function Cursor:getInt(column)
  return C.exdbGetInt(self.rec,column)
end

-- Get floating point column
-- Parameters:
--   column: zero-based column index
-- Return: column value
-- Example: cursor:getReal(0)
function Cursor:getReal(column)
  return C.exdbGetReal(self.rec,column)
end

-- Get string column
-- Parameters:
--   column: zero-based column index
-- Return: column value
function Cursor:getString(column)
  return ffi.string(C.exdbGetString(self.rec,column))
end

-- Get binary column
-- Parameters:
--   column: zero-based column index
-- Return: column value
function Cursor:getBinary(column)
  return C.exdbGetBinary(self.rec,column)
end

-- Get column value
-- Parameters:
--   column: zero-based column index
-- Return: column value
function Cursor:get(column)
  return exdbGetValue(self.rec,column)
end


-- Get sequence column
-- Parameters:
--   column: zero-based column index
-- Return: raw sequence: it can be passwed to xSQL sequrence functions or converted to sequence iterator using Sequence.iterator function
function Cursor:getSequence(column)
  return C.exdbGetSequence(self.rec,column)
end


-- Get boolean column
-- Parameters:
--   column: zero-based column index
-- Return: column value
-- Example: cursor:getBool(0)
function Cursor:getBool(column)
  return C.exdbGetBool(self.rec,column)
end

-- Get number of columns in result set
function Cursor:numberOfColumns()
  return C.exdbNumberOfColumns(self.rs)
end

-- Get current record as Lua table
function Cursor:getRow()
  local proxy = self.proxy
  if proxy then
    return proxy
  else
    return exdbCurrentRow(self.rs, self.rec)
  end
end

-- Close cursor
function Cursor:close()
  if self.csr then
    C.exdbCloseCursor(self.rs, self.csr)
    self.csr = nil
    self.rs = nil
  end
end

-- Reset cursor
function Cursor:reset()
  if self.rs == nil then 
    error ("reset empty dataset")
  end
  self.csr = C.exdbResetCursor(self.rs, self.csr)
end


-- Setup cursor to lazily extract columns
function Cursor:lazy()
  local n = self:numberOfColumns()
  local getters = {}

  local getters = {int = Cursor.getInt, real = Cursor.getReal, string = Cursor.getString, bool = Cursor.getBool, array = Cursor.get, struct = Cursor.get, sequence = Cursor.getSequence}

  for i=0,n-1 do
    getters[self:getColumnName(i)] = {i,getters[self:getColumnType(i)]}
  end

  local index = function(this, key)
    local method = getters[key]
    local column_no = method[1]
    local getter = method[2]
    return getter(self, column_no)
  end

  local metatable = {__index = index}
  self.proxy = setmetatable({}, metatable)

  return self
end

local SqlEngine = createClass()

-- Execute query
-- Parameters:
--   query: SQL query with %b (bool), %s (string), %L (integer), %f (real) placeholders used for parameters
--   varying list of string parameters
-- Returns: cursor
-- Examples: engine:query("select * from Customer where id=%s", cust_id)
function SqlEngine:query(sql, ...)
   local arg = {...}
   local ds = C.exdbExecuteQuery(self.session, sql, unpack(arg))
   if ds == nil then      
      error(ffi.string(C.exdbLastError()))
   end
   return Cursor:new({rs = ds, csr = C.exdbOpenCursor(ds), rec = nil})
end

-- Execute query for a single row and return all the columns
-- It expects that query result is one row
function SqlEngine:query_once(sql,...)
   local arg = {...}
   return exdbExecuteAndFetch(self.session, sql, unpack(arg))
end

-- Execute query for a single row and return single row result as a Lua table
-- It expects that query result is one row
function SqlEngine:query_row(sql,...)
   local arg = {...}
   return exdbExecuteAndFetchTable(self.session, sql, unpack(arg))
end

-- Execute insert/update/delete SQL statement
-- Parameters:
--   query: SQL statement with %[b (bool), %s (string), %L (integer), %f (real) placeholders used for parameters
--   varying list of string parameters
-- Returns: number of affected rows
-- Examples: engine:statement("insert into Customer values (%s,%s,%s)", "ABC123", "Honjuan Inc.", "Shengen, po.1234")
function SqlEngine:statement(sql, ...)
   local arg = {...}
   ret = C.exdbExecuteStatement(self.session, sql, unpack(arg))
   if ret == -2 then      
      error(ffi.string(C.exdbLastError()))
   end
   return ret
end

-- Call xSQL function with specified arguments
-- Parameters:
--   func: function name
--   varying list of string parameters
-- Returns: result of the function
-- Examples: engine:call("seq_avg",price)
function SqlEngine:call(func,...)
  local arg = {...}
  return exdbCallFunction(self.session, func, arg)
end


-- Close current session
function SqlEngine:close()
  if self.session then
    C.exdbCloseSession(self.session)
	self.session = nil
  end
end

dev_types = {
    null = 0,
    conv = 1,
    named = 2,
    file = 3,
    multifile = 4,
    raid = 5,
    int_desc = 6,
    cyclic_file_buf = 7
}

dev_assigns = {
    database = 0,
    cache = 1,
    persistent = 2,
    log = 3,
    ha_async_buf = 4,
    pipe_buf = 5
}

--
-- For IM conv memory device we allocate memory and store pointer to it to devs[i].ptr variable.
-- This pointer and variable should exists in all time when database is used
--
function create(dbname, devs, params, schema, persistent, debug, nosort, largeDatabase, compact, transient)

    pdevs = ffi.new("mco_device_t[?]", #devs)

    for i=1,#devs do

	no = i-1

	pdevs[no].type = dev_types[ devs[i].type ]
	pdevs[no].assignment = dev_assigns[devs[i].assignment]


	if devs[i].type == 'conv' then
	    pdevs[no].size  = devs[i].size
	    devs[i].ptr = ffi.new("char[?]", devs[i].size)
	    pdevs[no].dev.conv.ptr = devs[i].ptr
	    pdevs[no].dev.conv.flags = devs[i].flags == nil and 0 or devs[i].flags
	elseif devs[i].type == 'named' then
	    -- set memory name
	    pdevs[no].size  = devs[i].size
	    pdevs[no].dev.named.name = devs[i].name;
	    pdevs[no].dev.named.flags = devs[i].flags == nil and 0 or devs[i].flags
	    pdevs[no].dev.named.hint  = devs[i].hint == nil and 0 or devs[i].hint
	elseif devs[i].type == 'file' then

	    pdevs[no].size  = devs[i].size == nil and 0 or devs[i].size
	    pdevs[no].dev.file.name = devs[i].name
	    pdevs[no].dev.file.flags = devs[i].flags == nil and 0 or devs[i].flags

	elseif devs[i].type == 'multifile' then
	    pdevs[no].size  = devs[i].size == nil and 0 or devs[i].size
	    pdevs[no].dev.multifile.name = devs[i].name
	    pdevs[no].dev.multifile.flags = devs[i].flags == nil and 0 or devs[i].flags
	    pdevs[no].dev.multifile.segment_size = devs[i].size

	elseif devs[i].type == 'raid' then
	    pdevs[no].size  = devs[i].size == nil and 0 or devs[i].size
	    pdevs[no].dev.raid.name = devs[i].name
	    pdevs[no].dev.raid.flags = devs[i].flags == nil and 0 or devs[i].flags
	    pdevs[no].dev.raid.level = devs[i].level
	end

    end

    local dbparams = ffi.new("mco_db_params_t")
    C.exdbParamsInit(dbparams)

    if not (params.mem_page_size == nil) then dbparams.mem_page_size = params.mem_page_size end
    if not (params.disk_page_size == nil) then dbparams.disk_page_size = params.disk_page_size end
    if not (params.db_max_connections == nil) then dbparams.db_max_connections = params.db_max_connections end
    if not (params.disk_max_database_size == nil) then dbparams.disk_max_database_size = params.disk_max_database_size end
    if not (params.file_extension_quantum == nil) then dbparams.file_extension_quantum = params.file_extension_quantum end
    if not (params.db_log_type == nil) then dbparams.db_log_type = params.db_log_type end
    if not (params.connection_context_size == nil) then dbparams.connection_context_size = params.connection_context_size end

    if not (params.hash_load_factor == nil) then dbparams.hash_load_factor = params.hash_load_factor end
    if not (params.index_optimistic_lock_threshold == nil) then dbparams.index_optimistic_lock_threshold = params.index_optimistic_lock_threshold end

    if not (params.log_params == nil) then
	if not (params.log_params.default_commit_policy == nil) then dbparams.log_params.default_commit_policy = params.log_params.default_commit_policy end
	if not (params.log_params.redo_log_limit == nil) then dbparams.log_params.redo_log_limit = params.log_params.redo_log_limit end
	if not (params.log_params.delayed_commit_threshold == nil) then dbparams.log_params.delayed_commit_threshold = params.log_params.delayed_commit_threshold end
	if not (params.log_params.max_delayed_transactions == nil) then dbparams.log_params.max_delayed_transactions = params.log_params.max_delayed_transactions end
	if not (params.log_params.max_commit_delay == nil) then dbparams.log_params.max_commit_delay = params.log_params.max_commit_delay end
    end

    if not (params.mode_mask == nil) then dbparams.mode_mask = params.mode_mask end
    if not (params.min_conn_local_pages == nil) then dbparams.min_conn_local_pages = params.min_conn_local_pages end
    if not (params.max_conn_local_pages == nil) then dbparams.max_conn_local_pages = params.max_conn_local_pages end

    if not (params.allocation_bitmap_caching_priority == nil) then dbparams.allocation_bitmap_caching_priority = params.allocation_bitmap_caching_priority end
    if not (params.index_caching_priority == nil) then dbparams.index_caching_priority = params.index_caching_priority end
    if not (params.object_caching_priority == nil) then dbparams.object_caching_priority = params.object_caching_priority end

    if not (params.ddl_dict_size == nil) then dbparams.ddl_dict_size = params.ddl_dict_size end
    if not (params.ddl_dict_flags == nil) then dbparams.ddl_dict_flags = params.ddl_dict_flags end

    if not (params.cipher_key == nil) then dbparams.cipher_key = params.cipher_key end
    if not (params.dynamic_hash == nil) then dbparams.dynamic_hash = params.dynamic_hash end
    if not (params.license_key == nil) then dbparams.license_key = params.license_key end

    if not (params.max_classes == nil) then dbparams.max_classes = params.max_classes end
    if not (params.max_indexes == nil) then dbparams.max_indexes = params.max_indexes end

    if not (params.autocompact_threshold == nil) then dbparams.autocompact_threshold = params.autocompact_threshold end

    if not (params.trans_sched_policy == nil) then dbparams.trans_sched_policy = params.trans_sched_policy end
    if not (params.max_trans_time == nil) then dbparams.max_trans_time = params.max_trans_time end
    if not (params.max_active_pages == nil) then dbparams.max_active_pages = params.max_active_pages end
    if not (params.page_hash_bundles == nil) then dbparams.page_hash_bundlese = params.page_hash_bundles end
    if not (params.compression_level == nil) then dbparams.compression_level = params.compression_level end
    if not (params.compression_mask == nil) then dbparams.compression_mask = params.compression_mask end
    if not (params.expected_compression_ratio == nil) then dbparams.expected_compression_ratio = params.expected_compression_ratio end
    if not (params.btree_cursor_read_ahead_size == nil) then dbparams.btree_cursor_read_ahead_size = params.btree_cursor_read_ahead_size end
    if not (params.mvcc_bitmap_size == nil) then dbparams.mvcc_bitmap_size = params.mvcc_bitmap_size end
    if not (params.additional_heap_size == nil) then dbparams.additional_heap_size = params.additional_heap_size end
    if not (params.cow_pagemap_size == nil) then dbparams.cow_pagemap_size = params.cow_pagemap_size end
    if not (params.backup_map_size == nil) then dbparams.backup_map_size = params.backup_map_size end
    if not (params.backup_min_pages == nil) then dbparams.backup_min_pages = params.backup_min_pages end
    if not (params.backup_max_passes == nil) then dbparams.backup_max_passes = params.backup_max_passese end
    if not (params.backup_map_filename == nil) then dbparams.backup_map_filename = params.backup_map_filename end

    local dict = nil

    if not (schema == nil) then
       dict = C.exdbLoadDictionary(schema, persistent, debug, nosort, largeDatabase, compact)
    end

    return C.exdbCreateDatabaseDev(dbname, dict, pdevs, #devs, dbparams)
end

function close(db)
    print ("Closing. db=", db, "type(db)=", type(db))
    if type(db) == 'string' then
     if C.exdbCloseDatabase(db) ~= 0 then
        error(exdbError())
     end
    elseif type(db) == 'cdata' then
     if C.exdbDisconnect(db) ~= 0 then
        error(exdbError())
     end
    elseif type(db) == 'table' then
        db:close()
    else
        error("Invalid db")
    end
end

-- Create and connect to the database
-- Parameters:
--   params: a table
-- Returns: engine
-- Example: connect({dbname='testdb', pagesize=1024, dbsize=10*1024*1024})
function connect(params)
    local ret = nil


    if params.db ~= nil and type(params.db) == "cdata" then
        local sess = C.exdbConnectSession(params.db)
        if sess == nil then
            return nil
        end
        ret = SqlEngine:new({session = sess})
    elseif params.dbname ~= nil or type(params) == 'string' then
        local dbname 
        if type(params) == 'string' then
            dbname = params
        else
            dbname = params.dbname
        end
        local dbsize = params.dbsize or 10*1024*1024
        local sess = C.exdbOpenSession(dbname, dbsize, params.pagesize or 256)
        if sess == 0 then
            return nil
        end
        
        ret = SqlEngine:new({session = tonumber(sess)})
    elseif params['host'] ~= nil then
        local sess = C.exdbRemoteConnectSession(params.host, params.port,
            params.maxConnectAttempts or 10, params.localDomain or false, params.ssl_params or nil,
            params.connectTimeout or 2*1000, params.readTimeout or 1200*1000,
            params.sql_login or nil, params.sql_password or nil,
            params.compressionLevel or 0)

        if sess == 0 then
            return nil
        end

        ret = SqlEngine:new({session = sess})

    elseif params['nodes'] ~= nil then
        local nNodes = #params.nodes      
        nodes = ffi.new('char *[?]', nNodes)
        badNode = ffi.new('int *')
        for i = 1, nNodes do 
          nodes[i-1] = ffi.new("char[?]", #params.nodes[i] )
          ffi.copy (nodes[i-1], params.nodes[i])          
        end
        local sess = C.exdbDistributedConnectSession(nodes, nNodes,
            params.nReplicas or 1, params.replType or 0,
            params.maxConnectAttempts or 10, badNode, params.localDomain or false, params.ssl_params or nil,
            params.connectTimeout or 2*1000, params.readTimeout or 1200*1000,
            params.sql_login or nil, params.sql_password or nil,
            params.compressionLevel or 0)

        if sess == 0 then
            return nil
        end

        ret = SqlEngine:new({session = sess})
    else 
      error "Can't find connect method"
    end
    return ret
end

SSLVerifyMode = {
    VerifyNone = 0,
    VerifyPeer = 1,
    VerifyFailIfNoPeerCert = 2,
    VerifyClientOnce = 4
}

function ssl_init()
    local ret = C.exdb_ssl_init()
    return ret
end

function ssl_load_verify_locations(ca_file, ca_path)
    return C.exdb_ssl_load_verify_locations(ca_file, ca_path)
end

function ssl_params_init(params)
    C.exdb_ssl_params_init(params)
end

-- Current connection (for invoked stored procedure)
con = SqlEngine:new({session = current_session})

-- Iterator for the sequence, allows traverse over the sequence in the form:
-- for elem in iseq(it) do print(elem) end
-- it - is a sequence "handle" which is passed to the UDF as an argument, userdata
function iseq(it)   
   return function ()
     return exdbSequenceNext(it)
   end
end

-- Group iterator for the sequence, allows traverse over many sequences in the form:
-- for el1,el2,el3 in iseqn(it1,it2,it3) do print(elem) end
-- it1,... itN - are a sequence "handles" which are passed to the UDF as an argument, userdata
function iseqn(...)
    local args = {...}
   return function ()
     local ret = {}
     for i,v in ipairs(args) do
       ret[i] = exdbSequenceNext(v)
     end
     return unpack(ret)
   end
end

    
local Sequence = Iter:new()

-- Get next sequence element
function Sequence:next()
  return exdbSequenceNext(self.handle)
end

-- Search for subsequence sequence iterator
function Sequence:subseq()
  return C.exdbSequenceSubseq(self.handle)
end

-- Count element in the sequence
function Sequence:count()
  return C.exdbSequenceCount(self.handle)
end

-- Reset sequence iterator
function Sequence:reset()
  return exdbSequenceReset(self.handle)
end

-- Create Sequence with given handle
function seqiter(h)
  return Sequence:new({handle = h})
end
