-- _abs, _neg, _reverse
INSERT INTO SimpleSequence(testNumber,iVal1)
VALUES(1,[-3,-2,-1,0,1,2,3]);

SELECT iVal1, seq_abs(iVal1) AS "abs" FROM SimpleSequence WHERE testNumber = 1;
SELECT iVal1, seq_neg(iVal1) AS "neg" FROM SimpleSequence WHERE testNumber = 1;
SELECT iVal1, seq_neg(iVal1) AS "neg" FROM SimpleSequence WHERE testNumber = 1;
SELECT iVal1, seq_reverse(iVal1) AS "reverse" FROM SimpleSequence WHERE testNumber = 1;

-- _diff, _unique
INSERT INTO SimpleSequence(testNumber,iVal1)
VALUES(2,[4,9,2,9,1,4,9,7,5,3]);
INSERT INTO SimpleSequence(testNumber,iVal1)
VALUES(3,[4,4,7,7,7,5,4,3]);

SELECT iVal1, seq_diff(iVal1) AS "diff" FROM SimpleSequence WHERE testNumber = 2;
SELECT iVal1, seq_unique(iVal1) AS "unique" FROM SimpleSequence WHERE testNumber = 3;

-- _ceil, _floor
INSERT INTO SimpleSequence(testNumber,dVal1)
VALUES(4,[2.9,3.14,4.6]);

SELECT dVal1, seq_ceil(dVal1) AS "ceil" FROM SimpleSequence WHERE testNumber = 4;
SELECT dVal1, seq_floor(dVal1) AS "floor" FROM SimpleSequence WHERE testNumber = 4;

-- _norm
INSERT INTO SimpleSequence(testNumber,dVal1)
VALUES(5,[1,2,3]);
SELECT dVal1, seq_norm(seq_floor(dVal1)) AS "norm" FROM SimpleSequence WHERE testNumber = 5;

-- _trend
INSERT INTO SimpleSequence(testNumber,iVal1)
VALUES(6,[1,2,3,3,2,2,4,5,6,5]);

SELECT iVal1, seq_trend(iVal1) AS "trend" FROM SimpleSequence WHERE testNumber = 6;

-- _integer, _real
INSERT INTO SimpleSequence(testNumber,fVal1)
VALUES(7,[4.9,10.3,13.7]);
INSERT INTO SimpleSequence(testNumber,iVal1)
VALUES(8,[3,5,7,11,13]);

SELECT fVal1, seq_integer(fVal1) AS "int" FROM SimpleSequence WHERE testNumber = 7;
SELECT iVal1, seq_real(iVal1) AS "real" FROM SimpleSequence WHERE testNumber = 8;

-- _sqrt, _log, _exp
INSERT INTO SimpleSequence(testNumber,dVal1,dVal2,dVal3)
VALUES(9,[4,25,36],[10,100,1000],[2,3,4]);

SELECT dVal1, seq_sqrt(dVal1) AS "sqrt" FROM SimpleSequence WHERE testNumber = 9;
SELECT dVal2, seq_log(dVal2) AS "log" FROM SimpleSequence WHERE testNumber = 9;
SELECT dVal3, seq_exp(dVal3) AS "exp" FROM SimpleSequence WHERE testNumber = 9;

-- _sin, _cos, _tan, _asin, _acos, _atan
INSERT INTO SimpleSequence(testNumber,dVal1,dVal2,dVal3)
VALUES(10,[0,90,180],[-1.0,0.5,1.0],[3.14159,6.28319,9.42777]);

SELECT dVal3, seq_sin(dVal3) AS "sin" FROM SimpleSequence WHERE testNumber=10;
SELECT dVal3, seq_cos(dVal3) AS "cos" FROM SimpleSequence WHERE testNumber=10;
SELECT dVal3, seq_tan(dVal3) AS "tan" FROM SimpleSequence WHERE testNumber=10;
SELECT dVal2, seq_asin(dVal2) AS "asin" FROM SimpleSequence WHERE testNumber=10;
SELECT dVal2, seq_acos(dVal2) AS "acos" FROM SimpleSequence WHERE testNumber=10;
SELECT dVal2, seq_atan(dVal2) AS "atan" FROM SimpleSequence WHERE testNumber=10;

-- _not, _filter_pos
INSERT INTO SimpleSequence(testNumber,bVal1)
VALUES(11,[1,0,1,0]);

SELECT bVal1,seq_not(bVal1) As "not" FROM SimpleSequence where testNumber=11;
SELECT bVal1,seq_filter_pos(bVal1) As "filter_pos" FROM SimpleSequence where testNumber=11;

DELETE FROM SimpleSequence;
