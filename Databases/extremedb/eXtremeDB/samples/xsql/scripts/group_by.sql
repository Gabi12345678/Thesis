--Table S (sid	char(5) primary key, sname char(20), status integer, city char(15) )
INSERT INTO S(sid,sname,status,city) VALUES('S1','Smith',20,'London');
INSERT INTO S(sid,sname,status,city) VALUES('S2','Jones',10,'Paris');
INSERT INTO S(sid,sname,status,city) VALUES('S3','Black',30,'Paris');
INSERT INTO S(sid,sname,status,city) VALUES('S4','Clark',20,'London');
INSERT INTO S(sid,sname,status,city) VALUES('S5','Adams',30,'Athens');
--Table P (pid	char(5) primary key, pname char(20), weight  double, city char(15) )
INSERT INTO P(pid,pname,color,weight,city) VALUES('P1','Nut','Red',12,'London');
INSERT INTO P(pid,pname,color,weight,city) VALUES('P2','Bolt','Green',17,'Paris');
INSERT INTO P(pid,pname,color,weight,city) VALUES('P3','Screw','Blue',17,'Rome');
INSERT INTO P(pid,pname,color,weight,city) VALUES('P4','Screw','Red',14,'London');
INSERT INTO P(pid,pname,color,weight,city) VALUES('P5','Cam','Blue',12,'Paris');
INSERT INTO P(pid,pname,color,weight,city) VALUES('P6','Cog','Red',19,'London');
--Table J (jid	char(5) primary key, jname varchar, city char(15) )
INSERT INTO J (jid,jname,city) VALUES('J1','Sorter','Paris');
INSERT INTO J (jid,jname,city) VALUES('J2','Display','Rome');
INSERT INTO J (jid,jname,city) VALUES('J3','OCR','Athens');
INSERT INTO J (jid,jname,city) VALUES('J4','Console','Athens');
INSERT INTO J (jid,jname,city) VALUES('J5','RAID','London');
INSERT INTO J (jid,jname,city) VALUES('J6','EDS','Oslo');
INSERT INTO J (jid,jname,city) VALUES('J7','Tape','London');
--Table SP (sid char(5) using index, pid char(5) using index,	jid char(5) using index, qty bigint)
INSERT INTO SP (sid,pid,jid,qty) VALUES('S1','P1','J1',200);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S1','P1','J4',700);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S2','P3','J1',400);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S2','P3','J2',200);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S2','P3','J3',200);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S2','P3','J4',500);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S2','P3','J5',600);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S2','P3','J6',400);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S2','P3','J7',800);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S2','P5','J2',100);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S3','P3','J1',200);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S3','P4','J2',500);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S4','P6','J3',300);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S4','P6','J7',300);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P2','J2',200);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P2','J4',100);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P5','J5',500);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P5','J7',100);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P6','J2',200);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P1','J4',100);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P3','J4',200);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P4','J4',800);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P5','J4',400);
INSERT INTO SP (sid,pid,jid,qty) VALUES('S5','P6','J4',500);

SELECT * FROM S;

-- show count of rows having the same 'city'
SELECT city, COUNT(sid) "Count" FROM S GROUP BY city;

-- show count of rows having the same 'status'
SELECT status, COUNT(sid) "Count" FROM S GROUP BY status;

-- use HAVING clause to show count of rows having the 'status' = 30
SELECT status, COUNT(sid) "Count" FROM S GROUP BY status HAVING status = 30;

-- show Weight in Grams for red and blue items in P with quantity > 200 and sum > 350 with text in output
SELECT P.pid, 'Grams = ' AS TEXT1, P.weight * 454 AS GMWET, P.color, 'Max Qty = ' AS TEXT2, 
max(qty) as MQY FROM P,SP WHERE P.pid = SP.pid AND (P.color = 'Red' OR P.color = 'Blue') AND SP.qty > 200
GROUP BY P.pid, P.weight, P.color HAVING sum(SP.qty) > 350;

-- show items in P with average quantity > 320
trace on
SELECT DISTINCT SP.pid FROM SP GROUP BY SP.pid, SP.jid HAVING avg(SP.qty) > 320;

-- show items in S with links to P2
SELECT DISTINCT S.sname FROM S,SP WHERE S.sid = SP.sid AND SP.pid = 'P2' GROUP BY S.sname;

-- show items in SP grouped by pid and jid
SELECT SP.pid, SP.jid, sum(SP.qty) AS Y FROM SP GROUP BY pid, jid;

-- show items in S grouped by city with count > 1
SELECT count(*) AS c,city FROM S GROUP BY city HAVING c>1;

-- show items in S grouped by city with count > 1
SELECT count(*),city FROM S GROUP BY city HAVING count(*)>1;

DELETE FROM S;
DELETE FROM P;
DELETE FROM J;
DELETE FROM SP;
