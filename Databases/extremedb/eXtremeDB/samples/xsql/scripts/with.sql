create table tab (id integer primary key, s string);
insert into tab values (1, 'Hello');
insert into tab values (2, 'world');
insert into tab values (3, 'Good');
insert into tab values (4, 'bye');
with w1 as (
( select * from tab where id = 1 )
     union all
( select * from tab where id = 2 )
), w2 as (
( select * from tab where id = 3 )
     union all
( select * from tab where id = 4 )
)
select t1.s,t2.s from w1 as t1,w2 as t2;
