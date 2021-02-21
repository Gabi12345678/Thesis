# Influx

___
## Script details:
- The scripts will start the Influx components. If the script has an error or is force stopped, please make sure that the components are turned down.
- The script will write the UDF runtimes in a file ```time.txt``` in the same directory.
- In order to view the results, please start Influxd in a separate terminal, and then connect to it via Influx. You can then query the results directly from the database.

___
## To add a new UDF

Copy the kmeans experiment and change generate\_udf.py, template\_\*.py, udf.tick accordingly.
___
## To add a new built-in operator

Copy the records-select-long experiment and change generate\_udf.py, template\_\*.py, udf.tick accordingly.