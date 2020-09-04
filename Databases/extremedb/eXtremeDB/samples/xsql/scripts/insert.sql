--Table S (sid	char(5) primary key, sname char(20), status integer, city char(15) )
INSERT INTO S(sid,sname,status,city) VALUES('S1','Smith',20,'London');
INSERT INTO S(sid,sname,status,city) VALUES('S2','Jones',10,'Paris');
INSERT INTO S(sid,sname,status,city) VALUES('S3','Black',30,'Paris');
INSERT INTO S(sid,sname,status,city) VALUES('S4','Clark',20,'London');
INSERT INTO S(sid,sname,status,city) VALUES('S5','Adams',30,'Athens');
--Table P (pid	char(5) primary key, pname char(20), weight  double, city char(15) )
INSERT INTO P(pid,pname,color,weight,city) VALUES('P1','Nut','Red',12,'London');
INSERT INTO P(pid,pname,color,weight,city) VALUES('P2','Bolt','Green',17,'Paris'), ('P3','Screw','Blue',17,'Rome'), ('P4','Screw','Red',14,'London');
INSERT INTO P(pid,pname,color,weight,city) VALUES('P5','Cam','Blue',12,'Paris');
INSERT INTO P(pid,pname,color,weight,city) VALUES('P6','Cog','Red',19,'London');

SELECT * FROM S;
SELECT * FROM P;

-- insert a row with 'sname' selected from an existing row
INSERT INTO S (sname) SELECT sname FROM S WHERE status = 10;
-- notice that all columns except 'sname' are null
SELECT * FROM S;
-- the new row can be accessed by autoid
SELECT * FROM S WHERE autoid = 12;

-- insert a row with 'sname' selected from an existing row
INSERT INTO S (sid,sname,status,city) SELECT pid,pname,25,city FROM P;
SELECT * FROM S;

-- attempt to insert a value of the wrong type into column 'sid'
-- INSERT INTO S (sid, sname) SELECT autoid, sname FROM S;

DELETE FROM S;
