# Graphite

___
## Script details:
- The script will output the results to the console.
- If you want to review the results of the udf, just print the HTTP response in the ```generate_udf.py``` to console.

___
## Working with datasets of different  granularities

The four real-world datasets used in this work have already the granularities set up. To add a different granularity add in for-install/storage-schemas.conf the pattern and retention.

___
## To add a new UDF

Recommend using kmeans experiment as an example. First, add the UDF in for-install/udf.py, then copy and alter the kmeans/generate_udf.py accordingly. Note that you need to re-install graphite after making the changes. This might take some time, as Graphite has one of the slowest install times.
___
## To add a new built-in operator

Copy the records-select-long experiment and change generate_udf.py accordingly.