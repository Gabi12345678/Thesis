import saxpy
from saxpy.hotsax import find_discords_hotsax

def hotsax(matrix):
	result = []
	index_ts = 0
	for ts in matrix.T:
		discords = find_discords_hotsax(ts)
		for anomaly in discords:
			result.append([index_ts, anomaly[0], anomaly[1]])
		index_ts = index_ts + 1
	return result
