create table A (a int null, b int);
create index idx on A (a);
insert into A values (1,2);
select * from A a1, A a2 where a1.a=a2.a;
explain select * from A a1, A a2 where a1.a=a2.a;
