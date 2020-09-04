define({"343":{i:0.000133301487174907,u:"../Content/Users_Guides/Core/UDA/Programming.htm",a:"UDA Programming As with all eXtremeDB  applications, the runtime must be started and initialized, memory devices defined and an error handler mapped.  Then the Meta-dictionary is initialized and the database opened with  mco_uda_db_open() .  The following example demonstrates a typical sequence for ...",t:"UDA Programming"},"344":{i:0.000133301487174907,u:"../Content/Users_Guides/Core/UDA/Sequences.htm",a:"To support fields of type  sequence, three flags are accepted in mco_dict_field_info_t::flags :  \n    #define MCO_DICT_FI_ASC_SEQUENCE       0x100\n    #define MCO_DICT_FI_DESC_SEQUENCE      0x200\n    #define MCO_DICT_FI_SEQUENCE           0x400\n      If the field type is defined as  sequence the  ...",t:"UDA Sequence Support"},"345":{i:0.000502000202862242,u:"../Content/Users_Guides/Core/Web_Services.htm",a:"eXtremeDB provides Web Services using the popular REST ( Representational State Transfer ) protocol. The REST server can be accessed using any language that supports HTTP and JSON, which includes but is not limited to C/C++, Python, Java and C#. Note that the REST server is simply an HTTP server ...",t:"eXtremeDB Web Services"},"346":{i:0.00488965882670557,u:"../Content/Users_Guides/Core/Web_Services/McoREST_Resources_Responses.htm",a:"The eXtremeDBWeb Service Resources The mcorest library exposes a REST-like interface to access a number of eXtremeDB resources. The response data is encoded using JSON notation.  For an overview see page  eXtremeDB Web Services In order to enable access to the eXtremeDB Web Services, the REST server ...",t:"The eXtremeDB Web Service Resources"},"347":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/api_resource.htm",a:"The root api resource lists eXtremeDB version information and the paths of the registered web services.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will return a list of the available web services:  \n    ...",t:"The api Resource"},"348":{i:0.000351124436974035,u:"../Content/Users_Guides/Core/Web_Services/db_resource.htm",a:"The api/db resource lists the names of the available databases.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will return a list of the available databases:  \n    http://localhost:8083/api/db\n            \n ...",t:"The api/db Resource"},"349":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/db_stat_resource.htm",a:"The api/db/\u003cdb_name\u003e/stat resource displays database statistics for the specified database. For an overview see page  eXtremeDB Web Service Resources In order to enable access to the eXtremeDB Web Services, the REST server must be started. When using xSQL to start the REST server, the rest section ...",t:"The api/db/\u003cdb_name\u003e/stat Resource"},"350":{i:0.000410233749280596,u:"../Content/Users_Guides/Core/Web_Services/db_classes_resource.htm",a:"The api/db/\u003cdbname\u003e/classes resource shows the list of database classes.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will return a list of the available classes:  \n    ...",t:"The api/db/\u003cdbname\u003e/classes Resource"},"351":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/db_structs_resource.htm",a:"The api/db/\u003cdbname\u003e/structs resource shows the list of database structs.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will return a list of the available structs:  \n    ...",t:"The api/db/\u003cdbname\u003e/structs Resource"},"352":{i:0.000427239403318215,u:"../Content/Users_Guides/Core/Web_Services/db_schema_resource.htm",a:"The api/db/\u003cdbname\u003e/schema resource shows the schema of the specified database.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will return the database schema in JSON format:  \n    ...",t:"The api/db/\u003cdbname\u003e/schema Resource"},"353":{i:0.00049184331890122,u:"../Content/Users_Guides/Core/Web_Services/db_classes_struct_no_resource.htm",a:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e resource executes the following operations for HTTP GET, POST and DELETE: GET: Shows the schema for the specified struct_no POST: Writes records to the database DELETE: Deletes all records of the specified class For an overview see page  eXtremeDB Web Service ...",t:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e Resource"},"354":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/db_structs_struct_no_resource.htm",a:"The api/db/\u003cdbname\u003e/structs/\u003cstruct_no\u003e resource returns the schema of the specified struct. For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will display the schema for the specified struct:  \n    ...",t:"The api/db/\u003cdbname\u003e/structs/\u003cstruct_no\u003e Resource"},"355":{i:0.00049184331890122,u:"../Content/Users_Guides/Core/Web_Services/db_classes_struct_no_byindex_index_no_list_resource.htm",a:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e/byindex/\u003cindex_no\u003e/list resource executes the following operations for HTTP GET and DELETE: GET: Returns record(s) of the specified class using the index specified by index_no DELETE: Deletes individual objects, as specified by the query arguments For an ...",t:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e/byindex/\u003cindex_no\u003e/list Resource"},"356":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/db_classes_struct_no_byindex_index_no_eq_key_resource.htm",a:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e/byindex/\u003cindex_no\u003e/eq/\u003ckey\u003e resource executes the following operations for HTTP GET, DELETE and PATCH: GET: Returns record(s) of the specified class using the index specified by index_no DELETE: Deletes individual objects whose index_no matches the specified ...",t:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e/byindex/\u003cindex_no\u003e/eq/\u003ckey\u003e Resource"},"357":{i:0.00012163265184034,u:"../Content/Users_Guides/Core/Web_Services/db_classes_struct_no_byindex_index_no_eq_key_field_resource.htm",a:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e/byindex/\u003cindex_no\u003e/eq/\u003ckey\u003e/field/\u003cfield_no\u003e resource allows individual field access. The field is identified by its number, field_no, which can be retrieved from the class schema. The index referred to by index_no is required to be unique. It executes the ...",t:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e/byindex/\u003cindex_no\u003e/eq/\u003ckey\u003e/field/\u003cfield_no\u003e Resource"},"358":{i:0.000310340739316499,u:"../Content/Users_Guides/Core/Web_Services/db_classes_struct_no_byindex_index_no_eq_key_field_at_elem_resource.htm",a:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e/byindex/\u003cindex_no\u003e/eq/\u003ckey\u003e/field/\u003cfield_no\u003e/at/\u003celem_index\u003e resource allows element access for arrays, vectors, sequences, and structures. The field is identified by its number, field_no, which can be retrieved from the class schema. The index referred to by ...",t:"The api/db/\u003cdbname\u003e/classes/\u003cstruct_no\u003e/byindex/\u003cindex_no\u003e/eq/\u003ckey\u003e/field/\u003cfield_no\u003e/at/\u003celem_index\u003e Resource"},"359":{i:0.000702703950701333,u:"../Content/Users_Guides/Core/Web_Services/perf_resource.htm",a:"The api/perf resource reports the performance statistics described below.  For an overview see page  eXtremeDB Web Service Resources In order to enable access to eXtremeDB performance statistics the performance monitor must be enabled and the REST server must be started. When using xSQL to start the ...",t:"The api/perf Resource"},"360":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/sql_stat_resource.htm",a:"The api/sql/stat resource displays database statistics for the SQL engine managing the database(s) and remote clients. For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will display statistics for the SQL ...",t:"The api/sql/stat Resource"},"361":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/sql_stat_sessions_resource.htm",a:"The api/sql/stat/sessions resource displays statistics for current SQL sessions. For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will display statistics for SQL client sessions:   \n    ...",t:"The api/sql/stat/sessions Resource"},"362":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/sql_exec_resource.htm",a:"The api/sql/exec resource is requested using an HTTP POST with a SQL statement or statements to be executed, and a response is returned that consists of a query result set or the number of rows affected depending on the SQL statement(s) executed. For an overview see page  eXtremeDB Web Service ...",t:"The api/sql/exec Resource"},"363":{i:0.000360539649241062,u:"../Content/Users_Guides/Core/Web_Services/trace.htm",a:"The api/trace resource returns a fragment of the trace file.  For an overview see page  eXtremeDB Web Service Resources To define the size and the location of the trace fragment, the following query arguments can be specified: count - return the last count bytes from the trace file from and, ...",t:"The api/trace Resource"},"364":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/cluster_restlist_resource.htm",a:"The api/cluster/restlist resource lists the array of nodes in the cluster network.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will return a list of the nodes in the cluster network:  \n    ...",t:"The api/cluster/restlist Resource"},"365":{i:0.000360539649241062,u:"../Content/Users_Guides/Core/Web_Services/ha_restlist_resource.htm",a:"The api/ha/restlist resource lists the array of nodes in the High Availability network.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will return a list of the nodes in the HA network:  \n    ...",t:"The api/ha/restlist Resource"},"366":{i:0.000360539649241062,u:"../Content/Users_Guides/Core/Web_Services/iot_restlist_resource.htm",a:"The api/iot/restlist resource lists the array of nodes in the IoT network.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will return a list of the nodes in the IoT network:  \n    ...",t:"The api/iot/restlist Resource"},"367":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/db_backup_control_resource.htm",a:"The api/db/\u003cdbname\u003e/backup/control resource implements the asynchronous backup and restore control.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will execute one of the backup or restore start or stop ...",t:"The api/db/\u003cdbname\u003e/backup/control Resource"},"368":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/db_backup_status_resource.htm",a:"The api/db/\u003cdbname\u003e/backup/status resource returns the current status of the backup procedure.  For an overview see page  eXtremeDB Web Service Resources With the REST server started (listening on port 8083), the following http request will return the current status of the backup procedure:  \n    ...",t:"The api/db/\u003cdbname\u003e/backup/status Resource"},"369":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/db_backup_list_resource.htm",a:"The api/db/\u003cdbname\u003e/backup/list prints the contents of a backup file. The only supported method is GET. The response is a JSON object with only one key, \"backup_list\", that contains the list of the backup records in the file. (The entries in this list have the same fields as the mco_backup_info_t C ...",t:"The api/db/\u003cdbname\u003e/backup/list Resource"},"370":{i:0.000287882656307989,u:"../Content/Users_Guides/Core/Web_Services/db_backup_verify_resource.htm",a:"The api/db/\u003cdbname\u003e/backup/verify performs synchronous verification of a backup file. The only supported method is GET. Note that the operation is performed in a synchronous manner, and can take some time if the backup file is large. The following query parameters are supported (which correspond to ...",t:"The api/db/\u003cdbname\u003e/backup/verify Resource"},"371":{i:0.000137388441178233,u:"../Content/Users_Guides/Core/Database_Browser.htm",a:"The eXtremeDB SDK includes an HTML-based database browser / editor (HTTP Viewer) that can be used to examine any eXtremeDB  database. The browser is implemented via C API libraries which comprise a layer between the UDA API and the generic C APIs of the eXtremeDB database. The browser interface can ...",t:"Database Browser"},"372":{i:0.000297031116871025,u:"../Content/Users_Guides/Core/Database_Browser/Database_Browser_C.htm",a:"As explained in the Database Browser page, the eXtremeDB SDK includes an HTML-based database browser / editor (HTTP Viewer) that can be used to examine any eXtremeDB database.  The browser is implemented via a special library mcohv , which is a layer between the UDA API and the generic C APIs of the ...",t:"Database Browser C APIs"},"373":{i:0.000532982525305303,u:"../Content/Users_Guides/Server_Users_Guide.htm",a:"eXtremeDB for HPC builds on the proven high performance database implementation of eXtremeDB  for in-memory and persistent database solutions. eXtremeDB for HPC includes the embedded SQL APIs and the client-server tool xSQL, robust system recovery and data relay capability with eXtremeDB Transaction ...",t:"eXtremeDB for HPC User\u0027s Guide"},"374":{i:0.00144980279163365,u:"../Content/Users_Guides/xSQL_Users_Guide.htm",a:"As explained in the  xSQL Product Overview , xSQL is an interactive tool for processing SQL statements that can be run as server or client to access eXtremeDB databases. For developers new to xSQL we highly recommend the  quick start  tutorial to quickly get up and running with this powerful tool. ...",t:"xSQL User\u0027s Guide"},"375":{i:0.00128533184891016,u:"../Content/Users_Guides/SQL_Users_Guide.htm",a:"McObject \u0027s eXtremeSQL is a high-performance implementation of the SQL programming language for interfacing with eXtremeDB databases. The  SQL Introduction  and  Product Overview  pages provide a detailed introduction to eXtremeSQL features. The eXtremeSQL API is implemented as a C language library ...",t:"eXtremeSQL User\u0027s Guide"},"376":{i:0.000952224790453863,u:"../Content/Users_Guides/Server/Performance_Monitor.htm",a:"eXtremeDB for HPC includes an HTML-based Performance Monitor that utilizes the Javascript tool Highcharts from Highsoft.  Included is one licensed run-time copy with each eXtremeDB for HPC SDK purchased.  (If more run-time licenses are required, you must purchase them from  http://www.highsoft.com ...",t:"The eXtremeDB Performance Monitor"},"377":{i:0.000283512377439189,u:"../Content/Users_Guides/Server/Perfornamce_Monitor/Perfmon_C.htm",a:"The Performance Monitor C API consists of the functions described below which are intended to be incorporated into an application  that initializes the monitor, attaches an eXtremeDB database to be monitored, then detaches and closes the monitor. The application first initializes the monitor by ...",t:"eXtremeDB Performance Monitor C API"},"378":{i:0.000283512377439189,u:"../Content/Users_Guides/Server/Perfornamce_Monitor/Perfmon_Python.htm",a:"The Performance Monitor Python API is implemented in class  exdb.Perfmon . To use Perfmon in Python, first specify UsePerfmon=True in the  exdb.init_runtime() parameters:  \n    exdb.init_runtime(disk=is_disk, tmgr=options.tmgr, shm=is_shm, debug=is_debug, UsePerfmon=True)\n      Then initialize the ...",t:"eXtremeDB Performance Monitor Python API"},"379":{i:0.000283512377439189,u:"../Content/Users_Guides/Server/Perfornamce_Monitor/Perfmon_xSQL.htm",a:"The Performance Monitor can be run from xSQL by setting the following parameters in the startup config file :  \n    perfmon :\n    {\n        # Use mcohv\n        hv : true,\n        #IM database size for perfmon\n        database_size : 64m,\n        #Set listening address and port for embedded HTTP ...",t:"eXtremeDB Performance Monitor xSQL API"},"380":{i:0.00077185764534207,u:"../Content/Users_Guides/Server/Analytics_Function_Library.htm",a:"The eXtremeDB Vector-based Analytics functions library is a set of statistical / math functions and operators designed to operate on the  sequence data type. Generally these functions accept sequence iterators (or, in some cases, constants) as parameters and create another sequence iterator as a ...",t:"Analytics Functions Library"},"381":{i:0.000736595651757945,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples.htm",a:" The table below lists a number of examples using the Vector-based Analytics Functions library. The first example demonstrates how a C API \"pipeline\" of function calls can be used to compute closing prices adjusting for splits; then compares this to the SQL select statement which performs the same ...",t:"Analytics Functions Library Examples"},"382":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example1.htm",a:"Historical Closing Prices Adjusting for Splits The following code snippets demonstrate how to adjust the historical closing prices of IBM stocks for the effect of splits. First, in the following C API code snippet, assume that \u0026sec is an object handle of an instance of a class representing the ...",t:"Analytics Functions Example 1: Splits"},"383":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example2.htm",a:"Volume Weighted Average Buy and Sell Prices Given a table of trades with price and volume, the following SQL select statement will compare each trade price Pn against its previous one Pn-1. If Pn-1 \u003e Pn, it is a buy initiated trade; if Pn-1 \u003c Pn, it is a sell initiated trade; if Pn-1 = Pn, it will ...",t:"Analytics Functions Example 2: Weighted Average"},"384":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example3.htm",a:"Sequence Interval Extract data belonging to a specified interval (January 2013). The ordered sequence day is used to select the interval and then this interval is projected to the other (unordered) sequences open and close :  \n    SELECT flattened symbol, seq_search(day, 20130101, 20130131) as ...",t:"Analytics Functions Example 3: Intervals"},"385":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example4.htm",a:"Arithmetic on Sequences Perform arithmetic operations on sequence elements. These two sequences are averaged by adding each element of high to the corresponding element of  low and dividing by 2 to produce a result sequence:  \n    SELECT flattened symbol, seq_search(day, 20130101, 20130131) as ...",t:"Analytics Functions Example 4: Sequence Arithmetic"},"386":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example5.htm",a:"Filter Sequence Elements Filter elements of a sequence - extract the days of February 2013 when the  close value was greater than open:  \n    SELECT symbol, seq_search(day, 20130201, 20130228) as Feb_2013,\n        close@Feb_2013 as Feb_Close, open@Feb_2013 as Feb_Open,\n        seq_filter(Feb_Close \u003e ...",t:"Analytics Functions Example 5: Filter"},"387":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example6.htm",a:"Correlation Perform a correlation calculation for two sequences - calculate the correlation between low and high values:  \n    SELECT symbol, high, low, seq_corr(low, high) as “correlation”\n    FROM Quote WHERE symbol=\u0027IBM\u0027;\n     \n    symbol\n    high{}\n    low{}\n    correlation\n    ...",t:"Analytics Functions Example 6: Correlation"},"388":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example7.htm",a:"Grand Aggregate Maximum Get the maximum close value for IBM in the entire dataset (the first quarter of 2013):  \n    SELECT symbol, close, seq_max(close) FROM Quote WHERE symbol=\u0027IBM\u0027;\n     \n    symbol\n    close{}\n    max\n    ...",t:"Analytics Functions Example 7: Grand Aggregate"},"389":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example8.htm",a:"Window Aggregate Compute the 5 day moving average close for March 2013:  \n    SELECT symbol, \n        seq_search(day, 20130225, 20130331) as Mar_2013,\n        close@Mar_2013 as \"close\",\n        seq_window_agg_avg(close@Mar_2013, 5) as \"5-day-average\"\n    FROM Quote WHERE symbol=\u0027IBM\u0027;\n     \n    ...",t:"Analytics Functions Example 8: Window Aggregate"},"390":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example9.htm",a:"Grid Aggregate Get the minimum close value for 5-day intervals in March 2013:      \n    SELECT symbol, \n        seq_search(day, 20130301, 20130331) as Mar_2013,\n        close@Mar_2013 as \"close\",\n        seq_grid_agg_min(close@Mar_2013, 5) as \"min\"\n    FROM Quote WHERE symbol=\u0027IBM\u0027;\n     \n    ...",t:"Analytics Functions Example 9: Grid Aggregate"},"391":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example10.htm",a:"Group Aggregate Sum Get the total volume for each month of the first quarter of 2013:      \n    SELECT symbol, \n        seq_search(day, 20130101, 20130331) as trade_date,\n        volume@trade_date as \"volume\",\n        seq_group_agg_sum(volume@trade_date, trade_date/100) as \"month_vol\"\n    FROM Quote ...",t:"Analytics Functions Example 10: Group Aggregate"},"392":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example11.htm",a:"Hash Aggregate Average Compute the average close value for groups of trades having volume values grouped by millions of trades per day (for the first two weeks of March 2013):      \n    SELECT symbol, \n        seq_search(day, 20130304, 20130315) as Mar_2013,\n        volume@Mar_2013 as \"volume\", ...",t:"Analytics Functions Example 11: Hash Aggregate"},"393":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example12.htm",a:" Sort Sequence Elements Show close and volume values in descending order by volume:      \n    SELECT symbol, \n        seq_sort(volume, \u0027desc\u0027) as desc_volumes, \n        close@desc_volumes\n    FROM Quote WHERE symbol=\u0027SYM0\u0027;\n \n    symbol\n    desc_volumes{}\n    close{}\n    ...",t:"Analytics Functions Example 12: Sort "},"394":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example13.htm",a:" Top Values Show the top 10 close and corresponding volume values:  \n    SELECT symbol, \n        seq_top_pos_max(close, 10) as top_pos,\n        close@top_pos as \"top-10-close\", \n        volume@top_pos as \"top-10-volume\"\n    FROM Quote WHERE symbol=\u0027IBM\u0027;\n \n    symbol\n    top_pos{}\n    ...",t:"Analytics Functions Example 13: Top "},"395":{i:0.000152938148341156,u:"../Content/Users_Guides/Server/Analytics_Functions/Examples/Example14.htm",a:"Value Weighted Average Show the volume weighted average close prices for the last week of March 2013:  \n    SELECT symbol, \n        seq_search(day, \n        seq_last_int(day)-5, \n        seq_last_int(day)) as last_week,\n        seq_sum(close@last_week*volume@last_week)/seq_sum(volume@last_week) as ...",t:"Analytics Functions Example 14: Value Weighted Average"},"396":{i:0.000224328119776276,u:"../Content/Users_Guides/xSQL/xSQL_Product_Description.htm",a:"xSQL is an interactive tool for processing SQL statements that can be run as server or client to access eXtremeDB databases. The  Getting Started  pages give a tutorial presentation providing a basic introduction to xSQL. Users new to eXtremeSQL should start  here  to become familiar with xSQL, the ...",t:"xSQL Product Overview"},"397":{i:0.00103639380875589,u:"../Content/Users_Guides/xSQL/xSQL_Startup_Options.htm",a:"xSQL is a highly configurable utility that can run SQL scripts on local and/or remote databases, using conventional or shared memory or persistent storage, with various eXtremeDB  runtime options including the choice of  MVCC or  MURSIW transaction managers, and debug or release libraries.  xSQL can ...",t:"xSQL Startup Options"},"398":{i:0.000477915931665564,u:"../Content/Users_Guides/xSQL/Startup_Options/xSQL_Command_Line_Options.htm",a:"xSQL Command Line Options As explained in  xSQL Startup Options  there are a variety of configuration parameters that determine xSQL behavior. The following table defines the options that can be specified on the command line. Note that command line options have precedence over the configuration file ...",t:"Command Line Options"},"399":{i:0.00447678889238437,u:"../Content/Users_Guides/xSQL/Startup_Options/xSQL_Config_File_Options.htm",a:"xSQL Configuration File Parameters As explained in  xSQL Startup Options  there are a variety of configuration parameters that determine xSQL behavior. The following table defines the options that can be specified in the configuration file. The configuration file uses JSON format with some ...",t:"Configuration File Parameters"},"400":{i:0.000248476692313598,u:"../Content/Users_Guides/xSQL/Startup_Options/Config_File_Sections/Runtime_Config.htm",a:"xSQL  Runtime Configuration Options  The xSQL configuration file can be used to define  a number of runtime configuration parameters that determine what eXtremeDB libraries are loaded at startup. For an overview see page  Configuration File Parameters Parameters are specified by name:value pairs ...",t:"Runtime Configuration Options"},"401":{i:0.000248476692313598,u:"../Content/Users_Guides/xSQL/Startup_Options/Config_File_Sections/Runtime_Options.htm",a:"xSQL  Runtime Options Settings   The xSQL configuration file can be used to define a number of  parameters that correspond to eXtremeDB runtime options (set in C API applications by calling  mco_runtime_setoption() ).  For an overview see page  Configuration File Parameters The runtime_options ...",t:"Runtime Options Settings"},"402":{i:0.000248476692313598,u:"../Content/Users_Guides/xSQL/Startup_Options/Config_File_Sections/Devices.htm",a:"xSQL Devices Specification The xSQL configuration file can be used to define an array of memory devices which corresponds to C API  mco_device_t structures.  For an overview see page  Configuration File Parameters The devices section has syntax like the following to define an array:  \n    devices : ...",t:"Devices Specification"},"403":{i:0.000285533460258129,u:"../Content/Users_Guides/xSQL/Startup_Options/Config_File_Sections/Schema.htm",a:"xSQL Schema Definition The xSQL configuration file can be used to define  a database schema.  For an overview see page  Configuration File Parameters The database schema can be defined either by quoting the schema definition in a text string or by specifying a schema file. Schema File The database ...",t:"Schema Definition"},"404":{i:0.000772432294644243,u:"../Content/Users_Guides/xSQL/Startup_Options/Config_File_Sections/Db_Params.htm",a:"xSQL Database Parameters The xSQL configuration file can be used to define  database parameters corresponding to the C API  mco_db_params_t structure.  For an overview see page  Configuration File Parameters The db_params section consists of a set of \"name : value\" pairs enclosed within braces. For ...",t:"Database Parameters"},"405":{i:0.00302428801113381,u:"../Content/Users_Guides/xSQL/Startup_Options/Config_File_Sections/ExtremeSQL_Params.htm",a:"xSQL SQL Parameters The xSQL configuration file can be used to define parameters that  affect SQL operation.  For an overview see page  Configuration File Parameters These parameters consist of \"name : value\" pairs that can appear anywhere in the configuration file outside of other sections. For ...",t:"SQL Parameters"},"406":{i:0.000248476692313598,u:"../Content/Users_Guides/xSQL/Startup_Options/Config_File_Sections/TL_Params.htm",a:"xSQL Transaction Logging Parameters The xSQL configuration file can be used to define eXtremeDB Transaction Logging parameters corresponding to the C API  mco_TL_start_data_t structure.  For an overview see page  Configuration File Parameters The tl_params section consists of a set of \"name : value\" ...",t:"Transaction Logging Parameters"},"407":{i:0.000443611051736478,u:"../Content/Users_Guides/xSQL/Startup_Options/Config_File_Sections/HA_Params.htm",a:"xSQL High Availability Parameters The xSQL configuration file can be used to define eXtremeDB High Availability parameters corresponding to the C API  mco_ha_params_t structure.  For an overview see page  Configuration File Parameters The ha_params section consists of a set of \"name : value\" pairs ...",t:"High Availability Parameters"},});