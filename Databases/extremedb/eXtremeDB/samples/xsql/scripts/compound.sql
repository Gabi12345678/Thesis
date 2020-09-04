create table A (a int, b int, c int, d int);
create index idx on A (a, b, c);
insert into A values (1,2,3,4);
select * from A a1, A a2 where a1.a=a2.a and a1.c=a2.c;
explain select * from A a1, A a2 where a1.a=a2.a and a1.c=a2.c;
