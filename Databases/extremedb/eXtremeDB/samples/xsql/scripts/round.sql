create table numbers(i integer, r real, n numeric(10,3));
insert into numbers values (123, 1.235, '12.345');
select * from numbers;
select round(i,2),round(r,2),round(n,2) from numbers;
