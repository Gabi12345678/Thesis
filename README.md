# Thesis

___
## Prerequisites and dependencies

- Ubuntu 18 or higher
- Clone this repository
- All other dependencies will be installed via the install script.

___
## Install

In order to install the databases, go to the Databases directory and run the installation script there.

```bash
$ cd Databases
$ sh install_all.sh
```

For installing one particular database, then go to the directory of that database in the Databases directory and run the install script there. For example,

```bash
$ cd Databases/influx
$ sh install.sh
```

___
## Running eval

Each of the databases has a dedicated directory in Databases. In them, there is a dedicated directory for each experiment. In order to run an experiment, go to the directory of the experiment and use the Python3 script there. For example,

```bash
$ cd Databases/influx/kmeans
$ python3 generate_udf.py
```

Note that the scripts can be configured to run on different datasets and different dimensions. For example,
```bash
$ cd Databases/influx/kmeans
$ python3 generate_udf.py --file <path_to_file> --lines 1000 --columns 50
```