import sys
sys.path.append('/home/gabi/Algorithms/screen')
import screen
import numpy

matrix = numpy.random.rand(10, 3)
timestamps = range(10)
screen.screen(matrix, timestamps, 0.01, -0.01, 3)
