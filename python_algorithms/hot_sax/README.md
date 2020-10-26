# Python HOTSAX implementation

The directory contains the Python implementation of the HOTSAX algorithm for Anomaly Detection.

Use the [run.py](../run.py) algorithm found in the parent directory, where the possible parameters are:

* _datafile_ - Path to the file containg the data. The format of the file is: space separated, one timestamp per line.
* _saxwindow_ - Size of the window. 
* _outfile_ - Path to the file where to store the result. After the run, the file will contain the anomalies. Each anomaly is a tuple of 3 elements: the timeseries index, the timestamp index of the start of the anomaly and the anomaly metric value.

Example usage,

```bash
./run.py --algorithm hotsax --datafile ./data.txt --saxwindow 40 \
	--outfile ./results/hotsax.txt
```
