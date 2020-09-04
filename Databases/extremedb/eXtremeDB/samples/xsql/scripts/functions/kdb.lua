-- Convert table to array

function maptoarray(map,key)
  local arr = {}
  for k,v in pairs(map) do
    v[key] = k
    table.insert(arr, v)
  end
  return arr
end

-- Deep copy of table
-- Only values, no metatables
function deepcopy(orig, keys)
    local orig_type = type(orig)
    local copy
    
    local hkeys = {}
    if keys then 
      for i,v in ipairs(keys) do hkeys[v]=true end
    end
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            if not keys or hkeys[orig_key] then 
              copy[orig_key] = deepcopy(orig_value)
            end
        end
        -- setmetatable(copy, deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

-- Internal functions

function recordsEqual(t1,t2)
  local ty1 = type(t1)
  local ty2 = type(t2)
  if ty1 ~= ty2 then return false end
  if ty1 ~= 'table' then return t1 == t2 end

  for k1,v1 in pairs(t1) do
    if v1 ~= t2[k1] then return false end
  end
  return true
end

function compareRecords(keys,rec1, rec2)
  local key = 1
  local diff = 0
  for key in pairs(keys) do
     local kk = keys[key]
     if rec1[kk] == nil or rec2[kk] == nil then
        print("key=", key, kk, "rec1[key] = ", rec1[kk], " rec2[key]=", rec2[kk])
        print("rec1 = ", rec1)
        for k,v in pairs(rec1) do print(k, "->", v) end
        print("rec2 = ", rec2)
        for k,v in pairs(rec2) do print(k, "->", v) end
     end
    if rec1[kk] ~= rec2[kk] then
       if rec1[kk] < rec2[kk] then
          diff = -1
       else
          diff = 1
       end
--       print("key=", key, kk, "rec1[kk] = ", rec1[kk], " rec2[kk]=", rec2[kk], "diff=", diff)
	  break
    end
    key = key + 1
  end
--  print ("return ", diff, key)
  return diff, key
end

function hash_key(x)
  if type(x) == "table" then 
    return table.concat(x,',')
  else
    return x
  end
end

function concatRecords(rec1, rec2, override)
  local rec = {}
  for k,v in pairs(rec1) do rec[k] = v end
  if rec2 then  
    for k,v in pairs(rec2) do 
      if override or rec[k]==nil then 
        rec[k] = v 
      end 
    end
  else
    error ("Unexpected nil")
  end
  return rec
end

------------------------------------------------------------------------

local AsofJoin = Iter:new()

function AsofJoin:next()

   if self.outer == nil then
    error("self.outer == nil") 
   end  
   
   local outer = self.outer:next()
   if not outer then
      return nil
   end
   self.inner:reset()
   local prevInner = nil
   while true do
      local inner = self.inner:next()
      if not inner then
         if prevInner then
 --           print ("Concat: outer = ", outer.sym, outer.time, " prevInner = ", prevInner.sym, prevInner.time)
            return concatRecords(outer, prevInner)
         else
            return outer
         end
      end

      local diff, diffKey = compareRecords(self.keys, outer, inner)
--      print("compare ", self.keys[1], outer.sym, outer.time, inner.sym, inner.time, " return ", diff)
      if diff == 0 then
         if outer[self.t] == nil or inner[self.t] == nil then
          error ("Nil value for record") 
         end 
         if outer[self.t] > inner[self.t] then
            prevInner = inner
         else
            --            print ("concat ", outer, prevInner)
            if prevInner then 
              return concatRecords(outer, prevInner)
            else 
              return outer
            end
          end
      end               
   end
end

function AsofJoin:reset()
  self.outer:reset()
end

function AsofJoin:close()
  if self.outer then
    self.outer:close()
    self.inner:close()
	self.outer = nil
	self.inner = nil
  end
end

-- ASOF join
-- Parameters: 
--   keys: array of join keys
--   t1: outer iterator (ordered by time - last key in keys)
--   t2: inner iterator (ordered by time - last key in keys)
-- Returns: iterator
-- Example: aj({'sym','date'},a,b)
function aj(keys,t1,t2)
   if t1 == nil or t2 == nil then
    error("aj: table should not be nil")
   end
   local key = {}
   for i = 1, #keys-1 do table.insert(key, keys[i]) end   
   return AsofJoin:new({keys=key, t=keys[#keys],  outer=t1, inner=t2})
end

-- ASOF join for column based arrays
--   keys: array of join keys
--   t1: outer iterator (ordered by time - last key in keys)
--   t2: inner iterator (ordered by time - last key in keys)
-- Returns: column based array
-- Example: aj({'sym','date'},a,b)
function ajc(keys, tc1, tc2)
   if t1 == nil or t2 == nil then
    error("aj: table should not be nil")
   end
   local key = {}   
   local ret
   
   return ret
end
------------------------------------------------------------------------

local WindowJoin = Iter:new()

function WindowJoin:applyAgg(rec)
    for i,agg in ipairs(self.aggregates) do
        local aggFunc = agg[1]
        local aggCol = agg[2]
        local alias = agg[3] or aggCol              
        rec[alias] = aggFunc(rec[alias], self.innerRecord[aggCol])
--        print ("Applied agg to ", alias, "=", rec[alias], "add value=", self.innerRecord[aggCol])
    end          
end

function WindowJoin:next()
   local outer = self.outer:next()
   if not outer or self.i > #self.w[1] then 
      --print("Return nil, outer=", outer, " i=", self.i, "#w=", #self.w[1])
      return nil
   end

   local ret = outer

   while true do      
      if not self.innerRecord then 
        --print ("No inner record. Return outer")
        return ret
      end      
      if self.cursym ~= self.innerRecord[self.sym] then
        -- new symbol - rewind interval and return aggregate
        self.i = 1
        self.cursym = self.self.innerRecord[self.sym]
        self.interval = {start=self.w[self.i][1], finish=self.w[self.i][2]}
        --print("Return result. New sym=", self.cursym, "New interval = ", self.interval.start, self.interval.finish, "i = ", self.i)
        return ret
        -- continue         
      elseif self.innerRecord[self.time] >= self.interval.finish then
          -- new interval - make a switch and return
          
          if self.innerRecord[self.time] == self.interval.finish then
              self:applyAgg(ret)
          end 
                    
          self.i = self.i + 1
          if self.i <= #self.w[1] then            
            self.interval = {start=self.w[1][self.i], finish=self.w[2][self.i]}
          else
            self.interval = {start=nil, finish=nil}
          end 
          
--          self.innerRecord = self.inner:next()
--          print("Return result. New interval = ", self.interval.start, self.interval.finish, "i = ", self.i)
          return ret
      else
          if self.innerRecord[self.time] >= self.interval.start then
            self:applyAgg(ret)
          end
          self.innerRecord = self.inner:next()          
      end
   end
end

local comment = [[
function WindowJoin:next()
  local nextOuter = true
  local acc = {}
  while true do
    if nextOuter then
      self.outerRecord = self.outer:next()
      if not self.outerRecord then 
        return nil
      end
      acc = { count = 0 }
    end
    if self.innerRecord then
      local diff, diffKeyNo = compareRecords(self.on, self.outerRecord, self.innerRecord)
      if diffKeyNo >= table.getn(self.on)-1 and diff >= 0 then
        local innerMatch = self.innerRecord
        while true do  
          acc.count = acc.count + 1
          for i,agg in ipairs(self.aggregates) do
            local aggFunc = agg[1]
            local aggCol = agg[2]
            local alias = agg[3] or aggCol
            acc[alias] = aggFunc(acc[alias], innerMatch[aggCol])
          end
          self.innerRecord = self.inner:next()
          if not self.innerRecord then 
            break
          end
          diff, diffKeyNo = compareRecords(self.on, self.outerRecord, self.innerRecord)
          if diffKeyNo < table.getn(self.on)-1 or diff < 0 then
            break
          end
          innerMatch = self.innerRecord
        end
        for i,agg in ipairs(self.aggregates) do
          local aggFunc = agg[1]
          if aggFunc == avg then 
            local aggCol = agg[2]
            local alias = agg[3] or aggCol
            acc[alias] = acc[alias] / acc.count
          end
        end
        return concatRecords(concatRecords(self.outerRecord, self.innerRecord), acc)
      end
    end
    if diff < 0 then
      -- outer join
      return self.outerRecord
    end
    self.innerRecord = self.inner:next()
    nextOuter = false
  end
end
]]

function WindowJoin:reset()
  self.inner:reset()
  self.outer:reset()
  self.innerRecord = nil
end

function WindowJoin:close()
  if self.outer then
    self.outer:close()
    self.inner:close()
	self.outer = nil
	self.inner = nil
  end
end

-- Window join: ASOF join with aggregation
-- Parameters:
--   w: array of join keys
--   t1: outer iterator (ordered by join keys)
--   t2: inner iterator (ordered by join keys)
--   keys: array of koin keys. Should have 2 elements - 'symbol' and 'time' columns, according to it's KDB's prototype 
--   aggs: array of lists of pairs/triples with aggregate function, aggregated column and optional result alias (if not specified, then aggregated column name will be used as result)
-- Returns: iterator 
function wj(w,t1,t2,keys,aggs)
  return WindowJoin:new{w=w, outer=t1, inner=t2, keys=keys, time=keys[2], outerRecord = nil, innerRecord = t2:next(), aggregates = aggs, interval={start=w[1][1], finish=w[2][1]}, i=1}
end

------------------------------------------------------------------------

-- Aggregate returning sum of values in the group
function sum(acc, val)
  if val == nil then return acc elseif acc == nil then return val else return acc + val end
end

-- Aggregate returning aveage of values in the group (will be devided by group size at end of group)
function avg(acc, val)
  if val == nil then 
    return {sum=acc.sum, count=acc.count} 
  elseif acc == nil then 
    return {sum=val, count=1}
  else 
   return {sum=acc.sum + val, count=acc.count+1} 
  end
end

-- Aggregate returning number of non-null values of the column
function count(acc, val)
  if val == nil then return acc elseif acc == nil then return 0 else return acc + 1 end
end

-- Aggregate returning minimul value in the group
function min(acc, val)
  if val == nil then return acc elseif acc == nil or acc > val then return val else return acc end
end

-- Aggregate returning maximul value in the group
function max(acc, val)
  if val == nil then return acc elseif acc == nil or acc < val then return val else return acc end
end

-- Aggregate returning first value in the group
function first(acc, val)
  if acc == nil then return val else return acc end
end

-- Aggregate returning last value in the group
function last(acc, val)
  return val
end

-- Accumulate all the values in the table
function acc(tacc, val)
  if tacc == nil then 
    return {val} 
  else
    table.insert(tacc, val)
    return tacc
  end
end
     

------------------------------------------------------------------------

local RangeWindowLag = Iter:new()

function RangeWindowLag:next()
  local curr = self.source:next()
  if not curr then
    return nil
  end
  local prev = nil
  while true do
    local a = self.history[curr[self.key]]
	if not a then
       a = {head=1,tail=1,data={}}
    else 
	   while a.tail < a.head and a.data[a.tail][self.timestamp] + self.delta <= curr[self.timestamp] do
	     prev = a.data[a.tail]
         table.remove(a.data, a.tail)
         a.tail = a.tail + 1
       end       
    end
  end
  a.data[a.head] = curr
  a.head = a.head + 1
  return curr,prev
end

function RangeWindowLag:reset()
  self.source:reset()
  self.history = {}
end

-- Return pairs of current and preceeding records for each symbol 
-- Parameters:
--   self: input iterator sorted by tm
--   sym: name of symbol column
--   ts: name of timestamp column
--   dt: time delta
-- Returns: iterator
-- Example: cursor:range_lag(cursor, 'sym', 'date', 1000)
function Iter:range_lag(sym, ts, dt)
  return RangeWindowLag:new({source = self, key = sym, timestamp = ts, delta = dt, history = {}})
end

------------------------------------------------------------------------

local SortedRangeWindowLag = Iter:new()

function SortedRangeWindowLag:next()
  local curr = self.source:next()
  if not curr then
    return nil
  end
  local prev = nil
  if self.tail ~= self.head and self.history[self.tail][self.key] ~= curr[self.key] then
    self.history = {}
	self.head = 1
	self.tail = 1
  else
    while self.tail < self.head and self.history[self.tail][self.timestamp] + self.delta <= curr[self.timestamp] do
      prev = self.history[self.tail]
      table.remove(self.history, self.tail)
      self.tail = sefl.tail + 1
    end
  end
  self.history[self.head] = curr
  self.head = self.head + 1
  return curr,prev
end

function SortedRangeWindowLag:reset()
  self.source:reset()
  self.head = 1
  self.tail = 1
  self.history = {}
end

-- Return pairs of current and preceeding records for each symbol 
-- Parameters:
--   self: input iterator sorted by sym,tm
--   sym: name of symbol column
--   ts: name of timestamp column
--   dt: time delta
-- Returns: iterator
-- Example: cursor:inc_range_lag('sym', 'date', 1000)
function Iter:inc_range_lag(sym, ts, dt)
  return SortedRangeWindowLag:new({source = self, key = sym, timestamp = ts, delta = dt, history = {}, head = 1, tail = 1})
end

------------------------------------------------------------------------

local RowsWindowLag = Iter:new()

function RowsWindowLag:next()
  local curr = self.source:next()
  if not curr then
    return nil
  end
  local prev = nil
  while true do
    local a = self.history[curr[self.key]]
	if not a then
      a = {head=1,tail=1,data={}}
    elseif a.tail + self.size <= a.head then
	  prev = a.data[a.tail]
      table.remove(a.data, a.tail)
      a.tail = a.tail + 1
    end
  end
  a.data[a.head] = curr
  a.head = a.head + 1
  return curr,prev
end

function RowsWindowLag:reset()
  self.source:reset()
  self.history = {}
end

-- Return pairs of current and preceeding records for each symbol 
-- Parameters:
--   self: input iterator sorted by tm
--   sym: name of symbol column
--   sz: window size
-- Returns: iterator
-- Example: cursor:rows_lag(cursor, 'sym', 10)
function Iter:rows_lag(sym, sz)
  return RowsWindowLag:new({source = self, key = sym, size = sz, history = {}})
end

------------------------------------------------------------------------

local SortedRowsWindowLag = Iter:new()

function SortedRowsWindowLag:next()
  local curr = self.source:next()
  if not curr then
    return nil
  end
  local prev = nil
  if self.tail ~= self.head and self.history[self.tail][self.key] ~= curr[self.key] then
    self.history = {}
	self.head = 1
	self.tail = 1
  elseif self.tail + self.size <= self.head then
    prev = self.history[self.tail]
    table.remove(self.history, self.tail)
    self.tail = self.tail + 1
  end
  self.history[self.head] = curr
  self.head = self.head + 1
  return curr,prev
end

function SortedRowsWindowLag:reset()
  self.source:reset()
  self.head = 1
  self.tail = 1
  self.history = {}
end

-- Return pairs of current and preceeding records for each symbol 
-- Parameters:
--   self: input iterator sorted by sym,tm
--   sym: name of symbol column
--   sz: window size
-- Returns: iterator
-- Example: cursor:inc_rows_lag('sym', 10)
function Iter:inc_rows_lag(sym, sz)
  return SortedRowsWindowLag:new({source = self, key = sym, size = sz, history = {}, head = 1, tail = 1})
end

------------------------------------------------------------------------

local Filter = Iter:new()

function Filter:next()
  local elem = self.source:next()
  while elem and not self.predicate(elem) do
    elem = self.source:next()
  end
  return elem
end

function Filter:reset()
  self.source:reset()
end

-- Filter input data using specified predicate function
-- Parameters:
--   cond: predicate function
-- Returns: iterator
-- Example: cursor:filter(function(r) return e.volume > 100 end)  
function Iter:filter(cond) 
  return Filter:new({source = self, predicate = cond})
end

------------------------------------------------------------------------

local Project = Iter:new()

function Project:next()
  local elem = self.source:next()
  if elem then
    elem = self.mapFunc(elem)
  end
  return elem
end

function Project:reset()
  self.source:reset()
end

-- Apply function to each record of input iterator
-- Parameters:
--   map: applied function
-- Returns: iterator
-- Example: cursor:project(function(r) return {r.sym, r.date, r.open, r.close} end)   
function Iter:project(map) 
  return Project:new({source = self, mapFunc = map})
end

-- Select specified columns
-- Parameters:
--   columns: array with column names
-- Returns: iterator
-- Example: cursor:select({'col1', 'col2', ... 'colN'})
function Iter:select(columns)
  return Project:new({source = self, mapFunc = function (input) local output = {} for i,k in ipairs(columns) do output[k] = input[k] end return output end})
end


-- Rename specified columns
--   Parameters:
--  columns: map of column names to rename. Each item is a pair {'oldName', 'newName'}
--  Columns not in the array will be left untouched
-- Returns: iterator
-- Example: cursor:update({{'oldA', 'A'}, {'oldB', 'B'}}) 
------------------------------------------------------------------------
function Iter:update(columns)
  return Project:new({source = self, mapFunc = function (input) 
    local output = {} 
    for k,v in pairs(input) do
      if columns[k] == nil then         
        output[k] = v
      else
        if type(columns[k]) == 'table' then
          output[columns[k][1]] = columns[k][2](v, input)          
        else
          output[columns[k]] = v
        end  
      end 
    end
    return output
  end
  })        
end


-- Map reduce 
-- Parameters:
--   map: map functions - extracting group key from record
--   reduce: reduce function - aggregating record with same key value
-- Returns: table  
-- Example: cursor:mapReduce(function(x) return x.sym end, sum)
function Iter:mapReduce(map, reduce)
  local result = {}
  while true do 
    local elem = self:next()
    if not elem then
      self:close() 
      return result 
    end
    local k = hash_key(map(elem))
    local v = result[k]
    result[k] = reduce(v, elem)
  end
end
     
------------------------------------------------------------------------

-- Perform aggregation with group by using hash table 
-- Parameters:
--   group_by: group_by column
--   aggs: list of pairs/triples with aggregate function, aggregated column and optional result alias (if not specified, then aggregated column name will be used as result)
-- Returns: table
-- Order of output records is undefined
-- Example: cursor:aggregate('sym', {{max,'bid'}, {min,'ask'}})
function Iter:groupBy(group_by, aggs)

  local function iter(table, keys, level, applier)
      for k,v in pairs(table) do
        local keyname = group_by[1 + #group_by - level]
        keys[keyname]=k 
        if level == 1 then          
          applier(v, keys) -- keys {key1=val1,... keyN=valN}
        else           
          iter(v, keys, level-1, applier)
        end
      end  
  end

  local result = {}
  local tbl = {}
  
  if type(group_by) == 'string' then
    group_by = {group_by}
  end

  local n = #group_by
  
  while true do
    local t = tbl   
    local elem = self:next()
    if not elem then

      keys = {}    

      iter(tbl, keys, n, function(out_rec, keys)
        local row = deepcopy(keys)
                          
        for i,agg in ipairs(aggs) do        
        
          local aggFunc = agg[1]
          local aggCol = agg[2]
          local alias = agg[3] or aggCol
          v = out_rec[aggCol]
          if aggFunc == avg then                                   
            row[alias] = v.sum / v.count
          else
            row[alias] = v
          end
        end
        table.insert(result, row)             
      end)
                
      self:close() 
      return result 
    end
    
    for i=1,n-1 do
      local k = elem[group_by[i]] or 0      
      if not t[k] then
        t[k] = {}
      end
      t = t[k]
    end
    
    local key = elem[group_by[n]]
    local acc = t[key]
    local new = false
    if not acc then
      acc = { count = 0 }
      new = true
    end
    acc.count = acc.count + 1
    for i,agg in ipairs(aggs) do
      local aggFunc = agg[1]
      local aggCol = agg[2]
      local alias = agg[3] or aggCol
      acc[alias] = aggFunc(acc[alias], elem[aggCol])
    end
    if new then 
      t[key] = acc
    end
  end
end

------------------------------------------------------------------------

local Aggregate = Iter:new()

function Aggregate:next()
  local curr = self.curr
  if not curr then
    return nil
  end
  local group = {}
  local group_by = self.group_by
  for i,key in ipairs(group_by) do
    group[key] = curr[key]
  end
  local result = { count = 0 }
  repeat
    local equal = true
    for i,key in ipairs(group_by) do
      if group[key] ~= curr[key] then
        equal = false
        break
      end
    end
    if not equal then
	  break
    end
	result.count = result.count + 1
	for i,agg in ipairs(self.aggregates) do
      local aggFunc = agg[1]
	  local aggCol = agg[2]
      local alias = agg[3] or aggCol
	  result[alias] = aggFunc(result[alias], curr[aggCol])
    end
	curr = self.source:next()
  until not curr

  for i,agg in ipairs(self.aggregates) do
    local aggFunc = agg[1]
    if aggFunc == avg then
      local aggCol = agg[2]
      local alias = agg[3] or aggCol
      result[alias] = result[alias] / result.count
    end
  end
  self.curr = curr
  return concatRecords(group, result)
end

function Aggregate:reset()
  self.source:reset()
  self.curr = self.source:next()
end  

-- Perform incremental aggregation with group by
-- Parameters:
--   group_by: list of grouping column names
--   aggs: list of pairs/triples with aggregate function, aggregated column and optional result alias (if not specified, then aggregated column name will be used as result)
-- Returns: iterator
-- Example: cursor:inc_aggregate({'sym'}, {{max,'bid'}, {min,'ask'}})
function Iter:inc_aggregate(gby, aggs)
  return Aggregate:new({source = self, group_by = gby, aggregates = aggs, curr = self:next()})
end

------------------------------------------------------------------------

-- Perform aggregation of the whole input result set using hash table 
-- Parameters:
--   aggs: list of pairs/triples with aggregate function, aggregated column and optional result alias (if not specified, then aggregated column name will be used as result)
-- Returns: table
-- Example: cursor:grand_aggregate({{max,'bid'}, {min,'ask'}})
function Iter:grand_aggregate(aggs)
  local result = { count = 0 }
  while true do 
    local elem = self:next()
    if not elem then
      for i,agg in ipairs(aggs) do
        local aggFunc = agg[1]
        if aggFunc == avg then
          local aggCol = agg[2]
          local alias = agg[3] or aggCol
          result[alias] = result[alias] / result.count
          break
        end
      end
	  self:close() 
      return result 
    end
	result.count = result.count + 1
	for i,agg in ipairs(aggs) do
      local aggFunc = agg[1]
	  local aggCol = agg[2]
      local alias = agg[3] or aggCol
	  result[alias] = aggFunc(result[alias], elem[aggCol])
    end
  end
end

------------------------------------------------------------------------

local UpdateBy = Iter:new()

function UpdateBy:next()
  if not self.groups then
    self.groups = {} 
    while true do 
      local rec = self.source:next()
      if not rec then 
	    break
      end
      local key = rec[self.group_by]
      local acc = self.groups[key]
  	  local new = false
  	  if not acc then
        acc = {count = 0}
        new = true
      end
	  acc.count = acc.count + 1
  	  for i,agg in ipairs(self.aggregates) do
        local aggFunc = agg[1]
  	    local aggCol = agg[2]
	    local alias = agg[3] or aggCol
		acc[alias] = aggFunc(acc[alias], rec[aggCol])
      end
  	  if new then 
        self.groups[key] = acc
  	  end
    end

    for i,agg in ipairs(self.aggregates) do
      local aggFunc = agg[1]
	  if aggFunc == avg then
  	    local aggCol = agg[2]
	    local alias = agg[3] or aggCol
        for k,v in pairs(self.groups) do 
          v[alias] = v[alias] / v.count
        end
      end
    end
	self.source:reset()
  end
  local rec = self.source:next()
  if rec then
    return concatRecord(rec, self.groups[rec[self.group_by]])
  else
    return nil
  end
end

function UpdateBy:reset()
  self.source:reset()
  self.groups = nil
end

-- Perform aggregation with group by and the result is assigned to all records in the group. 
-- Parameters:
--   gby: group_by column (only one grouping key is accepted now)
--   aggs: list of pairs/triples with aggregate function, aggregated column and optional result alias (if not specified, then aggregated column name will be used as result)
-- Returns: iterator
-- Example: cursor:updateby('sym', {{max,'bid'}, {min,'ask'}})
function Iter:updateby(gby, aggs)
  return UpdateBy:new({source = self, group_by = gby, aggregates = aggs})
end

------------------------------------------------------------------------

-- Materialized version of udateby.
-- Perform aggregation with group by and the result is assigned to all records in the group. 
-- Parameters:
--   input: input iterator
--   group_by: group_by column (only one grouping key is accepted now)
--   aggs: list of pairs/triples with aggregate function, aggregated column and optional result alias (if not specified, then aggregated column name will be used as result)
-- Returns: table
-- Example: updateby(cursor, 'sym', {{max,'bid'}, {min,'ask'}})
function updateby(input, group_by, aggs)
  local result = {}
  local groups = {} 
  while true do 
    local rec = input:next()
    if not rec then 
	  input.close()
  	  for i,agg in ipairs(aggs) do
        local aggFunc = agg[1]
 		if aggFunc == avg then
   	      local aggCol = agg[2]
          local alias = agg[3] or aggCol
          for k,v in pairs(groups) do
            v[alias] = v[alias] / v.count
          end
        end
      end
      for i,rec in ipairs(result) do
         result[i] = concatRecord(rec,groups[rec[group_by]])
	  end 
      return result 
    end
    local key = rec[group_by]
    local acc = groups[key]
	local new = false
	if not acc then
      acc = { count = 0}
      new = true
    end
	acc.count = acc.count + 1	
	for i,agg in ipairs(aggs) do
      local aggFunc = agg[1]
	  local aggCol = agg[2]
      local alias = agg[3] or aggCol
	  acc[alias] = aggFunc(acc[alias], rec[aggCol])
    end
	if new then 
  	  groups[key] = acc
	end
	table.insert(result, rec)
  end
end
    
------------------------------------------------------------------------

local TableIterator = Iter:new()
  
function TableIterator:next()
  local k, v = next(self.source, self.key)
  self.key = k
  if k then     
     return deepcopy(v, self.keys)
  else 
    return nil
  end
end

function TableIterator:reset()
  self.key = nil
end

function TableIterator:close()
end


-- Iterator for Lua table. Can be used to include result mapReduce() and aggreate() into the pipe
-- Parameters:
--   tbl: iterated table
-- Returns: iterator
-- Example: iterator(mapReduce(cursor, function(x) return x.sym end, sum))
function iterator(tbl, keys) 
  return TableIterator:new({source = tbl, keys=keys, key = nil})
end

------------------------------------------------------------------------

-- Sort table by specified columns
-- Parameters:
--   tbl: input table
--   by: array with order by column names
-- Returns: table
-- Example: sort(t, {'sym','date'})
function sort(tbl, by)
  table.sort(tbl, function (a,b) for i,k in ipairs(by) do if a[k] < b[k] then return true elseif a[k] > b[k] then return false end end end)
  return tbl
end

-- Sort input by specified columns in ascent order
-- Parameters:
--   by: array with order by column names
-- Returns: table
-- Example: cursor:xasc({'sym','date'})
function Iter:xasc(by)
  table.sort(self:materialize(), function (a,b) for i,k in ipairs(by) do if a[k] < b[k] then return true elseif a[k] > b[k] then return false end end end)
  return tbl
end

-- Sort input by specified columns in descent order
-- Parameters:
--   by: array with order by column names
-- Returns: table
-- Example: cursor:xdesc({'sym','date'})
function Iter:xdesc(by)
  table.sort(self:materialize(), function (a,b) for i,k in ipairs(by) do 
    if a[k] > b[k] then return true elseif a[k] < b[k] then return false end 
  end end)
  return tbl
end

------------------------------------------------------------------------

local LeftOuterJoin = Iter:new()

function LeftOuterJoin:keyvals(rec)
  if rec == nil then
    return nil
  end
  if self.keys == nil then 
    return rec[self.key]
  else 
    ret = {}
    for k,v in ipairs(self.keys) do
      table.insert(ret, rec[v])
    end 
    return ret
  end
end

function LeftOuterJoin:next()

  local outer_rec = self.outer:next()
  local inner_rec = nil
  if not outer_rec then
    return nil
  end
  local outer_key = self:keyvals(outer_rec)
  if self.keys == nil then
    -- Single key, we can use a map
    if not self.map then
     
      self.map = {}
      while true do 
        local inner_rec = self.inner:next()
        if not inner_rec then
          break
        end
        local inner_key = self:keyvals(inner_rec)
        if inner_key then
          self.map[inner_key] = inner_rec
        else
          print ("inner_key=", inner_key, "rec=", inner_rec)          
        end
      end
    end    
    inner_rec = self.map[outer_key]
  else
    -- Multiple keys, use sequential scan
    self.inner:reset()
    while true do 
      inner_rec = self.inner:next()
      if not inner_rec then
        break
      end

      if compareRecords(self.keys, inner_rec, outer_rec) == 0 then
        break
      end
    end                  
  end
  if inner_rec then
    return concatRecords(outer_rec, inner_rec, true)
  else
    return outer_rec
  end
end

function LeftOuterJoin:reset()
  self.outer:reset()
  self.inner:reset()
  self.map = nil
end

function LeftOuterJoin:close()
  if self.outer then
    self.outer:close()
    self.inner:close()
	self.outer = nil
	self.inner = nil
  end
end

-- Left outer join. Only one-top-one join with single join key is currently supported.
-- Parameters:
--   left: outer iterator
--   right: innect iterator
--   left_key: column name of outer join key
--   right_key: column name of innner join key
-- Returns: iterator
-- Example: lj(a, b, 'id_a', 'id_b')
function lj(left, right, keys)
  if not keys then
    error ("no keys provided")
  end
  if type(keys) == 'table' then
    return LeftOuterJoin:new({outer = left, inner = right, keys=keys})
  else
    return LeftOuterJoin:new({outer = left, inner = right, key = keys})
  end
end

------------------------------------------------------------------------

local LeftJoin = Iter:new()

function LeftJoin:next() 
  if not self.map then
    self.map = {}
    while true do 
      local inner_rec = self.inner:next()
      if not inner_rec then
	    break
      end
	  local inner_key = hash_key(self.inner_key(inner_rec))
      self.map[inner_key] = inner_rec
    end
  end
  while true do
    local outer_rec = self.outer:next()
    if not outer then
      return nil
    end
    local outer_key = hash_key(self.outer_key(outer_rec))
    local inner_rec = self.map[outer_key]
    if inner_rec then
      return concatRecords(outer_rec, inner_rec)
    elseif self.outer_join then
      return outer_rec
    end
  end
end

function LeftJoin:reset()
  self.outer:reset()
  self.inner:reset()
  self.map = nil
end

function LeftJoin:close()
  if self.outer then
    self.outer:close()
    self.inner:close()
	self.outer = nil
	self.inner = nil
  end
end

-- Left join. Only one-top-one join join is currently supported.
-- Parameters:
--   left: outer iterator
--   right: innect iterator
--   left_key: function returning outer key
--   right_key: function returning inner key
--   outer_join: whether join is outer
-- Returns: iterator
-- Example: join(a, b, function(a) return a.id end, function(b) return b.id end, false)
function join(left, right, left_key, right_key, outer)
  return LeftJoin:new({outer = left, inner = right, outer_key = left_key, inner_key = right_key, outer_join = outer})
end

------------------------------------------------------------------------

local RangeWindow = Iter:new()

function RangeWindow:next()
  local curr = self.source:next()
  if not curr then
    return nil
  end
  while true do
    local a = self.history[curr[self.key]]
	if not a then
      a = {head=1,tail=1,data={},acc=nil}
    else 
	  while a.tail < a.head and a.data[a.tail][self.timestamp] + self.delta <= curr[self.timestamp] do
	    a.acc = self.shrink(a.acc, a.data[a.tail])
        table.remove(a.data, a.tail)
        a.tail = a.tail + 1
      end
    end
  end
  a.data[a.head] = curr
  a.acc = self.extend(a.acc, curr)
  a.head = a.head + 1
  return a.acc,curr
end

function RangeWindow:reset()
  self.source:reset()
  self.history = {}
end

-- Calculate range window function
-- Parameters:
--   self: input iterator sorted by tm
--   sym: name of symbol column
--   ts: name of timestamp column
--   dt: time delta
--   extend: function for adding element to the window: (state,value)->state
--   shrink: function for extending window: (state,value)->state
-- Returns: iterator of pairs <aggregate,current_value>
-- Example: cursor:range_window('sym', 'date', 1000, 
--            function(acc,x) if not acc then acc = {sum=x, avg=x, count=1} 
--			                  else acc.sum = acc.sum + x; acc.count = acc.count + 1; acc.avg = acc.sum / acc.count 
--			  			      end
--			  end,
--            function(acc,x) acc.sum = acc.sum - x; acc.count = acc.count - 1; acc.avg = acc.sum / acc.count end)
function Iter:range_window(sym, ts, dt, extend_func, shrink_func)
  return RangeWindow:new({source = self, key = sym, timestamp = ts, delta = dt, extend = extend_func, shrink = shrink_func, history = {}})
end

------------------------------------------------------------------------

local SortedRangeWindow = Iter:new()

function SortedRangeWindow:next()
  local curr = self.source:next()
  if not curr then
    return nil
  end
  local prev = nil
  if self.tail ~= self.head and self.history[self.tail][self.key] ~= curr[self.key] then
    self.history = {}
	self.head = 1
	self.tail = 1
	self.acc = nill
  else
    while self.tail < self.head and self.history[self.tail][self.timestamp] + self.delta <= curr[self.timestamp] do
      self.acc = self.shrink(self.acc, self.history[self.tail])
      table.remove(self.history, self.tail)
      self.tail = sefl.tail + 1
    end
  end
  self.history[self.head] = curr
  self.acc = self.extend(self.acc, curr)
  self.head = self.head + 1
  return self.acc,curr
end

function SortedRangeWindow:reset()
  self.source:reset()
  self.head = 1
  self.tail = 1
  self.history = {}
  self.acc = nil
end

-- Calculate range window function
-- Parameters:
--   self: input iterator sorted by sym,tm
--   sym: name of symbol column
--   ts: name of timestamp column
--   dt: time delta
--   extend: function for adding element to the window: (state,value)->state
--   shrink: function for extending window: (state,value)->state
-- Returns: iterator of pairs <aggregate,current_value>
-- Example: cursor:inc_range_window('sym', 'date', 1000, 
--            function(acc,x) if not acc then acc = {sum=x, avg=x, count=1} 
--			                  else acc.sum = acc.sum + x; acc.count = acc.count + 1; acc.avg = acc.sum / acc.count 
--			 			      end
--		      end,
--            function(acc,x) acc.sum = acc.sum - x; acc.count = acc.count - 1; acc.avg = acc.sum / acc.count end)
function Iter:inc_range_window(sym, ts, dt, extend_func, shrink_func)
  return SortedRangeWindow:new({source = self, key = sym, timestamp = ts, delta = dt, extend = extend_func, shrink = shrink_func, history = {}, head = 1, tail = 1})
end

------------------------------------------------------------------------

local RowsWindow = Iter:new()

function RowsWindow:next()
  local curr = self.source:next()
  if not curr then
    return nil
  end
  while true do
    local a = self.history[curr[self.key]]
	if not a then
      a = {head=1,tail=1,data={},acc=nil}
    elseif a.tail + self.size <= a.head then
      a.acc = self.shrink(a.acc, a.data[a.tail])
      table.remove(a.data, a.tail)
      a.tail = a.tail + 1
    end
  end
  a.data[a.head] = curr
  a.acc = self.extend(a.acc, curr)
  a.head = a.head + 1
  return a.acc,curr
end

function RowsWindow:reset()
  self.source:reset()
  self.history = {}
end

-- Calculate window function
-- Parameters:
--   self: input iterator
--   sym: name of symbol column
--   sz: window size
--   extend: function for adding element to the window: (state,value)->state
--   shrink: function for extending window: (state,value)->state
-- Returns: iterator of pairs <aggregate,current_value>
-- Example: cursor:rows_window('sym', 100, 
--            function(acc,x) if not acc then acc = {sum=x, avg=x, count=1} 
--			                  else acc.sum = acc.sum + x; acc.count = acc.count + 1; acc.avg = acc.sum / acc.count 
--			  			      end
--			  end,
--            function(acc,x) acc.sum = acc.sum - x; acc.count = acc.count - 1; acc.avg = acc.sum / acc.count end)
function Iter:rows_window(sym, sz, extend_func, shrink_func)
  return RowsWindow:new({source = self, key = sym, size = sz, extend = extend_func, shrink = shrink_func, history = {}})
end

------------------------------------------------------------------------

local SortedRowsWindow = Iter:new()

function SortedRowsWindow:next()
  local curr = self.source:next()
  if not curr then
    return nil
  end
  local prev = nil
  if self.tail ~= self.head and self.history[self.tail][self.key] ~= curr[self.key] then
    self.history = {}
	self.head = 1
	self.tail = 1
	self.acc = nill
  elseif self.tail + self.size <= self.head then
    self.acc = self.shrink(self.acc, self.history[self.tail])
    table.remove(self.history, self.tail)
    self.tail = sefl.tail + 1
  end
  self.history[self.head] = curr
  self.acc = self.extend(self.acc, curr)
  self.head = self.head + 1
  return self.acc,curr
end

function SortedRowsWindow:reset()
  self.source:reset()
  self.head = 1
  self.tail = 1
  self.history = {}
  self.acc = nil
end

-- Calculate rows window function
-- Parameters:
--   self: input iterator sorted by sym
--   sym: name of symbol column
--   sz: window size
--   extend: function for adding element to the window: (state,value)->state
--   shrink: function for extending window: (state,value)->state
-- Returns: iterator of pairs <aggregate,current_value>
-- Example: cursor:inc_rows_window('sym', 10, 
--            function(acc,x) if not acc then acc = {sum=x, avg=x, count=1} 
--			                  else acc.sum = acc.sum + x; acc.count = acc.count + 1; acc.avg = acc.sum / acc.count 
--			 			      end
--		      end,
--            function(acc,x) acc.sum = acc.sum - x; acc.count = acc.count - 1; acc.avg = acc.sum / acc.count end)
function Iter:inc_rows_window(sym, sz, extend_func, shrink_func)
  return SortedRowsWindow:new({source = self, key = sym, size = sz, extend = extend_func, shrink = shrink_func, history = {}, head = 1, tail = 1})
end

------------------------------------------------------------------------

local Zip = Iter:new()

function Zip:next()
  local left = self.left:next()
  local right = self.right:next()
  if left and right then 
    return concatRecords(left, right)
  else 
    return nil
  end
end

function Zip:close()
  if self.left then
    self.left:close()
    self.right:close()
	self.left = nil
    self.right = nil
  end
end

function Zip:reset()
  self.left:reset()
  self.right:reset()
end

-- Combine two interators into one
-- Parameters:
--   left: left iterator
--   right: right iterator
-- Returns: iterator through combined records
-- Example: zip(a, b)
function zip(a,b)
  return Zip:new({left=a, right=b})
end

------------------------------------------------------------------------

local Concat = Iter:new()

function Concat:next()
  if not self.append then
    local elem = self.head:next()
    if elem then
      return elem
    end
    self.append = true
  end
  return self.tail:next()
end

function Concat:close()
  if self.head then
    self.head:close()
    self.tail:close()
	self.head = nil
    self.tail = nil
  end
end

function Concat:reset()
  self.head:reset()
  self.tail:reset()
  self.append = true
end

-- Concatenate two interators
-- Parameters:
--   head: head iterator
--   tail: tail iterator
-- Returns: iterator 
-- Example: concat(a, b)
function concat(a,b)
  return Concat:new({head=a, tail=b})
end

------------------------------------------------------------------------

-- Reduce input iterator using supplied function
-- Parameters:
--   accumulate: reduce function
--   state: initial state
-- Returns: reduced state
-- Example: cursor:reduce(function(state,rec) return state + rec.volume end, 0)
function Iter:reduce(accumulate, state)
  while true do
    local elem = self:next()
    if not elem then
	  self:close()
	  return state
    end
    state = accumulate(state, elem)
  end
end

------------------------------------------------------------------------

local FlatMap = Iter:new()

function FlatMap:next()
  local elem
  repeat
    if not self.inner then
      local source = self.source:next()
      if source == nil then return nil end
      self.inner = self.mapFunc(source)
    end
    elem = self.inner:next()
    if elem == nil then
      self.inner = nil
    end
  until not elem
  return elem
end

function FlatMap:reset()
  self.source:reset()
  self.inner = nil
end

-- Flat map: apply map function to each element of input sequence and iterate throw 
-- returned iterator
-- Parameters:
--   map: map function
-- Returns: iterator
-- Example: cursor:flatMap(function(rec) return con.query("select * from Trade where sym=%s"", rec.sym) end)
function Iter:flatMap(map)
    return FlatMap:new({source = self, mapFunc = map, inner = nil })
end

------------------------------------------------------------------------

local GroupBy = Iter:new()

function GroupBy:next()  
  if type(self.key) == 'table' then
    self.source:reset()
    local keyrec = 1
    while true do
      local rec = self.source:next()
      if compareRecords(self.keys,rec1,rec2) then end
    end  
  else
    while true do
    local rec = self.source:next()
    
    -- 
    
    end
    return ret
  
  end
end

function GroupBy:reset()
  self.source:reset()
end

-- groupBY
-- Parameters:
--   src: source for grouping
--   key: group by this column(s)
--   grpBy: list of grouping columns in form { {agg, "column"} }
--          where agg - aggregate function, and "column" is column for aggregation calculate
-- Returns: iterator 
-- Example: groupBy(src, 'sym', {''} 
function groupBy(src, key, grpBy)
  return  GroupBy:new{source=src, key=key, groupBy=grpBy}
end

