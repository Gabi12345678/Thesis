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

SELECT P.color, P.city FROM P WHERE P.city <> 'Paris' AND P.weight > 10;

SELECT color, city FROM P WHERE city <> 'Paris' AND weight > 10;
SELECT DISTINCT color, city FROM P WHERE city <> 'Paris' AND weight > 10;
SELECT DISTINCT P.color, P.city FROM P WHERE P.city <> 'Paris' AND weight > 10 ORDER BY city ASC;
SELECT DISTINCT P.color, P.city FROM P WHERE P.city <> 'Paris' AND weight > 10 ORDER BY city DESC;

SELECT pid, weight * 454 AS GMWT FROM P;

SELECT S.sid, S.sname, S.status, S.city, P.pid, P.pname, P.color, P.weight FROM S, P WHERE S.city = P.city;

SELECT DISTINCT color, city FROM P;

SELECT pid FROM P where weight = (select min(weight) from P);
	
SELECT * FROM S JOIN P USING city;

SELECT * FROM S NATURAL JOIN P;

SELECT S.city FROM S UNION SELECT P.city from P; 

SELECT S.city FROM S UNION ALL SELECT P.city from P; 

SELECT S.sid, S.city, P.pid FROM S LEFT OUTER JOIN P where S.city = P.city;

DELETE FROM S;
DELETE FROM P;
