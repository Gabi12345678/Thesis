create table foo(x integer, y integer);
insert into foo values (1, 3);
insert into foo values (1, 1);
insert into foo values (1, 2);
insert into foo values (3, 2);
insert into foo values (3, 1);
insert into foo values (2, 3);
insert into foo values (2, 2);
insert into foo values (2, 1);
echo
explain select x, string_agg(y, ', ') from foo group by x;
explain select x, string_agg(y, ', ') from (select * from foo order by y) group by x;
select x, string_agg(y, ', ') from foo group by x;
select x, string_agg(y, ', ') from (select * from foo order by y) group by x;