# Script to run individual algorithms directly from files

## Python requirements
```bash
pip3 install -r requirements.txt
```

## Running an algoirthm

Simple usage
```bash
./run.py --algorithm <algorithm_name> --datafile <filepath>
```

Example for KMeans
```bash
./run.py --algorithm kmeans --datafile ./data.txt
```

Some algorithm might require additional parameters. Please consult script help if unsure
```bash
./run.py --help
```

Example of setting additional parameters for KMeans
```bash
./run.py --algorithm kmeans --clusters 20 --iterations 10
```

## Running all algorithms

A script that runs all algorithms at once is also provided.
```bash
./run_all.sh
```

## Note
Note that the data provided in this directory only serves as an example and was hand-generated for testing purposes.

