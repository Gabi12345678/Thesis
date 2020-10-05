# Script to run individual algorithms directly from files

___
## Python requirements
```bash
pip3 install -r requirements.txt
```

___
## Running an algoirth

Simple usage
```bash
./run --algorithm <algorithm_name> --datafile <filepath>
```

Example for KMeans
```bash
./run --algorithm kmeans --datafile ./data.txt
```

Some algorithm might require additional parameters. Please consult script help if unsure
```bash
./run --help
```

Example of setting additional parameters for KMeans
```bash
./run --algorithm kmeans --clusters 20 --iterations 10
```

___
## Note
Note that the data provided in this directory only serves as an example and was hand-generated for testing purposes.
