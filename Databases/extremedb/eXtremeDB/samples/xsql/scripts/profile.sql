select start_profile(10);
create table foo(x integer);
insert into foo  select s ."#1" from generate_series(1,100000) s;
select count(*) from foo where (x %% 2) = 1;
select * from stop_profile();
