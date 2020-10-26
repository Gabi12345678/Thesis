# Python Screen implementation

The directory contains the Python implementation of the Screen algorithm for Anomaly Repairing.

Use the [run.py](../run.py) algorithm found in the parent directory, where the possible parameters are:

* _datafile_ - Path to the file containg the data. The format of the file is: space separated, one timestamp per line.
* _timestampfile_ - Path to the file containing the timestamps, one per line.
* _smax_ - Maximum allowed speed.
* _smin_ - Minimum allowed speed. Usually negative value.
* _screenwindow_ - Window for speed calculation.
* _outfile_ - Path to the file where to store the result. After the run, the file will contain the data after the repair - space separated, one timestamp per line.

Example usage,

```bash
./run.py --algorithm screen --datafile ./data.txt --timestampfile ./timestamps.txt \
	--smax 0.01 --smin -0.01 --screenwindow 5 --outfile ./results/screen.txt
```
