--  seq_filter
INSERT INTO SimpleSequence(testNumber,bVal1,iVal1)
VALUES(1,'{0,1,0,1}','{42,-13,27,19}');

SELECT bVal1, iVal1, seq_filter(bVal1,iVal1) AS "filter" FROM SimpleSequence WHERE testNumber = 1;
SELECT bVal1, iVal1, seq_filter_pos(bVal1) AS "filter_pos" FROM SimpleSequence WHERE testNumber = 1;

-- _concat
INSERT INTO SimpleSequence(testNumber,iVal1,iVal2)
VALUES(2,'{1}','{2,3}');

SELECT iVal1,iVal2,seq_concat(iVal1,iVal2) AS "concat" FROM SimpleSequence WHERE testNumber=2;

-- _map
INSERT INTO SimpleSequence(testNumber,iVal1,ui8)
VALUES(3,'{10,20,30}','{0,2}');

SELECT iVal1,ui8,seq_map(iVal1,ui8) AS "map" FROM SimpleSequence WHERE testNumber=3;

-- _repeat
INSERT INTO SimpleSequence(testNumber,iVal1)
VALUES(4,'{42,17}');

SELECT iVal1,seq_repeat(iVal1,3) AS "repeat" FROM SimpleSequence WHERE testNumber=4;

-- _stretch
INSERT INTO SimpleSequence(testNumber,ts1,ts2,dVal3)
VALUES(5,'{1,2,3,4,5}','{2,4}','{1.1,2.2}');

SELECT ts1,ts2,dVal3,seq_stretch(ts1,ts2,dVal3) AS "stretch" FROM SimpleSequence WHERE testNumber=5;

-- _stretch0
INSERT INTO SimpleSequence(testNumber,ts1,ts2,dVal3)
VALUES(6,'{1,2,3,5}','{2,3,4}','{1.1,1.2,1.3}');

SELECT ts1,ts2,dVal3,seq_stretch0(ts1,ts2,dVal3) AS "stretch0" FROM SimpleSequence WHERE testNumber=6;

-- _asof_join
INSERT INTO SimpleSequence(testNumber,ts1,ts2,dVal3)
VALUES(7,'{4,9}','{1,3,6,10}','{0.1,0.3,0.6,1.0}');

SELECT ts1,ts2,dVal3,seq_asof_join(ts1,ts2,dVal3) AS "asof_join" FROM SimpleSequence WHERE testNumber=7;

-- _cross
INSERT INTO SimpleSequence(testNumber,iVal1)
VALUES(8,'{-1,0,1,-3,2}');

SELECT iVal1,seq_cross(iVal1,-1) AS "cross-neg",seq_cross(iVal1,1) AS "cross-pos",seq_cross(iVal1,0) AS "cross-0" 
FROM SimpleSequence WHERE testNumber=8;

-- _extrema
INSERT INTO SimpleSequence(testNumber,iVal1)
VALUES(9,'{-3,-2,0,3,2}');

SELECT iVal1,seq_extrema(iVal1,-1) AS "extrema-neg",seq_extrema(iVal1,1) AS "extrema-pos",seq_extrema(iVal1,0) AS "extrema-0"  
FROM SimpleSequence WHERE testNumber=9;

DELETE FROM SimpleSequence;
