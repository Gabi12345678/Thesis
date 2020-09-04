create table foo(x integer, y integer);
create index idx on foo(x);
insert into foo values (1,1);
insert into foo values (1,2);
insert into foo values (2,3);
insert into foo values (2,3);
insert into foo values (2,4);
explain select x,sum(distinct y) from foo group by x;
select x,sum(distinct y) from foo group by x;
