create table foo(x integer primary key, y integer, z integer);
insert into foo values (1,2,3);
insert into foo values (4,5,6);
create table boo(a integer primary key, b integer);
insert into boo values (4,44); 
update foo set z=b from foo join boo on x=a;
select * from foo;
insert into boo values (1,101);
update boo set b=55 where a=4;
update foo set z=b from foo join boo on x=a where b > 100;
select * from foo;
update foo set z=b from boo join foo on x=a where b < 100;
select * from foo;

