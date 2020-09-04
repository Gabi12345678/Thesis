-- create table S (sid 	char(5) primary key, 
-- 		sname	char(20),
-- 		status  integer,
-- 		city    char(15));

-- create table P (pid 	char(5) primary key,
-- 		pname   char(20),
-- 		color   char(6),
-- 		weight  double,
-- 		city    char(15));

-- create table J (jid 	char(5) primary key,	
-- 		jname   varchar,
-- 		city    char(15));


-- create table SP(sid 	char(5) using index,
-- 		pid 	char(5) using index,
-- 		jid 	char(5) using index,
-- 		qty	bigint);


insert into S (sid,sname,status,city) values('S1', 'Smith', 20, 'London');
insert into S (sid,sname,status,city) values('S2', 'Jones', 10, 'Paris');
insert into S (sid,sname,status,city) values('S3', 'Black', 30, 'Paris');
insert into S (sid,sname,status,city) values('S4', 'Clark', 20, 'London');
insert into S (sid,sname,status,city) values('S5', 'Adams', 30, 'Athens');

insert into P (pid,pname,color,weight,city) values('P1', 'Nut', 'Red', 12, 'London');
insert into P (pid,pname,color,weight,city) values('P2', 'Bolt', 'Green', 17, 'Paris');
insert into P (pid,pname,color,weight,city) values('P3', 'Screw', 'Blue', 17, 'Rome');
insert into P (pid,pname,color,weight,city) values('P4', 'Screw', 'Red', 14, 'London');
insert into P (pid,pname,color,weight,city) values('P5', 'Cam', 'Blue', 12, 'Paris');
insert into P (pid,pname,color,weight,city) values('P6', 'Cog', 'Red', 19, 'London');

insert into J (jid,jname,city) values('J1', 'Sorter', 'Paris');
insert into J (jid,jname,city) values('J2', 'Display', 'Rome');
insert into J (jid,jname,city) values('J3', 'OCR', 'Athens');
insert into J (jid,jname,city) values('J4', 'Console', 'Athens');
insert into J (jid,jname,city) values('J5', 'RAID', 'London');
insert into J (jid,jname,city) values('J6', 'EDS', 'Oslo');
insert into J (jid,jname,city) values('J7', 'Tape', 'London');

insert into SP (sid,pid,jid,qty) values('S1', 'P1', 'J1', 200);
insert into SP (sid,pid,jid,qty) values('S1', 'P1', 'J4', 700);
insert into SP (sid,pid,jid,qty) values('S2', 'P3', 'J1', 400);
insert into SP (sid,pid,jid,qty) values('S2', 'P3', 'J2', 200);
insert into SP (sid,pid,jid,qty) values('S2', 'P3', 'J3', 200);
insert into SP (sid,pid,jid,qty) values('S2', 'P3', 'J4', 500);
insert into SP (sid,pid,jid,qty) values('S2', 'P3', 'J5', 600);
insert into SP (sid,pid,jid,qty) values('S2', 'P3', 'J6', 400);
insert into SP (sid,pid,jid,qty) values('S2', 'P3', 'J7', 800);
insert into SP (sid,pid,jid,qty) values('S2', 'P5', 'J2', 100);
insert into SP (sid,pid,jid,qty) values('S3', 'P3', 'J1', 200);
insert into SP (sid,pid,jid,qty) values('S3', 'P4', 'J2', 500);
insert into SP (sid,pid,jid,qty) values('S4', 'P6', 'J3', 300);
insert into SP (sid,pid,jid,qty) values('S4', 'P6', 'J7', 300);
insert into SP (sid,pid,jid,qty) values('S5', 'P2', 'J2', 200);
insert into SP (sid,pid,jid,qty) values('S5', 'P2', 'J4', 100);
insert into SP (sid,pid,jid,qty) values('S5', 'P5', 'J5', 500);
insert into SP (sid,pid,jid,qty) values('S5', 'P5', 'J7', 100);
insert into SP (sid,pid,jid,qty) values('S5', 'P6', 'J2', 200);
insert into SP (sid,pid,jid,qty) values('S5', 'P1', 'J4', 100);
insert into SP (sid,pid,jid,qty) values('S5', 'P3', 'J4', 200);
insert into SP (sid,pid,jid,qty) values('S5', 'P4', 'J4', 800);
insert into SP (sid,pid,jid,qty) values('S5', 'P5', 'J4', 400);
insert into SP (sid,pid,jid,qty) values('S5', 'P6', 'J4', 500);

