import math
import numpy as np

def zscore(datapoints):
	lines = datapoints.shape[0]
	columns = datapoints.shape[1]
	result = np.zeros((lines, columns))
	for j in range(columns):
		avg = 0.0
		for i in range(lines):
			avg = avg + datapoints[i][j]
		avg = avg / lines
		stdev = 0.0
		for i in range(lines): 
			stdev = stdev + ((datapoints[i][j] - avg) ** 2)
		stdev = math.sqrt(stdev / lines)
		for i in range(lines):	
			result[i][j] = (datapoints[i][j] - avg) / stdev
	return result
