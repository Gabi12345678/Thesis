-- Show how to return dataset from UDF and use it in query

create table flatquotes(sym char(4), bid real, ask real);
    
create function myflatten() returns flatquotes in 'lua' as '
    local ret = {{sym = "IBM", bid=14.6, ask=15.0},
                 {sym = "IBM", bid=14.7, ask=15.2},
                 {sym = "IBM", bid=14.8, ask=15.3},
                 {sym = "AAPL", bid=10.0, ask=10.6},
                 {sym = "AAPL", bid=10.2, ask=10.7},
                 {sym = "AAPL", bid=10.3, ask=10.8}
    }
    return ret
end';


--select sym, sum(bid)/count(bid), sum(ask)/count(ask) from myflatten() group by sym;
select * from myflatten();
    
select sym, sum(bid)/count(*) as avgbid, sum(ask)/count(*) as avgask  from myflatten() group by sym;
