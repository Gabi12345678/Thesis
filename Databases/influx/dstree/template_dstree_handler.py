import sys
import numpy as np
import random
from datetime import datetime
sys.path.append("<kapacitor_library>")
sys.path.append("<implementation>")

from kapacitor.udf.agent import Agent, Handler
from kapacitor.udf import udf_pb2
import logging
from dstree.dynamicsplit import IndexBuilder, IndexExactSearcher

#output the logs in kapacitor
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(name)s: udf: %(message)s')
logger = logging.getLogger()

class DSTreeHandler(Handler):
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
    self.datapoints = []
    self.begin_req = begin_req
    logger.info("begin batch")

  def point(self, point):
    columns = len(point.fieldsDouble)
    current_point = np.zeros(columns)
    for v in range(columns):
      current_point[v] = point.fieldsDouble["dim"+str(v)]
    self.datapoints.append(current_point)

  def end_batch(self, end_req):
    logger.info("end_batch")
    response = udf_pb2.Response()
    #send back a begin batch to kapacitor
    response.begin.CopyFrom(self.begin_req)
    self.agent.write_response(response)

    start_time = datetime.now()
    matrix = np.array(self.datapoints)
    n = matrix.shape[0]
    m = matrix.shape[1]
    logger.info("Start DSTree")
    distances = IndexExactSearcher.search(matrix.T, "<index_path>.idx_dyn_100_1_" + str(m))
    logger.info("End DSTree")
    end_time = datetime.now()
    t = (end_time - start_time).total_seconds()
    response = udf_pb2.Response()
    response.point.fieldsDouble["DSTreeTime"] = t
    response.point.time = int((datetime.now() - datetime(1970,1,1)).total_seconds() * 1000000000)
    self.agent.write_response(response)

    #29 feb for clusters
    time = 1583000000000000000
    for v in distances:
      time = time + 10000000000
      response = udf_pb2.Response()
      response.point.time = time
      response.point.fieldsDouble["dist"] = v
      self.agent.write_response(response)

    response = udf_pb2.Response()
    response.end.CopyFrom(end_req)
    self.agent.write_response(response)
    logger.info(str(distances))

    output_file = open("<output_file>", "a")
    output_file.write(str(matrix.shape) + " -- " + str((end_time - start_time).total_seconds()) + "\n")

if __name__ == '__main__':
  agent = Agent()
  handler = DSTreeHandler(agent)
  agent.handler = handler

  logger.info('Starting agent for DSTreeDecomposition')
  agent.start()
  agent.wait()
  logger.info('DSTree agent finished')
