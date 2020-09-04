--Table S (sid	char(5) primary key, sname char(20), status integer, city char(15) )
INSERT INTO S(sid,sname,status,city) VALUES('S1','Smith',20,'London');
INSERT INTO S(sid,sname,status,city) VALUES('S2','Jones',10,'Paris');
INSERT INTO S(sid,sname,status,city) VALUES('S3','Black',30,'Paris');
INSERT INTO S(sid,sname,status,city) VALUES('S4','Clark',20,'London');
INSERT INTO S(sid,sname,status,city) VALUES('S5','Adams',30,'Athens');

SELECT * FROM S;

SELECT FRST.sid AS SA, SCND.sid AS SB FROM S AS FRST, S AS SCND 
WHERE FRST.city = SCND.city AND FRST.sid < SCND.sid;

SELECT COUNT(*) N FROM S;

SELECT DISTINCT S.sname FROM S WHERE S.sid IN ('S1', 'S2', 'S3', 'S4'); 

SELECT ALL S.sid FROM S WHERE status < (SELECT MAX(status) FROM S);

SELECT S.sid FROM S WHERE status < (SELECT status FROM S WHERE sid = 'S1');

SELECT COUNT(*) AS c, city FROM S GROUP BY city HAVING c > 1;

SELECT COUNT(*), city FROM S GROUP BY city HAVING COUNT(*) > 1;

SELECT * FROM S WHERE sid='S1' UNION SELECT * FROM S WHERE sid > 'S4';

DELETE FROM S;
