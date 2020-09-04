set implicit_aggregates=true;

SELECT
	Ccy AS NAME,
	bx_wt_avg ( ["PTR_neg1secs", usdnotional] ) AS "-1secs",
	bx_wt_avg ( ["PTR_neg2secs", usdnotional] ) AS "-2secs",
	bx_wt_avg ( ["PTR_neg3secs", usdnotional] ) AS "-3secs"
FROM
	trade
GROUP BY
	Ccy;
