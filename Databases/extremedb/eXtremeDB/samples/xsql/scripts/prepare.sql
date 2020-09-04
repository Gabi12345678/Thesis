create table t(x integer primary key, y integer);
prepare lookup as select * from t where x=?::integer;
prepare lookup2 as select * from t where x=?::integer and y=?::integer;
insert into t values (1,10);
execute lookup(1);
explain execute lookup(1);
execute lookup2(1,10);
explain execute lookup2(1,10);
prepare selfjoin as select * from t as t1 left join t as t2 on t1.y=t2.y;
execute selfjoin();
explain execute selfjoin();
execute selfjoin();

