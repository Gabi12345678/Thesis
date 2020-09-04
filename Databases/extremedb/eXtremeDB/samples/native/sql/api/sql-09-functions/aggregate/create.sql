create table trade(
    id varchar primary key,
    "Ccy" char(3),
    "PTR_neg1secs" double,
    "PTR_neg2secs" double,
    "PTR_neg3secs" double,
    usdnotional double
);

set implicit_aggregates=true;
    
create function bx_wt_avg(state array(double), ops array(double)) returns array(double) 
    as 'api-sql-09-functions-aggregate',  'bx_wt_avg';
    
    
