import sys
sys.path.append('/home/gabi/Algorithm/screen_python/')
import numpy as np
import screen

def dist(a, b):
	x = np.array(a) - np.array(b)
	return np.sqrt(np.mean(x ** 2))

l = open("stock10k.data", "r").readlines()
l = [x[:-1].split(',') for x in l]

matrix = np.zeros((len(l), 1))
for i in range(len(l)):
	matrix[i][0] = float(l[i][1])
timestamps = [int(x[0]) for x in l]
matrixR = screen.screen(matrix, timestamps, 6, -6, 1)
print(matrixR.shape)

modif_truth = [float(x[2]) for x in l]
modif_actual = [matrixR[i][0] for i in range(len(l))]

print("Final RMS:", dist(modif_actual, modif_truth))
