# Python RecovDB implementation

The directory contains the Python implementation of the RecovDB for Recovery of Missing Values.

Use the [run.py](../run.py) algorithm found in the parent directory, where the possible parameters are:

* _datafile_ - Path to the file containg the data. The format of the file is: space separated, one timestamp per line.
* _trunc_col_ - The number of columns to truncate in the Recov algorithm.
* _col_drop_ - When data is not missing, then artificially remove data from the first col_drop columns. Used together with _perc_.
* _perc_ - When data is not missing, then artificially remove perc percentage of the data. Used together with _col_drop_.
* _outfile_ - Path to the file where to store the result. After the run, the file will contain the recovered data - space separated, one timestamp per line.

Example usage,

```bash
./run.py --algorithm recov --datafile ./data.txt \
	--trunc_col 45 --perc 0.1 --col_drop 2 \
	--outfile ./results/recov.txt
```
