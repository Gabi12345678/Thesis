# Python ZNormalization implementation

The directory contains the Python implementation of the ZNormalization for normalization.

Use the [run.py](../run.py) algorithm found in the parent directory, where the possible parameters are:

* _datafile_ - Path to the file containg the data. The format of the file is: space separated, one timestamp per line.
* _outfile_ - Path to the file where to store the result. After the run, the file will contain the normalized data - space separated, one timestamp per line.

Example usage,

```bash
./run.py --algorithm znormalization --datafile ./data.txt --outfile ./results/znorm.txt
```
