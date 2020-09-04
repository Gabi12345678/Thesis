-- Some example functions inspired by KDB Q language

create table t(stamp int, sym string, qty int);

insert into t values( [100101, 100103, 100104], ['msft', 'ibm', 'ge'], [100,200,150]);
    
create table q(stamp int, sym string, px int);

insert into q values( [100100, 100100,100100,100102], ['ibm', 'msft','msft', 'ibm'], [100, 99, 101, 98]);
    
create table restbl(stamp int, sym string, qty int, px int);
    
create function aj(keys array(string), tname string, qname string) returns restbl in 'lua' as '
    kdb = require ("kdb")

    print ("kdb = ", kdb)

    tit = con:query("select * from " .. tname)
    qit = con:query("select * from " .. qname)
    local res = aj(keys, tit, qit):materialize()
    return res
end';    

select * from aj(['sym', 'stamp'], 't', 'q');
    
