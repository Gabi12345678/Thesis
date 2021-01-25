from graphite.functions.params import Param, ParamTypes
import graphite.functions.custom.kmeans as kmeans
import graphite.functions.custom.cd_ssv as cd_ssv
import graphite.functions.custom.recovery as recovery
import graphite.functions.custom.hotsax as hotsax
import graphite.functions.custom.znormalization as znormalization
import graphite.functions.custom.screen as screen
import graphite.functions.custom.saxtransformation as saxtransformation
import graphite.functions.custom.knn as knn
import sys
sys.path.append("/opt/graphite/webapp/graphite/functions/custom")
from dstree.dynamicsplit import IndexBuilder, IndexExactSearcher
from graphite.render.datalib import TimeSeries
import numpy as np
import os


def applyKmeans(requestContext, seriesLists):
        """Custom function that runs kmeans"""
        # Extract the data into 2D numpy.array
        matrix = []
        for i in range(len(seriesLists)):
                datapoints = seriesLists[i].datapoints()
                current_measurements = []
                for j in range(len(datapoints)):
                        if datapoints[j][0] == None:
                                continue
                        current_measurements.append( datapoints[j][0] )
                current_measurements = np.array(current_measurements)
                matrix.append(current_measurements)
        matrix = np.array(matrix).T
        f = open("/var/log/debug.txt", "w")
        f.write(str(matrix.shape) + "\n")
        f.write(str(matrix))
        f.close()

        # Apply KMeans
        clusterCount = min(10, matrix.shape[0])
        clusters = kmeans.kmeans(matrix, clusterCount, 20)

        # Format the data into TimeSeries
        result = []
        timeSeriesCount = matrix.shape[1]
        startTime = seriesLists[0].start
        stepTime = seriesLists[0].step
        endTime = startTime + clusterCount * stepTime
        for j in range(timeSeriesCount):
                result.append( TimeSeries(name = "master.kmeans.dim" + str(j), start = startTime, end = endTime, step = stepTime, values = clusters.T[j].tolist()) )

        return result


applyKmeans.group = 'Custom'
applyKmeans.params = [
        Param('seriesLists', ParamTypes.seriesLists, required=True),
]




def applyCD(requestContext, seriesLists):
        """Custom function that runs cd"""

        # Extract the data into 2D numpy.array
        matrix = []
        for i in range(len(seriesLists)):
                datapoints = seriesLists[i].datapoints()
                current_measurements = []
                for j in range(len(datapoints)):
                        if datapoints[j][0] == None:
                                continue
                        current_measurements.append( datapoints[j][0] )
                current_measurements = np.array(current_measurements)
                matrix.append(current_measurements)
        matrix = np.array(matrix).T
        f = open("/var/log/debug.txt", "w")
        f.write(str(matrix.shape) + "\n")
        f.write(str(matrix))
        f.close()

        # Apply KMeans
        matrix_l, matrix_r, z = cd_ssv.CD(matrix, matrix.shape[0], matrix.shape[1])

        # Format the data into TimeSeries
        result = []
        timeSeriesCount = matrix_r.shape[1]
        startTime = seriesLists[0].start
        stepTime = seriesLists[0].step
        endTime = startTime + timeSeriesCount * stepTime
        for j in range(timeSeriesCount):
                result.append( TimeSeries(name = "master.cd.dim" + str(j), start = startTime, end = endTime, step = stepTime, values = matrix_r.T[j].tolist()) )

        return result


applyCD.group = 'Custom'
applyCD.params = [
        Param('seriesLists', ParamTypes.seriesLists, required=True),
]



