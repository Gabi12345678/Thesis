# Datasets

## Install

The install script can be used to download the data files. This script is called from the top-level install script, so only do this if something happened with the files.

```bash
	$ sh install.sh
```

## Synthetic files

The directory contains two data files with synthetic data.
- **synth_1K** - Containg 1000 measurements of 1000 samples each.
- **synth_1M** - Containing 100 measurements of 1,000,000 samples each.
Please note that the format of the file is space-separated and each line contains one sample for each of the measurement - corresponding to the samples at the respective time.
