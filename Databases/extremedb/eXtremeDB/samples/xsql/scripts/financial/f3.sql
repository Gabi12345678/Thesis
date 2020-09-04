-- _add, _sub, _mul, _div, _mod
INSERT INTO SimpleSequence(testNumber,dVal1,dVal2)
VALUES(1,'{1,2}','{3,4}');

SELECT dVal1,dVal2,seq_add(dVal1,dVal2) As "add" FROM SimpleSequence where testNumber=1;
SELECT dVal1,dVal2,seq_sub(dVal1,dVal2) As "sub" FROM SimpleSequence where testNumber=1;
SELECT dVal1,dVal2,seq_mul(dVal1,dVal2) As "mul" FROM SimpleSequence where testNumber=1;
SELECT dVal1,dVal2,seq_div(dVal1,dVal2) As "div" FROM SimpleSequence where testNumber=1;

-- mod
INSERT INTO SimpleSequence(testNumber,dVal1,dVal2)

VALUES(2,'{6,7}','{3,4}');
SELECT dVal1,dVal2,seq_mod(dVal1,dVal2) As "mod" FROM SimpleSequence where testNumber=2;

-- _eq, _ne, _gt, _ge, _lt, _le
INSERT INTO SimpleSequence(testNumber,iVal1,iVal2)
VALUES(3,'{42,-13,27,19}','{42,-12,26,20}');

SELECT iVal1,iVal2,seq_eq(iVal1,iVal2) AS "eq" FROM SimpleSequence WHERE testNumber=3;
SELECT iVal1,iVal2,seq_ne(iVal1,iVal2) AS "ne" FROM SimpleSequence WHERE testNumber=3;
SELECT iVal1,iVal2,seq_gt(iVal1,iVal2) AS "gt" FROM SimpleSequence WHERE testNumber=3;
SELECT iVal1,iVal2,seq_ge(iVal1,iVal2) AS "ge" FROM SimpleSequence WHERE testNumber=3;
SELECT iVal1,iVal2,seq_lt(iVal1,iVal2) AS "lt" FROM SimpleSequence WHERE testNumber=3;
SELECT iVal1,iVal2,seq_le(iVal1,iVal2) AS "le" FROM SimpleSequence WHERE testNumber=3;

-- _and, _or, _xor
INSERT INTO SimpleSequence(testNumber,bVal1,bVal2)
VALUES(4,'{1,1,0,0}','{1,0,1,0}');

SELECT bVal1,bVal2,seq_and(bVal1,bVal2) AS "and" FROM SimpleSequence WHERE testNumber=4;
SELECT bVal1,bVal2,seq_or(bVal1,bVal2) AS "or" FROM SimpleSequence WHERE testNumber=4;
SELECT bVal1,bVal2,seq_xor(bVal1,bVal2) AS "xor" FROM SimpleSequence WHERE testNumber=4;

-- _wsum, _wavg
INSERT INTO SimpleSequence(testNumber,dVal1,dVal2)
VALUES(5,'{2,3,4}','{1,2,4}');

SELECT dVal1,dVal2,seq_wsum(dVal1,dVal2) AS "wsum" FROM SimpleSequence WHERE testNumber=5;
SELECT dVal1,dVal2,seq_wavg(dVal1,dVal2) AS "wavg" FROM SimpleSequence WHERE testNumber=5;

-- _cov, _corr
INSERT INTO SimpleSequence(testNumber,dVal1,dVal2)
VALUES(6,'{2,3,5,7}','{3,3,5,9}');

SELECT dVal1,dVal2,seq_cov(dVal1,dVal2) AS "cov" FROM SimpleSequence WHERE testNumber=6;
SELECT dVal1,dVal2,seq_corr(dVal1,dVal2) AS "corr" FROM SimpleSequence WHERE testNumber=6;

-- _pow
INSERT INTO SimpleSequence(testNumber,dVal1,dVal2)
VALUES(7,'{2,3}','{2,3}');

SELECT dVal1,dVal2,seq_pow(dVal1,dVal2) AS "pow" FROM SimpleSequence WHERE testNumber=7;

DELETE FROM SimpleSequence;
