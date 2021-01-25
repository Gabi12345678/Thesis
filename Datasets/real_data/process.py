import os
import itertools
import numpy as np
import datetime
import sys

stations = ["bs", "cedarpoint", "disl", "katrinacut", "meaher", "middlebay", "perdido"]
years = range(int(sys.argv[1]), int(sys.argv[2]))

unique_cols = []
for s, y in itertools.product( stations, years ):
    fp = s + "_met_" + str(y) + ".txt"
    f = open("./data/" + fp, "r")
    cols = f.readline()[:-1].split(",")
    for c in cols:
        if c in ["tabid", "yeardata", "jday", "timedata", "stationid", "battery", "comments"]:
            continue
        if "Flag" in c:
            continue
        if "Direction" in c:
            continue
        unique_cols.append(s + "_" + c)

unique_cols = set(unique_cols)
order_cols = {}
i = 0
for x in unique_cols:
    order_cols[x] = i
    i = i + 1

data = {}
for s, y in itertools.product( stations, years ):
    fp = s + "_met_" + str(y) + ".txt"
    f = open("./data/" + fp, "r")
    print(fp)
    cols = f.readline()[:-1].split(",")
    
    for l in f:
        sp = l[:-1].split(",")
        if sp[3] == "2400":
            sp[3] = "0000"
            sp[2] = str(int(sp[2]) + 1)
        while len(sp[2]) < 2:
            sp[2] = "0" + sp[2]
        while len(sp[3]) < 4:
            sp[3] = "0" + sp[3] 
        t = sp[1] + "-" + sp[2] + "-" + sp[3]
        if not( t in data ):
            data[t] = np.zeros(len(unique_cols))
            data[t][:] = np.nan
 
        for j in range(len(cols)):
            if (s + "_" + cols[j]) in order_cols:
                try:
                    data[t][order_cols[s + "_" + cols[j]]] = float(sp[j])
                except ValueError:
                    continue
     

l = []
for x in unique_cols:
   l.append( (x, sum([1 for t in data if not( np.isnan( data[t][order_cols[x]] ) ) ])) )
print(l)

to_del = []
for x in data:
    if np.isnan(data[x]).any():
        print(unique_cols)
        print(data[x])
        print(order_cols)
        sys.exit()
        to_del.append(x)


for x in to_del:
    del data[x]

print(len(unique_cols))
print(len(data))
w = open("data.txt", "w")
to_write = []
for x in sorted(list(data.keys())):
    sp = x.split("-")
    t = datetime.datetime.strptime(sp[0] + "-1-1 " + sp[2][:2] + ":" + sp[2][2:], "%Y-%m-%d %H:%M") + datetime.timedelta(int(sp[1]) - 1)
    to_write.append(','.join([ t.strftime("%Y-%m-%dT%H:%M") ] + [ str(v) for v in data[x] ]))


w.write('\n'.join(to_write))

