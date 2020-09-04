create table A (a int, b int null);
create index idx on A (a,b);
insert into A (a) values (1);
insert into A (a) values (1);
select * from A s, A m where s.a=m.a;
select * from A a1, A a2 where a1.a in (1);
explain select * from A s, A m where s.a=m.a;
explain select * from A a1, A a2 where a1.a in (1);