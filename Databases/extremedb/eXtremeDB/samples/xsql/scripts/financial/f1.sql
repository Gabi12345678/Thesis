-- _count, _max, _min
INSERT INTO SimpleSequence(testNumber,iVal1)
VALUES(1,'{-3,-2,-1,0,1,2,3}');

SELECT iVal1, seq_count(iVal1) AS "count", seq_max(iVal1) AS "max", seq_min(iVal1) AS "min" 
FROM SimpleSequence WHERE testNumber = 1;

INSERT INTO SimpleSequence(testNumber,iVal1,fVal1)
VALUES(2,'{-3,-2,-1,0,1,2,3}','{11.1,17.7,-23.3,42.5}');

SELECT seq_first_int(iVal1) AS "first_int", seq_last_int(iVal1) AS "last_int",
	   seq_first_real(fVal1) AS "first_real", seq_last_real(fVal1) AS "last_real"  
FROM SimpleSequence WHERE testNumber = 2;

-- _sum, _prd, _avg, _var, _dev
INSERT INTO SimpleSequence(testNumber,fVal1)
VALUES(3,'{1.1,2.2,3.3}');

SELECT fVal1, seq_sum(fVal1) AS "sum", seq_prd(fVal1) AS "prd", seq_avg(fVal1) AS "avg",
       seq_var(fVal1) AS "var", seq_dev(fVal1) AS "dev" 
FROM SimpleSequence WHERE testNumber = 3;

-- _approxdc
INSERT INTO SimpleSequence(testNumber,fVal1)
VALUES(4,'{1.1,1.1,2.2,3.3}');

SELECT fVal1, seq_approxdc(fVal1) AS "approxdc" 
FROM SimpleSequence WHERE testNumber = 4;

-- _empty
INSERT INTO SimpleSequence(testNumber,fVal1,iVal1)
VALUES(5,'{1.1,1.1,2.2,3.3}', '{}');

SELECT seq_empty(iVal1) AS "iVal1 empty?", seq_empty(fVal1) AS "fVal1 empty?"  
FROM SimpleSequence WHERE testNumber = 5;

DELETE FROM SimpleSequence;