def applyRecov(requestContext, seriesLists):
	matrix = []
	timestamps = []
	for i in range(len(seriesLists)):
		datapoints = seriesLists[i].datapoints()
		current_measurement = []
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			current_measurement.append( datapoints[j][0] )
		current_measurement = np.array(current_measurement)
		matrix.append( current_measurement )
	matrix = np.array(matrix).T
	
	n, m = matrix.shape
	rec_time, iter, rmse, rec_mat = recovery.recovery(matrix, n, m, 3, 0.2, 10)

	result = []
	for i in range(m):
		startTime = seriesLists[i].start
		stepTime = seriesLists[i].step

		datapoints = seriesLists[i].datapoints()
		current_measurement = []
		index_result = 0
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				current_measurement.append( None )
			else:
				current_measurement.append( rec_mat[index_result][i] )
				index_result = index_result + 1
		endTime = startTime + len(current_measurement) * stepTime
		result.append( TimeSeries(name = "recov.result.dim" + str(i), start = startTime, end = endTime, step = stepTime, values = current_measurement) )
	return result


applyRecov.group = 'Custom'
applyRecov.params = [
	Param('seriesLists', ParamTypes.seriesLists, required=True),
]



def applyHotSax(requestContext, seriesLists):
	matrix = []
	time = {}

	f = open("/var/log/debug.txt", "w")
	f.write(str(len(seriesLists)))
	f.close()

	for i in range(len(seriesLists)):
		datapoints = seriesLists[i].datapoints()
		current_measurement = []
		index = 0
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			time[index] = datapoints[j][1]
			index = index + 1
			current_measurement.append(datapoints[j][0])
		current_measurement = np.array(current_measurement)
		matrix.append(current_measurement)
	matrix = np.array(matrix).T

	#Apply Anomaly algorithm using SAX
	discord = hotsax.hotsax( matrix )

	r_time = []
	r_index = []
	r_value = []
	for i in range(len(discord)):
		r_time.append(time[discord[i][1]])
		r_index.append(discord[i][0])
		r_value.append(discord[i][2])
	start = seriesLists[0].start
	step = seriesLists[0].step
	end = start + step * len(discord)
	return [ TimeSeries(name = 'recov.result.timestamp', start = start, end = end, step = step, values = r_time),
		 TimeSeries(name = 'recov.result.column', start = start, end = end, step = step, values = r_index),
		 TimeSeries(name = 'recov.result.value', start = start, end = end, step = step, values = r_value) ]


applyHotSax.group = 'Custom'
applyHotSax.params = [
	Param('seriesLists', ParamTypes.seriesLists, required=True),
]



def applyZScore(requestContext, seriesLists):
	"""Custom function that runs zscore"""
	# Extract the data into 2D numpy.array
	matrix = []
	for i in range(len(seriesLists)):
		datapoints = seriesLists[i].datapoints()
		current_measurements = []
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			current_measurements.append( datapoints[j][0] )
		current_measurements = np.array(current_measurements)
		matrix.append(current_measurements)
	matrix = np.array(matrix).T

	# Apply ZScore
	zscore = znormalization.zscore(matrix)

	# Format the data into TimeSeries
	result = []
	timeSeriesCount = matrix.shape[1]
	startTime = seriesLists[0].start
	stepTime = seriesLists[0].step
	endTime = startTime + len(seriesLists[0]) * stepTime
	for j in range(timeSeriesCount):
		result.append( TimeSeries(name = "master.zscore.dim" + str(j), start = startTime, end = endTime, step = stepTime, values = zscore.T[j].tolist()) )

	return result


applyZScore.group = 'Custom'
applyZScore.params = [
	Param('seriesLists', ParamTypes.seriesLists, required=True),
]


def applyScreen(requestContext, seriesLists):
	# Extract the data into 2D numpy.array
	matrix = []
	timestamp = []
	for i in range(len(seriesLists)):
		datapoints = seriesLists[i].datapoints()
		current_measurements = []
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			timestamp.append(datapoints[j][1])
			current_measurements.append( datapoints[j][0] )
		current_measurements = np.array(current_measurements)
		matrix.append(current_measurements)
	matrix = np.array(matrix).T

	# Apply Screen
	screenResult = screen.screen(matrix, timestamp, 0.1, -0.1, 300)

	# Format the data into TimeSeries
	result = []
	timeSeriesCount = matrix.shape[1]
	startTime = seriesLists[0].start
	stepTime = seriesLists[0].step
	endTime = startTime + len(seriesLists[0]) * stepTime
	for j in range(timeSeriesCount):
		result.append( TimeSeries(name = "master.screen.dim" + str(j), start = startTime, end = endTime, step = stepTime, values = screenResult.T[j].tolist()) )

	return result


