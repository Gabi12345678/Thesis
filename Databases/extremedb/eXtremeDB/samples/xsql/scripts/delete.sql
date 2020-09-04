--Table S (sid	char(5) primary key, sname char(20), status integer, city char(15) )
INSERT INTO S(sid,sname,status,city) VALUES('S1','Smith',20,'London');
INSERT INTO S(sid,sname,status,city) VALUES('S2','Jones',10,'Paris');
INSERT INTO S(sid,sname,status,city) VALUES('S3','Black',30,'Paris');
INSERT INTO S(sid,sname,status,city) VALUES('S4','Clark',20,'London');
INSERT INTO S(sid,sname,status,city) VALUES('S5','Adams',30,'Athens');
SELECT * FROM S;

DELETE FROM S WHERE status < 15;
SELECT * FROM S;

DELETE FROM S WHERE sid IN (SELECT sid FROM S WHERE status = 30);
SELECT * FROM S;

DELETE FROM S WHERE sname LIKE 'Clark';
SELECT * FROM S;

DELETE FROM S;
