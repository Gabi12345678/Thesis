This is a sample for making aggregating functions in SQL, i.e. functions used in GROUP BY operator.

Please refer to documentation for the details.

To run this sample:

1. build UDF binary

  MCO_QUICK_BUILD=on make

2. Run XSQL and:
2.1 create tables

   create table trade(
    id varchar primary key,
    "Ccy" char(3),
    "PTR_neg1secs" double,
    "PTR_neg2secs" double,
    "PTR_neg3secs" double,
    usdnotional double
);

2.2 Create User Defined Function (UDF):

set implicit_aggregates=true;
    
create function bx_wt_avg(state array(double), ops array(double)) returns array(double) 
    as 'api-sql-09-functions-aggregate',  'bx_wt_avg';

2.3 Import data:

insert or update into trade
  select *
  from 
       foreign table(path='data.csv', skip=1) as trade;

2.4  run query which can use aggregating UDF:
SELECT
	Ccy AS NAME,
	bx_wt_avg ( ["PTR_neg1secs", usdnotional] ) AS "-1secs",
	bx_wt_avg ( ["PTR_neg2secs", usdnotional] ) AS "-2secs",
	bx_wt_avg ( ["PTR_neg3secs", usdnotional] ) AS "-3secs"
FROM
	trade
GROUP BY
	Ccy;


Steps in 2 can be run in a single command line:
    ../../../../../../target/bin/xsql -c xsql.cfg -f create.sql -f import.sql -f query.sql

or using provided shell script:
    ./run.sh