applyScreen.group = 'Custom'
applyScreen.params = [
	Param('seriesLists', ParamTypes.seriesLists, required=True),
]


def applySaxRepresentation(requestContext, seriesLists):
	matrix = []
	for i in range(len(seriesLists)):
		datapoints = seriesLists[i].datapoints()
		current_measurements = []
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			current_measurements.append( datapoints[j][0] )
		current_measurements = np.array(current_measurements)
		matrix.append(current_measurements)
	matrix = np.array(matrix).T

	saxRepresentation = saxtransformation.saxrepresentation(matrix)
	result = []
	timeSeriesCount = matrix.shape[1]
	startTime = seriesLists[0].start
	stepTime = seriesLists[0].step
	endTime = startTime + len(seriesLists[0]) * stepTime
	for j in range(timeSeriesCount):
		result.append( TimeSeries(name = "master.saxrepresentation.dim" + str(j), start = startTime, end = endTime, step = stepTime, values = list(saxRepresentation[j])) )
	return result


applySaxRepresentation.group = 'Custom'
applySaxRepresentation.params = [
	Param('seriesLists', ParamTypes.seriesLists, required = True),
]


def applyDstreeIndex(requestContext, seriesLists, columns):
	try:
		os.mkdir("/var/log/out/")
	except FileExistsError:
		pass
	IndexBuilder.buildIndex("/var/log/data.txt", "/var/log/out/index", 100,  1, 1000, columns)
	return []	

applyDstreeIndex.group = 'Custom'
applyDstreeIndex.params = [
	Param('seriesLists', ParamTypes.seriesLists, required = False),
	Param('columns', ParamTypes.integer, required = True),
]


def applyDstreeSearch(requestContext, seriesLists, columns):
	matrix = []
	for i in range(len(seriesLists)):
		datapoints = seriesLists[i].datapoints()
		current_measurements = []
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			current_measurements.append( datapoints[j][0] )
		current_measurements = np.array(current_measurements)
		matrix.append(current_measurements)
	matrix = np.array(matrix)

	distances = IndexExactSearcher.search(matrix, "/var/log/out/index.idx_dyn_100_1_" + str(columns))

	startTime = seriesLists[0].start
	stepTime = seriesLists[0].step
	endTime = startTime + len(distances) * stepTime
	return [ TimeSeries(name = "master.dstree.dist", start = startTime, end = endTime, step = stepTime, values = distances) ]


applyDstreeSearch.group = 'Custom'
applyDstreeSearch.params = [
	Param('seriesLists', ParamTypes.seriesLists, required = True),
	Param('columns', ParamTypes.integer, required = True),
]

def applyKNN(requestContext, labelSeries, labelColumn, unlabelSeries):
	labelMatrix = []
	for i in range(len(labelSeries)):
		datapoints = labelSeries[i].datapoints()
		current_measurements = []
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			current_measurements.append( datapoints[j][0] )
		current_measurements = np.array(current_measurements)
		labelMatrix.append(current_measurements)
	labelMatrix = np.array( labelMatrix ).T
	
	unlabelMatrix = []
	for i in range(len(unlabelSeries)):
		datapoints = unlabelSeries[i].datapoints()
		current_measurements = []
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			current_measurements.append( datapoints[j][0] )
		current_measurements = np.array( current_measurements )
		unlabelMatrix.append(current_measurements)
	unlabelMatrix = np.array( unlabelMatrix ).T

	labels = []
	datapoints = labelColumn[0].datapoints()
	for j in range(len(datapoints)):
		if datapoints[j][0] == None:
			continue
		labels.append( datapoints[j][0] )

	result = knn.knn(labelMatrix, labels, unlabelMatrix, 3)

	startTime = labelSeries[0].start
	stepTime = labelSeries[0].step
	endTime = startTime + len(result) * stepTime
	return [ TimeSeries(name = "master.knn.dist", start = startTime, end = endTime, step = stepTime, values = result) ]

