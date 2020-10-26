# Python DSTree implementation

The directory contains the Python implementation of the DSTree algorithm for Similarity Search.

Use the [run.py](../run.py) algorithm found in the parent directory, where the possible parameters are:

* _datafile_ - Path to the file containg the data. The format of the file is: space separated, one timeseries per line.
* _threshold_ - The maximum number of timeseries to store inside a node of the tree.
* _indexpath_ - Path where to store the DSTree index.
* _buffermemory_ - Maximum MB of timeseries to keep in memory before flushing them to disk.
* _querypath_ - Path to the file containing the queries. The format of the file is: space separated, one query per line.
* _outfile_ - Path to the file where to store the result. After the run, the file will contain the minimum distance of each query to the dataset, one per line.

Example usage,

```bash
./run.py --algorithm dstree --datafile ./data_transpose.txt \
	--threshold 5 --indexpath ./tmp/index --buffermemory 100 \
	--querypath ./queries.txt --outfile ./results/dstree.txt
```
