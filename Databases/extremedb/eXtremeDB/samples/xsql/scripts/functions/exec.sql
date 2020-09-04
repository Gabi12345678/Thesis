create table t(i integer primary key, k string);
insert into t values ([1,2], ['hello', 'world']);   

create function execfn(i integer) returns string in 'lua' as '
    local ffi = require "ffi" 
    for row in con:query("select k from t where i=%i", ffi.new("int", i)):tuples() do    
        return row.k
    end
    return nil
end
';
    
select execfn(1);
    
