create table quotes(sym sequence(char(5)), ask sequence(real), bid sequence(real));
insert into quotes values('IBM', [15.0, 15.2, 15.3], [14.6, 14.7, 14.8]);    
insert into quotes values('AAPL', [10.0, 10.2, 10.3], [10.6, 10.7, 10.8]);    

    
-- This function shows how to pass sequence iterator into UDF and how to return a value    
create function seq_min_max_avg(inp sequence(real)) returns real in 'python' as '
    smin = inp.agg_min().next()
    inp.reset()
    smax = inp.agg_max().next()
    return (smin + smax) / 2
';

select sym, seq_min_max_avg(bid), seq_min_max_avg(ask) from quotes;


-- This function uses iseqn() to simultaneously iterate over 2 sequences    
create function avg_mid(bid sequence(real), ask sequence(real)) returns real in 'python' as '
    sum = 0
    count = 0
    for bv, av in zip(bid, ask):
        sum = sum + bv + av
        count = count + 1
    return sum / (2*count)
';


select sym, avg_mid(bid, ask) from quotes;
