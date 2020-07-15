from graphite.functions.params import Param, ParamTypes
import graphite.functions.custom.kmeans as kmeans
import graphite.functions.custom.cd_ssv as cd_ssv
import graphite.functions.custom.recovery as recovery
from graphite.render.datalib import TimeSeries
import numpy as np



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
	rec_time, iter, rmse, rec_mat = recovery.recovery(matrix, n, m, m - 1, 0.2, 2)

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

SeriesFunctions = {
        'kmeans': applyKmeans,
	'cd' : applyCD,
	'recov': applyRecov,
}
