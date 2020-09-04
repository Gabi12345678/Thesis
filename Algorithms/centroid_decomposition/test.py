import cd_ssv
import numpy as np

a = np.array([[1,2,3],[4,5,6],[7,8,9],[10,11,12]])

L, R, z = cd_ssv.CD(a, 4, 3)
print(a)
print("-" * 100)
print(L)
print("-" * 100)
print(R)
print("-" * 100)
print(z)
