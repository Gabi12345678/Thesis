--create table S (sid 	char(5) primary key, 
--		sname	char(20),
--		status  integer,
--		city    char(15));

insert into S (sid,sname,status,city) values('S1', 'Smith', 20, 'London');
insert into S (sid,sname,status,city) values('S2', 'Jones', 10, 'Paris');
insert into S (sid,sname,status,city) values('S3', 'Black', 30, 'Paris');
insert into S (sid,sname,status,city) values('S4', 'Clark', 20, 'London');
insert into S (sid,sname,status,city) values('S5', 'Adams', 30, 'Athens');
select * from S;

delete from S;
