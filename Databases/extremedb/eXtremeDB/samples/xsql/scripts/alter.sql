create table foo(x integer);
insert into foo values (1);
insert into foo values (2);
insert into foo values (3);
alter table foo add column y varchar;
insert into foo values (4, 'ok');
select * from foo;


create table A(i integer, s string);
alter table A rename to B;

alter table B rename column i to d;

alter table B add column c char;

create table C(i integer);

alter table B add constraint constr1 primary key (d);
alter table B add constraint constr2 foreign key (d) references C(d) on delete cascade;
alter table B add constraint constr3 unique (c);

alter table B drop column s;
alter table B drop constraint constr1;
-- alter table B drop constraint constr2;
alter table B drop constraint constr3;

alter table B alter column c type unsigned;

alter table B set temporary;
alter table B set not temporary;
