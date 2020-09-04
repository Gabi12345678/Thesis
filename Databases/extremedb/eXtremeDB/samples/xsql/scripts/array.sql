create table foo (a array(int, 10), v array(string));
insert into foo values ('{0,1,2,3,4,5,6,7,8,9}', '{"one", "two", "three"}');
insert into foo values ([1,2,3,4,5,6,7,8,9,10], ['abc', 'xyz']);
insert into foo values ([1,2,3,4,5,6,7,8,9,10], []);
select * from foo;
