create table o(a integer);
create table i(b integer);
insert into o values (1),(2),(3),(4),(5);
insert into i values (1),(1),(4),(5),(5);
select * from o join i on o.a = i.b;
explain select * from o join i on o.a = i.b;

