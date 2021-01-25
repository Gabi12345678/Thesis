import sys
import numpy as np
import random
from datetime import datetime
sys.path.append("/home/gabi/Thesis-master/Databases/influx/kapacitor-master/udf/agent/py")

from kapacitor.udf.agent import Agent, Handler
from kapacitor.udf import udf_pb2
import logging

#output the logs in kapacitor
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(name)s: udf: %(message)s')
logger = logging.getLogger()

class CartesianHandler(Handler):
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
    self.points = []
    self.begin_req = begin_req  
    logger.info("begin batch")

  def point(self, point):
    self.points.append(point)
  
  def end_batch(self, end_req):
    response = udf_pb2.Response()
    #send back a begin batch to kapacitor
    response.begin.CopyFrom(self.begin_req)
    self.agent.write_response(response)
    
    lines = len(self.points)
    columns = int(len(self.points[0].fieldsDouble) / 2)
    #logger.info("Dimensions: " + str((lines, columns)))
    current_time = 0
    for i in range(lines):
      for j in range(lines):
        current_point = udf_pb2.Response()
        current_point.point.time = current_time
        current_time = current_time + 10 ** 9
        for t in range(columns):
          current_point.point.fieldsDouble['d' + str(t)] = self.points[i].fieldsDouble['d' + str(t)]
          current_point.point.fieldsDouble['l' + str(t)] = self.points[j].fieldsDouble['l' + str(t)]
        current_point.point.fieldsDouble['label'] = self.points[j].fieldsDouble['label']
        current_point.point.fieldsInt['original_time'] = self.points[i].time
        #logger.info("Point:" + str(current_point))
        self.agent.write_response(current_point)
    
    response = udf_pb2.Response()
    response.end.CopyFrom(end_req)
    self.agent.write_response(response)
    logger.info("end batch")

if __name__ == '__main__':
  agent = Agent()
  handler = CartesianHandler(agent)
  agent.handler = handler

  logger.info('Starting agent for CartianProduct')
  agent.start()
  agent.wait()
  logger.info('CartesianProduct agent stopped')