select P.color, P.city from P where P.city <> 'Paris' and P.weight > 10;

select distinct color, city from P where city <> 'Paris' and weight > 10;

select distinct P.color, P.city from P where P.city <> 'Paris' and weight > 10 order by city desc;

select pid, weight * 454 as GMWT from P;

select * from S;

select S.sid, S.sname, S.status, S.city, P.pid, P.pname, P.color, P.weight 
from S, P
where S.city = P.city;

select distinct S.city as scity, P.city as pcity from S, SP, P where S.sid = SP.sid and SP.pid = P.pid;

select FRST.sid as SA, SCND.sid as SB 
from S as FRST, S as SCND 
where FRST.city = SCND.city and FRST.sid < SCND.sid;

select count(*) N from S;

select max(SP.qty) as MAXQ, min (SP.qty) as MINQ from SP where SP.pid = 'P2';

select SP.pid, SUM(SP.qty) as TOTQTY from SP group by SP.pid;

select pid from SP group by pid having count(sid) > 1;

select distinct S.sname from S where sid in (select SP.sid from SP where SP.pid = 'P2');

select distinct S.sname from S where S.sid in ('S1', 'S2', 'S3', 'S4'); 

select distinct sname from S, SP where S.sid = SP.sid and SP.pid = 'P2';

select distinct sname from S where S.sid in 
	(select SP.sid from SP where SP.pid in 
		(select P.pid from P where P.color = 'Red' ));

select all S.sid from S where status < (select MAX(status) from S);

select distinct S.sname from S where exists (SELECT * from SP where SP.sid = S.sid and SP.pid = 'P2');

select distinct S.sname from S where not exists 
	(select * from SP where SP.pid = S.sid and SP.pid = 'P2');

select distinct S.sname from S where S.sid not in 
	(select SP.sid from SP where SP.pid = 'P2');

select distinct S.sname from S where not exists 
	(select * from P where not exists
		(select* from SP where SP.sid = S.sid and SP.pid = P.pid));

select distinct S.sname from S where (select count(SP.pid) from SP where SP.sid = S.sid)
	= (select count(P.pid) from P);	
	
select P.pid, 'Weight in grams = ' AS TEXT1, P.weight * 454 as GMWET, P.color, 'Max quantity = ' as TEST2, max(qty) as MQY
from P,SP
where P.pid = SP.pid and (P.color = 'Red' or P.color = 'Blue') and SP.qty > 200
group by P.pid, P.weight, P.color
having sum(SP.qty) > 350;

select distinct sname from S where S.sid in
	(select SP.sid from SP where SP.pid = 'P2');

select distinct sname from S where S.sid = any (select sid from SP where pid = 'P2');

select distinct S.sname from S where exists (select * from SP where SP.sid = S.sid and SP.pid = 'P2');

select distinct S.sname from S, SP where S.sid = SP.sid and SP.pid = 'P2';

select distinct sname from S where 0 < (select count(*) from SP where SP.sid = S.sid and SP.pid = 'P2');

select distinct S.sname from S where 'P2' in (select SP.pid from SP where SP.sid = S.sid);

select distinct S.sname from S,SP where S.sid = SP.sid and SP.pid = 'P2' group by S.sname;

select distinct SP.jid from SP where SP.jid = 'J1';
	
select SP.* from SP where qty >= 300 and qty < 750;

select distinct color,city from P;

select S.sid, P.pid, J.jid from S, P, J where S.city = P.city and P.city = J.city;

select S.sid, P.pid, J.jid from S, P, J where not (S.city = P.city and P.city = J.city);

select sid, pid, jid from S, P, J where S.city <> P.city and P.city <> J.city and J.city <> P.city;

select distinct SP.pid from SP where (select S.city from S where S.sid = SP.sid) = 'London';

select distinct SP.pid from SP where (select S.city from S where S.sid = SP.sid) = 'London' 
	and (select J.city from J where J.jid = SP.jid) = 'London';

select distinct S.city as SCITY, J.city as JCITY 
from S, J 
where exists (select * from SP where SP.sid = S.sid and SP.jid = J.jid);

select distinct SP.pid from SP where (select S.city from S where S.sid = SP.sid) = 
	(select J.city from J where J.jid = SP.jid);

select distinct SP.pid from SP where (select S.city from S where S.sid = SP.sid) <>
	(select J.city from J where J.jid = SP.jid);

