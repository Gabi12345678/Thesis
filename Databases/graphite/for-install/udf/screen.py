import os
import jnius_config
jnius_config.set_classpath(os.path.abspath(os.path.dirname(__file__)) + '/*')
import jnius
"""
import numpy as np

def screen(matrix, timestamps, sMax, sMin, windowSize):	
	lines = matrix.shape[0]
	columns = matrix.shape[1]

	Screen = jnius.autoclass('cn.edu.thu.screen.Screen')
	TimePoint = jnius.autoclass('cn.edu.thu.screen.entity.TimePoint')
	TimeSeries = jnius.autoclass('cn.edu.thu.screen.entity.TimeSeries')
	ArrayList = jnius.autoclass('java.util.ArrayList')
	
	result = np.zeros((lines, columns))
	for i in range(columns):
		alist = ArrayList()
		for j in range(lines):
			alist.add( TimePoint(timestamps[j], matrix[j][i]) )
		screen = Screen(TimeSeries(alist), sMax, sMin, windowSize)
		ts = screen.mainScreen().getTimeseries()
		for j in range(lines):
			p = ts.get(j)
			if p.isModified():
				result[j][i] = p.getModified()
			else:
				result[j][i] = p.getValue()
	return result 

		

"""