applyKNN.group = 'Custom'
applyKNN.params = [
	Param('labelSeries', ParamTypes.seriesLists, required = True),
	Param('labelColumn', ParamTypes.seriesList, required = True),
	Param('unlabelSeries', ParamTypes.seriesLists, required = True),
]

def applyCartesian(requestContext, labelSeries, labelColumn, unlabelSeries):
	labelColumns = []
	for i in range(len(labelSeries)):
		datapoints = labelSeries[i].datapoints()
		current_measurements = []
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			current_measurements.append( datapoints[j][0] )
		labelColumns.append(current_measurements)
	labelColumns = np.array(labelColumns).T.tolist()

	unlabelColumns = []
	for i in range(len(unlabelSeries)):
		datapoints = unlabelSeries[i].datapoints()
		current_measurements = []
		for j in range(len(datapoints)):
			if datapoints[j][0] == None:
				continue
			current_measurements.append( datapoints[j][0] )
		unlabelColumns.append(current_measurements)
	unlabelColumns = np.array(unlabelColumns).T.tolist()

	labels = []
	datapoints = labelColumn[0].datapoints()
	for j in range(len(datapoints)):
		if datapoints[j][0] == None:
			continue
		labels.append( datapoints[j][0] )

	columns = len(unlabelSeries)
	cart = np.zeros( (len(labelColumns) * len(unlabelColumns), columns * 2 + 1) )
	f = open("/var/log/labelColumns.txt", "w")
	f.write(str(labelColumns))
	f.close()
	f = open("/var/log/unlabelColumns.txt", "w")
	f.write(str(unlabelColumns))
	f.close()
	f = open("/var/log/labels.txt", "w")
	f.write(str(labels))
	f.close()
	p = 0
	for i in range(len(labelColumns)):
		for j in range(len(unlabelColumns)):
			for t in range(columns):
				cart[p][t] = labelColumns[i][t]
			for t in range(columns):
				cart[p][t + columns] = unlabelColumns[j][t]
			cart[p][2 * columns] = labels[i]
			p = p + 1
	cart = cart.T.tolist()
	result = []
	startTime = labelSeries[0].start
	stepTime = labelSeries[0].step
	endTime = startTime + len(cart) * stepTime
	for i in range(columns):
		result.append( TimeSeries(name = "master.cartesian.l" + str(i), start = startTime, end = endTime, step = stepTime, values = cart[i]) )
	for i in range(columns):
		result.append( TimeSeries(name = "master.cartesian.d" + str(i), start = startTime, end = endTime, step = stepTime, values = cart[i + columns]) )
	result.append( TimeSeries(name = "master.cartesian.label", start = startTime, end = endTime, step = stepTime, values = cart[2 * columns]) )
	return result
	
applyCartesian.group = 'Custom'
applyCartesian.params = [
	Param('labelSeries', ParamTypes.seriesLists, required = True),
	Param('labelColumn', ParamTypes.seriesList, required = True),
	Param('unlabelSeries', ParamTypes.seriesLists, required = True),
]
		
SeriesFunctions = {
        'kmeans': applyKmeans,
	'cd' : applyCD,
	'recov': applyRecov,
	'hotsax': applyHotSax,
	'zscore': applyZScore,
	'screen': applyScreen,
	'saxrepresentation': applySaxRepresentation,
	'dstreeindex': applyDstreeIndex,
	'dstreesearch': applyDstreeSearch,
	'knn': applyKNN,
	'cartesian': applyCartesian
}

