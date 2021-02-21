# KairosDB-Cassandra

___
## Script details:
- The script will run KairosDB directly in the script. If the script has an error or is force stopped, please make sure to turn KairosDB down before running a new evaluation.
- The script prints the runtimes to the console.
- In order to view the results, then print the HTML response in ```generate_udf.py```.


___
## To add a new UDF

Please follow kmeans as an example. 
Add an aggregator in udf/src/main/java/master. Then re-install KairosDB.
Copy and alter the generate_udf.py from KMeans accordingly.
___
## To add a new built-in operator

Please take as an example the records-select-long and make the changes in the generate_udf.py