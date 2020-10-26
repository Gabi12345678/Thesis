# Python KMeans implementation

The directory contains the Python implementation of the KMeans algorithm for clustering.

Use the [run.py](../run.py) algorithm found in the parent directory, where the possible parameters are:

* _datafile_ - Path to the file containg the data. The format of the file is: space separated, one timestamp per line.
* _clusters_ - The number of clusters.
* _iterations_ - The number of iterations that KMeans will do. 
* _outfile_ - Path to the file where to store the result. After the run, the file will contain the clusters, one per line.

Example usage,

```bash
./run.py --algorithm kmeans --datafile ./data.txt --clusters 10 \
	--iterations 20 --outfile ./results/kmeans.txt
```
