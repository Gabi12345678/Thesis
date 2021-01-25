import sys
import numpy as np
import random
from datetime import datetime
sys.path.append("<kapacitor_library>")

from kapacitor.udf.agent import Agent, Handler
from kapacitor.udf import udf_pb2
import logging
from queue import PriorityQueue

#output the logs in kapacitor
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s:%(name)s: udf: %(message)s')
logger = logging.getLogger()

class KnnHandler(Handler):
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
    
    q = {}
    a = self.points
    for i in range(len(a)):
      current_time = a[i].fieldsInt['original_time']
      if not(current_time in a):
        q[current_time] = PriorityQueue()
      q[current_time].put( (-a[i].fieldsDouble['dist'], a[i].fieldsDouble['label']) )
      while q[current_time].qsize() > 3:
        q[current_time].get()

    for t in q:
      a = q[t]
      l_size = a.qsize()
      l = [0] * l_size
      for i in range(l_size):
        l[i] = a.get()[1]
      response = udf_pb2.Response()
      response.point.time = int(t)
      response.point.fieldsDouble['label'] = max(set(l), key = l.count)
      self.agent.write_response(response)
    
    response = udf_pb2.Response()
    response.end.CopyFrom(end_req)
    self.agent.write_response(response)
    logger.info("end batch")

if __name__ == '__main__':
  agent = Agent()
  handler = KnnHandler(agent)
  agent.handler = handler

  logger.info('Starting agent for KnnProduct')
  agent.start()
  agent.wait()
  logger.info('KnnProduct agent stopped')
