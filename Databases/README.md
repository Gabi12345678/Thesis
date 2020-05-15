# Databases

## Directory structure

A directory was created for each of the databases analysed. Each of these directories contains the following:
- An installation script that can be used to re-install the individual databases. Note that these are also called from the top level installation script, so only run it if you want to install one database in particular.
```bash
$ cd <database_directory>
$ sh install.sh
```

- Subdirectories for each of the Algorithm analysed. 
	- **cd** for Centroid Decomposition
	- **kmeans** for K-Means

In each of these subdirectories, there is a script that enabled running the individual analysis `generate_udf.py` - algorithm in database. This script can be configured using the following parameters:
	- **file** - The path to the dataset file. By default, it takes the file `Datasets/synth_1K.txt`.
	- **lines** - A list of integers representing the number of samples to try out for each of the measurement. Used together with **columns** to configure the size of the input. If multiple values are provided, the script will try all combinations with columns.
	- **columns** - A list of integers representing the number of measurements to try out. Used together with **lines** to configure the size of the input. If multiple values are provided, the script will try all combinations with lines.
	-  **start_time** - The EPOCH time in seconds used for the first measurements, when inserting the points into Influx. All measurements after are inserted at a 10 second interval. Default value is 1583000000.

To run the script:
```bash
$ cd <database_directory>
$ python3 generate_udf.py
```

## Examples

### Installing
Examples of installing TimescaleDB.
```bash
$ cd timescaledb
$ sh install.sh
```

### Running analysis

This is an example of running Centroid Decomposition algorithm with 50 measurements and 100 data samples for each, on TimescaleDB

```bash
$ cd timescaledb/cd
$ python3 generate_udf.py --lines 100 --columns 50
```
The same can be applied for any other analysis
