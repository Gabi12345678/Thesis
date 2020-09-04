insert into S (sid,keywords) values (1, '{"one", "two", "three", "one"}');
insert into S (sid,keywords) values (2, '{"abc", "xyz", "cba", "zyx"}');
insert into S (sid,keywords) values (3, '{"one", "more", "abc"}');
insert into S (sid,keywords) values (4, '{"two", "three"}');
insert into S (sid,keywords) values (5, '{"xyz"}');
insert into S (sid,keywords) values (6, '{"cba", "abc", "one"}');
select * from S;
trace on
select * from S where keywords contains all '{"abc", "one"}';
select * from S where keywords contains any '{"abc", "one"}';