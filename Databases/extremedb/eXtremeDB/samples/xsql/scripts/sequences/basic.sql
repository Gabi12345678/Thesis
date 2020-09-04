create table foo (i int);
insert into foo (i) values (1);
insert into foo (i) values (2);
insert into foo (i) values (3);
insert into foo (i) values (3);
insert into foo (i) values (1);
select sequence(i) from foo;
select sequence(distinct i) from foo;
select seq_prd(sequence(all i)) from foo;
