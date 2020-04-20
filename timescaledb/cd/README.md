# Running Centroid Decomposition as UDF in TimescaleDB

The `generate_udf.py` is the general script that can be used to run the UDF. It has the following parameters:

- **file** - The path to the dataset file. By default, it takes the file `../../dataset/synth_1K.txt`.
- **lines** - A list of integers representing the number of lines to try out. For each element X in the list, it will take the first X lines in the file. Used together with **columns**
- **columns** - A list of integers representing the number of columns to try out. For each element X in the list, it will take the first X columns in the file. Used together with **lines**
- **start_time** - The EPOCH time in seconds used for the first measurements, when inserting the points into Influx. All measurements after are inserted at a 10 second interval. Default value is 1583000000.

First, `cd` into the directory of the script. Then simply run the script.

Example of script run:
```sh
python3 generate_udf --file ../../synth_1M.txt --lines 100 1000 --columns 10 50
```
This will run the UDF with all possible combination of lines and columns: (100, 10), (100, 50), (1000, 10), (1000, 50).