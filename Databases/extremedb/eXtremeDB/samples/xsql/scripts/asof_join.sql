create table t(tm time, sym char(5), qty real);
create table q(tm time, sym char(5), px real);

insert into t values ('10:01:01','msft',100),('10:01:03','ibm',200),('10:01:04','ge',150);
insert into q values ('10:01:00','ibm',100),('10:01:00','msft',99),('10:01:00','msft',101),('10:01:02','ibm',98);

explain select * from t asof outer join q on (t.sym=q.sym and t.tm >= q.tm);

select * from t asof outer join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:01 msft 100 101
--10:01:03 ibm  200 98
--10:01:04 ge   150
select * from t asof join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:01 msft 100 101
--10:01:03 ibm  200 98

delete from t;
delete from q;

insert into t values ('10:01:00','msft',100),('10:01:03','ibm',200),('10:01:04','ge',150);
insert into q values ('10:01:00','ibm',100),('10:01:02','msft',99),('10:01:03','msft',101),('10:01:02','ibm',98);

select * from t asof outer join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:00 msft 100   
--10:01:03 ibm  200 98
--10:01:04 ge   150   
select * from t asof join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:03 ibm  200 98

select * from t asof outer join q using (sym,tm);
--10:01:00 msft 100   
--10:01:03 ibm  200 98
--10:01:04 ge   150   
select * from t asof join q using (sym,tm);
--10:01:03 ibm  200 98

delete from t;
delete from q;

insert into t values ('10:01:01','msft',100),('10:01:03','ibm',200),('10:01:04','ge',150);
insert into q values ('10:01:00','ibm',100),('10:01:00','msft',99),('10:01:02','msft',101),('10:01:02','ibm',98);

select * from t asof outer join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:00 msft 100 99  
--10:01:03 ibm  200 98
--10:01:04 ge   150   
select * from t asof join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:00 msft 100 99  
--10:01:03 ibm  200 98

delete from t;
delete from q;

create index t_idx on t(sym,tm);
create index q_idx on q(sym,tm);

insert into t values ('10:01:01','msft',100),('10:01:03','ibm',200),('10:01:04','ge',150);
insert into q values ('10:01:00','ibm',100),('10:01:00','msft',99),('10:01:00','msft',101),('10:01:02','ibm',98);

explain select * from t asof outer join q on (t.sym=q.sym and t.tm >= q.tm);

select * from t asof outer join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:01 msft 100 101
--10:01:03 ibm  200 98
--10:01:04 ge   150
select * from t asof join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:01 msft 100 101
--10:01:03 ibm  200 98

delete from t;
delete from q;

insert into t values ('10:01:00','msft',100),('10:01:03','ibm',200),('10:01:04','ge',150);
insert into q values ('10:01:00','ibm',100),('10:01:02','msft',99),('10:01:03','msft',101),('10:01:02','ibm',98);

select * from t asof outer join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:00 msft 100   
--10:01:03 ibm  200 98
--10:01:04 ge   150   
select * from t asof join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:03 ibm  200 98

delete from t;
delete from q;

insert into t values ('10:01:01','msft',100),('10:01:03','ibm',200),('10:01:04','ge',150);
insert into q values ('10:01:00','ibm',100),('10:01:00','msft',99),('10:01:02','msft',101),('10:01:02','ibm',98);

select * from t asof outer join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:00 msft 100 99  
--10:01:03 ibm  200 98
--10:01:04 ge   150   
select * from t asof join q on (t.sym=q.sym and t.tm >= q.tm);
--10:01:00 msft 100 99  
--10:01:03 ibm  200 98


create table o (x time);
insert into o values ('10:01'),('10:03'),('10:07'),('10:08');
create table i (x time);
insert into i values ('10:02'),('10:04'),('10:06'),('10:08');
select * from o asof outer join i on (o.x >= i.x);
select * from o asof outer join i using x;
