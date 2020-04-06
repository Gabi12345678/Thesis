import sys
import numpy as np
import random
from datetime import datetime
sys.path.append("/home/gabi/influx/kapacitor-master/udf/agent/py")

from kapacitor.udf.agent import Agent, Handler
from kapacitor.udf import udf_pb2
import logging

#output the logs in kapacitor
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(name)s: %(message)s')
logger = logging.getLogger()

class KMeans:
  def __init__(self, clusters = 10, iterations = 20):
    self.clusters = clusters
    self.iterations = iterations
    self.centers = []
    self.datapoints = []

  def add_datapoint(self, point):
    self.datapoints.append(point)

  def distance(self, point_a, point_b):
    return np.sum(np.square(point_a - point_b))

  def calculate(self):
    self.centers = random.sample(self.datapoints, self.clusters)
    self.columns = self.centers[0].shape[0]

    for iteration in range(self.iterations):
      logger.info("Iteration %s" % iteration)
      total = []
      count = []
      for i in range(self.clusters):
        total.append(np.zeros(self.columns))
        count.append(0.0)

      for point in self.datapoints:
        closest_cluster = 0
        closest_distance = self.distance(point, self.centers[0])
        for i in range(1, self.clusters):
          current_distance = self.distance(point, self.centers[i])
          if current_distance < closest_distance:
            closest_distance = current_distance
            closest_cluster = i
        total[closest_cluster] = total[closest_cluster] + point
        count[closest_cluster] = count[closest_cluster] + 1

      for i in range(self.clusters):
        if count[i] == 0:
          self.centers[i] = random.sample(self.datapoints, 1)[0]
        else:
          self.centers[i] = total[i] / count[i]

class KMeansHandler(Handler):
  def __init__(self, agent):
    self.agent = agent

  def info(self):
    response = udf_pb2.Response()
    response.info.wants = udf_pb2.BATCH
    response.info.provides = udf_pb2.BATCH
    return response
    
  #kapacitor can give aditional paramaeters via init. We don t need any
  def init(self, init_req):
    response = udf_pb2.Response()
    response.init.success = True
    return response

  def snapshot(self):
    response = udf_pb2.Response()
    response.restore.success = False
    response.restore.error = 'not implemented'
    return response

  def restore(self):
    response = udf_pb2.Response()
    response.restore.success = False
    response.restore.error = 'not implemented'
    return response

  def begin_batch(self, begin_req):
    self.kmeans = KMeans()
    self.begin_req = begin_req
    logger.info("begin batch")

  def point(self, point):
    columns = len(point.fieldsDouble)
    current_point = np.zeros(columns)
    for v in range(columns):
      current_point[v] = point.fieldsDouble["dim"+str(v)]
    self.kmeans.add_datapoint(current_point)

  def end_batch(self, end_req):
    response = udf_pb2.Response()
    #send back a begin batch to kapacitor
    response.begin.CopyFrom(self.begin_req)
    self.agent.write_response(response)

    start_time = datetime.now()
    self.kmeans.calculate()
    end_time = datetime.now()
    t = (end_time - start_time).total_seconds()
    response = udf_pb2.Response()
    response.point.fieldsDouble["KMeanstime"] = t
    #seting timestamp for kmeans time
    response.point.time = int((datetime.now() - datetime(1970,1,1)).total_seconds() * 1000000000)
    self.agent.write_response(response)

if __name__ == '__main__':
  agent = Agent()
  handler = KMeansHandler(agent)
  agent.handler = handler

  logger.info('Starting agent for KMeans')
  agent.start()
  agent.wait()
  logger.info('KMeans agent finished')