select distinct SPJX.pid as PA, SPJY.pid as PB 
from SP as SPJX, SP as SPJY 
where SPJX.sid = SPJY.sid and SPJX.pid < SPJY.pid; 

select count(distinct SP.jid) as N from SP where SP.sid = 'S1';

select sum(qty) as X from SP where SP.sid = 'S1' and SP.pid = 'P1';

select SP.pid, SP.jid, sum(SP.qty) as Y from SP group by pid, jid;

select distinct SP.pid from SP group by SP.pid, SP.jid having avg(SP.qty) > 320;

select distinct J.jname from J, SP where J.jid = SP.jid and SP.sid = 'S1';

select distinct P.color from P, SP where P.pid = SP.pid and SP.sid = 'S1';

select distinct SP.pid from SP, J where SP.jid = J.jid and J.city = 'London';

select distinct SPJX.jid from SP as SPJX, SP as SPJY 
where SPJX.pid = SPJY.pid and SPJY.sid = 'S1';

select distinct SPJX.sid from SP as SPJX, SP as SPJY, SP as SPJZ
where SPJX.pid = SPJY.pid and SPJY.sid = SPJZ.sid and (select P.color from P where P.pid = SPJX.pid) = 'Red';

select S.sid from S where status < (select status from S where sid = 'S1');

select pid from P where weight = (select min(weight) from P);

select distinct SPJX.jid from SP as SPJX where SPJX.pid = 'P1' and 	
	(select avg(SPJY.qty) from SP as SPJY where SPJY.jid = SPJX.jid and SPJY.pid = 'P1') > 
	(select max (SPJZ.qty) from SP as SPJZ where SPJZ.jid = 'J1');

select distinct SPJX.sid from SP SPJX where SPJX.pid = 'P1' and
	SPJX.qty > (select avg(SPJY.qty) from SP as SPJY where SPJY.pid = 'P1' and SPJY.jid = SPJX.jid);

select J.jid from J where not exists (select * from S, P, SP 
	where SP.jid = J.jid and SP.pid = P.pid and SP.sid = S.sid and P.color = 'Red' and S.city = 'London');

select J.jid from J where not exists (select * from SP, P, S 
	where SP.jid = J.jid and SP.pid = P.pid and SP.sid = S.sid and P.color = 'Red' and S.city = 'London');
	
select jid from J where not exists (select * from SP where SP.jid = J.jid and NOT(SP.sid = 'S1'));

select pid from P where not exists 
	(select * from J where city = 'London' and not exists
		(select * from SP where SP.pid = P.pid and SP.jid = J.jid));

select S.sid from S where exists 
	(select * from P where not exists 
		(select * from J where not exists 
			(select * from SP where SP.sid = S.sid and SP.pid = P.pid and SP.jid = J.jid)));

select J.jid from J where not exists 
	(select * from SP as SPJX where SPJX.sid = 'S1' and not exists 
		(select * from SP as SPJY where SPJY.pid = SPJX.pid and SPJY.jid = J.jid));

select distinct SP.pid from SP where 
	(select S.city from S where S.sid = SP.sid) = 'London' or 
	(select J.city from J where J.jid = SP.jid) = 'London';

	
select * from S join P using city;

select * from S natural join P;

select distinct S.city as SCITY, P.city as PCITY from S join SP using sid join P using pid;

select distinct sname from S natural join SP where pid = 'P2';

select distinct sname from S join SP using sid where pid = 'P2';

select distinct T.sname from (S natural join SP) as T where pid = 'P2';

select distinct T.sname from (S join SP using sid) as T where T.pid = 'P2';

select S.city from S union select P.city from P union select J.city from J; 

select S.city from S union all select P.city from P union all select J.city from J; 

select distinct SP.* from S natural join SP where pid = 'P2';

select distinct * from (S natural join SP) as T where pid = 'P2';

select count(*) as c,city from S group by city having c>1;

select count(*),city from S group by city having count(*)>1;

select S.sid,S.city,P.pid from S left outer join P where S.city=P.city;

select * from S where sid='S1' UNION select * from S where sid>'S4';

select * from Metatable;

select G1.*, G2.* from (select sid,max(sname) from S group by sid) as G1, 
                       (select pid,max(pname) from P group by pid) as G2 
where G1.sid=G2.pid;
 
delete from S;
delete from P;
delete from J;
delete from SP;
