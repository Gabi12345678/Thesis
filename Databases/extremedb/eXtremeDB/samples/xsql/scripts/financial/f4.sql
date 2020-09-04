-- _iif, _if
INSERT INTO SimpleSequence(testNumber,bVal1,iVal2,iVal3)
VALUES(1,'{0,1,0,0,1}','{-1,-2,-3,-4,-5}','{1,2,3,4,5}');

SELECT bVal1,iVal2,iVal3,seq_iif(bVal1,iVal2,iVal3) AS "iif" FROM SimpleSequence WHERE testNumber=1;
SELECT bVal1,iVal2,iVal3,seq_if(bVal1,iVal2,iVal3) AS "if" FROM SimpleSequence WHERE testNumber=1;

DELETE FROM SimpleSequence;
