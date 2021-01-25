import os
import sys
import json
import numpy as np

from dstree.dynamicsplit import *

def f(o):
	return { k : o.__dict__[k] for k in o.__dict__ if not('parent' in k) }

root = IndexBuilder.buildIndex("./data.in", "./out/index", 2, 1, 1000, int(sys.argv[1]))
print(json.dumps(root, default = f, sort_keys = True, indent = 3))


#data = np.genfromtxt("./queries.in", delimiter = ' ')
#print( IndexExactSearcher.search(data, "./out/index.idx_dyn_2_1_5") )
