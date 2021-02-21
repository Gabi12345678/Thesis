# Druid

___
## Script details:
- Druid gets started and stopped as part of the script. Note that if an error occurs or the script gets force-stopped during the run, the Druid components might stay alive. Make sure to close them before starting a different run.
- Druid was configured for a machine with 125G of RAM. This is done via the configuration files in the ```for-install``` directory. If you want to run Druid with default memory allocation, please comment out the copy of the configuration files in the install script. If you do this, please make sure that you still copy the common file found in ```for-install/conf/druid/single-server/micro-quickstart/_common/``` as these make sure that Druid loads the UDFs.
- The scripts will write the insert and run times to the console.
- If you want to review the response of the UDF, you can do it in the ```generate_udf.py``` file, as the responses are returned as Pandas dataframes.

___
## To add a new UDF

Please follow kmeans as an example. 
Add an Aggregator in udf/src/main/java/master. Then re-install Druid.
Copy and alter the generate_udf.py from KMeans accordingly.
___
## To add a new built-in operator

Please take as an example the z-normalization-operators (for SQL) or simple-queries (for native JSON request) and make the changes in the generate_udf.py.