# Python CentroidDecomposition implementation

The directory contains the Python implementation of the CentroidDecomposition algorithm for data summarization.

Use the [run.py](../run.py) algorithm found in the parent directory, where the possible parameters are:

* _datafile_ - Path to the file containg the data. The format of the file is: space separated, one timestamp per line.
* _outfile_ - Path to the file where to store the result. After the run, the file will contain the R matrix of the algorithm.

Example usage,

```bash
./run.py --algorithm cd --datafile ./data.txt --outfile ./results/cd.txt
```
