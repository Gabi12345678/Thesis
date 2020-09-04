define({"626":{i:0.000300313642803342,u:"../Content/Programming/C/Structures/Feed_Handler.htm",a:"Feed Handler Types Following is the definition of the possible feed handler API  handle types: Following are enumeration of the possible feed handler API log levels: Following are enumeration of the possible feed handler API node types:",t:"Feed Handler Return Codes and Types"},"627":{i:0.00042577782070238,u:"../Content/Programming/C/Structures/Perfmon_Db.htm",a:"The performance monitor database stores scalar readings in class PerfReading defined as follows:  \n    transient class PerfReading\n    {\n        uint2   node; /* cluster node id, or 0 */\n        uint2   db;\n        uint8   stamp;\n        vector  \u003cuint4\u003e values;\n        tree \u003cnode,db,stamp\u003e rec_idx;\n ...",t:"The Performance Monitor database"},"628":{i:0.00050970392679426,u:"../Content/Programming/C/Structures/Perfmon_Options.htm",a:"The performance monitor database options are defined in structure mco_perf_options_t defined as follows in file mcoperfmon.h:  \n    typedef struct _mco_perf_options_tag\n    {\n        int enabled;\n        int monitor_interval_ms;\n        int monitor_length_sec;\n        int ui_update_rate_ms;\n        ...",t:"The Performance Monitor options"},"629":{i:0.000687297349476717,u:"../Content/Programming/C/Structures/Tracing_Enums.htm",a:"The following eXtremeDB modules are defined by enum mco_trace_module in mco.h:  \n    typedef enum \n    {\n        MCO_TRACE_BSE,\n        MCO_TRACE_LAYOUT,\n        MCO_TRACE_ORDER,\n        MCO_TRACE_DISK,\n        MCO_TRACE_TMGR,\n        MCO_TRACE_CONN,\n        MCO_TRACE_DBINST,\n        ...",t:"Tracing Framework Modules and Severity"},"630":{i:0.000421126077741417,u:"../Content/Programming/C/Structures/EWS_Web_Server.htm",a:"Request API Structure The MCO EWS HTTP request structure used by  mcorest_api_status_*()  functions is defined in file include/ws/mcohttp.h as follows:      \n    typedef struct tag_mcohs_request \n    {\n        #ifdef MCO_TRANSPORT_LAYER_EXT_INFO\n            mcohs_tlc_addr_h\tintf_addr;         /* ...",t:"EWS Web Server API Structures"},"631":{i:0.0024593535711106,u:"../Content/Programming/C/Structures/Web_Services.htm",a:"The eXtremeDB Web Services API uses streams to read and write data from JSON requests. The constants, prototypes, structures and enums used to manage streams are defined in the following sections. Stream Structures The stream prototypes and structures used by  mcorest_api_json_read_*()  and  ...",t:"Web Services API Structures"},"632":{i:0.000306211440647194,u:"../Content/Programming/C/Static.htm",a:"Static Application-Independent API The “static” application-independent functions  are used for database control (opening, connecting to and closing databases), transaction management and cursor navigation. Please use the links below to view the functions by category:",t:"Static Application-Independent API"},"633":{i:0.000138984720996937,u:"../Content/Programming/C/Core_Functions/Static/Runtime_Control.htm",a:"The eXtremeDB database runtime is the set of C or C++ language libraries linked (statically or dynamically) into the application. The runtime environment must be initialized before performing any database actions by  calling the function mco_runtime_start().  This startup procedure initializes one ...",t:"Runtime Control"},"634":{i:0.00100886113514737,u:"../Content/Programming/C/Core_Functions/Static/Runtime_Control/runtime_start.htm",a:"Start the database runtime. Prototype  \n    MCO_RET mco_runtime_start(void)\n  Arguments Description This function initializes a semaphore to coordinate access to the database dictionary. It must be called once, and only once, from each process that uses the eXtremeDB runtime, before the database is ...",t:"mco_runtime_start"},"635":{i:0.000141322220045191,u:"../Content/Programming/C/Core_Functions/Static/Runtime_Control/runtime_start_ex.htm",a:"Dynamically load the specified libraries and start the database runtime. Prototype  \n    MCO_RET mco_runtime_start_ex(mco_bool disk, \n                    mco_bool mvcc, \n                    mco_bool shmem,\n                    const char *sync_library, \n                    const char *memory_library, ...",t:"mco_runtime_start_ex"},"636":{i:0.000296533010050923,u:"../Content/Programming/C/Core_Functions/Static/Runtime_Control/runtime_stop.htm",a:"Stop the database runtime. Prototype  \n    MCO_RET mco_runtime_stop(void)\n  Arguments Description This function terminates the eXtremeDB runtime and releases all resources initialized by  mco_runtime_start() . In multi-threaded applications with multiple database connections, or multi-process ...",t:"mco_runtime_stop"},"637":{i:0.000274864959690362,u:"../Content/Programming/C/Core_Functions/Static/Runtime_Control/get_runtime_info.htm",a:"Reports the version and enabled features of the database runtime. Prototype  \n    MCO_RET mco_get_runtime_info(\t/*OUT*/ mco_runtime_info_t * pinfo)\n  Arguments Description This function reports the version and enabled features of the eXtremeDB database runtime. Return Codes Example  \n    Application ...",t:"mco_get_runtime_info"},"638":{i:0.00304344890285136,u:"../Content/Programming/C/Core_Functions/Static/Runtime_Control/runtime_setoption.htm",a:"Set a runtime option. Prototype  \n    void mco_runtime_setoption(\tint option, int value)\n  Arguments Description This function sets the runtime option to the specified value. The options and allowable values are described in page  Runtime Options . Return Codes Example  \n    Application snippet:\n    ...",t:"mco_runtime_setoption"},"639":{i:0.000153609304753072,u:"../Content/Programming/C/Core_Functions/Static/Runtime_Control/runtime_getoption.htm",a:"Get a runtime option value. Prototype  \n    int mco_runtime_getoption(\tint option )\n  Arguments Description Returns the value of the specified runtime option. Return Codes Example  \n    Application snippet:\n        \n     \n    int main(int argc, char* argv[])\n    {\n        int val = 0;\n         \n     ...",t:"mco_runtime_getoption"},"640":{i:0.000138984720996937,u:"../Content/Programming/C/Core_Functions/Static/Database_Control.htm",a:"Database Control Functions These functions are used to perform database operations such as: open and close, connect and disconnect, extend working memory, manage backup and recovery. ",t:"Database Control Functions"},"641":{i:0.000442964232916488,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/open.htm",a:"This function is deprecated in eXtremeDB version 4.0 and later.  It is retained for backward compatibility with in-memory only database applications written for previous versions of the eXtremeDB runtime. For applications using eXtremeDB version 4.0 and later the recommended database open API is  ...",t:"mco_db_open"},"642":{i:0.00874603891674777,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/open_dev.htm",a:"Create a database on the specified device(s). Prototype  \n    MCO_RET\tmco_db_open_dev(\t/*IN*/ const char * dbname,\n                  /*IN*/ mco_dictionary_h dict,\n                  /*IN*/ mco_device_t * devs,\n                  /*IN*/ mco_size_t n_devs,\n                  /*IN*/ mco_db_params_t * ...",t:"mco_db_open_dev"},"643":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/open_disk.htm",a:"Create or open a persistent database. This function is provided for compatibility with eXtremeDB version 3.5. It replaces the combination of  mco_db_open() and mco_disk_open() - deprecated function calls. For applications using eXtremeDB version 4.0 and later use  mco_db_open_dev() Prototype  \n    ...",t:"mco_db_open_disk"},"644":{i:0.000665379188111794,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/close.htm",a:"Close a database. Prototype  \n    MCO_RET\tmco_db_close(\t/*IN*/ const char * dbname);\n  Arguments Description This function destroys the database dbname. The data is discarded. All handles to the database become invalid. The memory is not freed, it is the responsibility of the calling application to ...",t:"mco_db_close"},"645":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/close_all.htm",a:"Close all database instances. Prototype  \n    MCO_RET\tmco_close_all_instances(\tvoid );\n  Arguments Description This function destroys all database instances. It is used to cleanup possible previous database instances after an application crash. Return Codes Example  \n    Application snippet:\n        ...",t:"mco_close_all_instances"},"646":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/kill.htm",a:"Remove a shared memory segment associated with a database. Prototype  \n    MCO_RET\tmco_db_kill(\t/*IN*/ const char * dbname);\n  Arguments Description This function removes the shared memory segment allocated by the runtime for the database identified by the dbname. It also removes dbname from the ...",t:"mco_db_kill"},"647":{i:0.0031441196498836,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/connect.htm",a:"Connect to a database. Prototype  \n    MCO_RET\tmco_db_connect(\t/*IN*/ const char * dbname, /*OUT*/ mco_db_h *db);\n  Arguments Description This function connects an application (thread) to a database. The database must have been previously opened by mco_db_open(). It returns a connection handle that ...",t:"mco_db_connect"},"648":{i:0.00279644886329762,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/connect_ctx.htm",a:"Connect to a database specifying a context. Prototype  \n    MCO_RET\tmco_db_connect_ctx(\t/*IN*/ const char * dbname, \n                    /*IN*/ void const* context, \n                    /*OUT*/ mco_db_h *db);\n  Arguments Description This function connects to a database specifying connection context ...",t:"mco_db_connect_ctx"},"649":{i:0.00265879998358114,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/connection_context.htm",a:"Get the context associated with a connection. Prototype  \n    void * mco_db_connection_context(\t/*IN*/ mco_db_h db);\n  Arguments Description This function returns the database context data associated with this connection. This function call is normally used as the second argument in a call to  ...",t:"mco_db_connection_context"},"650":{i:0.000409469236572165,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/disconnect.htm",a:"Disconnect from a database. Prototype  \n    MCO_RET\tmco_db_disconnect(\t/*IN*/ mco_db_h db);\n  Arguments Description This function destroys the connection handle to the database and invalidates any existing transaction handles for the connection. The function verifies that there are no active ...",t:"mco_db_disconnect"},"651":{i:0.000133922545570967,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/register_udf.htm",a:"Register a user-defined function. Prototype  \n    MCO_RET\tmco_db_register_udf(\t/*IN*/ const char * dbname,\n                    /*IN*/ mco_userdef_funcs_h udfs );\n  Arguments Description This function registers the array of user-defined functions with the runtime. Return Codes Example  \n    ...",t:"mco_db_register_udf"},"652":{i:0.000780486073494299,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/params_init.htm",a:"Initialize database parameters. Prototype  \n    void\tmco_db_params_init(\tmco_db_params_t * params );\n  Arguments Description This function sets default parameters (page sizes, number of connections, log policies, etc.,) for the call to the function  mco_db_open_dev(). Return Codes Example  \n    ...",t:"mco_db_params_init"},"653":{i:0.000424007851201976,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/extend.htm",a:"Extend database memory. Prototype  \n    MCO_RET\tmco_db_extend(\t/*IN*/ const char * dbname\n                 /*IN*/ void * mem,\n                  /*IN*/ uint4 size);\n  Arguments Description This function extends the amount of memory used by the database dbname. Return Codes Example  \n    Application ...",t:"mco_db_extend"},"654":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/extend_ctx.htm",a:"Extend database memory. Prototype  \n    MCO_RET\tmco_db_extend_ctx(\t/*IN*/ mco_trans_h  t,\n                  /*IN*/ void * mem,\n                  /*IN*/ uint4 size,\n                  /*IN*/ void const* ctx);\n  Arguments Description This function extends the amount of memory used by the database. Note ...",t:"mco_db_extend_ctx"},"655":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/extend_t.htm",a:"Extend database memory within a transaction. Prototype  \n    MCO_RET\tmco_db_extend_t(\t/*IN*/ mco_trans_h  t,\n                  /*IN*/ void * mem,\n                  /*IN*/ uint4 size);\n  Arguments Description This function extends the amount of memory used by the database. Note that whereas  ...",t:"mco_db_extend_t"},"656":{i:0.00100270110683938,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/extend_dev.htm",a:"Extend database memory by adding a memory device. Prototype  \n    MCO_RET\tmco_db_extend_dev(\t/*IN*/ const char* dbname,\n                    /*IN*/ mco_device_t * dev );\n  Arguments Description This function extends the available database memory by adding the specified memory device. Return Codes ...",t:"mco_db_extend_dev"},"657":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/extend_dev_ctx.htm",a:"Extend database memory by adding a memory device. Prototype  \n    MCO_RET\tmco_db_extend_dev_ctx(\t/*IN*/ mco_trans_h  t,\n                    /*IN*/ mco_device_t * dev,\n                    /*IN*/ void const* ctx);\n  Arguments Description This function extends the available database memory by adding ...",t:"mco_db_extend_dev_ctx"},"658":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/extend_dev_t.htm",a:"Extend database memory by adding a memory device within a transaction. Prototype  \n    MCO_RET\tmco_db_extend_dev_t(\t/*IN*/ mco_trans_h  t,\n                    /*IN*/ mco_device_t * dev );\n  Arguments Description This function extends the available database memory by adding the specified memory ...",t:"mco_db_extend_dev_t"},"659":{i:0.0034454329678398,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/save.htm",a:"Save a database image to an external file. Prototype  \n    MCO_RET\tmco_db_save(\t/*IN*/ void * stream_handle,\n                /*IN*/ mco_stream_write output_stream_writer,\n                /*IN*/ mco_db_h db);\n  Arguments Description This function is used to stream an eXtremeDB database to permanent ...",t:"mco_db_save"},"660":{i:0.000396204369354549,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/class_save.htm",a:"Save an entire class to an external file. Prototype  \n    MCO_RET\tmco_class_save( /*IN*/ uint2 class_code,\n                  /*IN*/ void * stream_handle,\n                  /*IN*/ mco_stream_write output_stream_writer,\n                  /*IN*/ mco_db_h db);\n  Arguments Description This function is ...",t:"mco_class_save"},"661":{i:0.000159621148698175,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/save_inmem.htm",a:"Save transient (in-memory) classes to an external file. This function has been deprecated in version 7.1. See  mco_db_save(). Prototype  \n    MCO_RET\tmco_inmem_save(\t/*IN*/ void * stream_handle,\n                 /*IN*/ mco_stream_write output_stream_writer,\n                 /*IN*/ mco_db_h db);\n  ...",t:"mco_inmem_save DEPRECATED"},"662":{i:0.000159621148698175,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/save_disk.htm",a:"Save persistent classes to an external file.  This function has been deprecated in version 7.1. See mco_db_save() . Prototype  \n    MCO_RET\tmco_disk_save(\t/*IN*/ void * stream_handle,\n                 /*IN*/ mco_stream_write output_stream_writer,\n                 /*IN*/ mco_db_h db);\n  Arguments ...",t:"mco_disk_save DEPRECATED"},"663":{i:0.000220493088787577,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/json_export.htm",a:"This function exports the entire database contents, or individual objects, in JSON format. Prototype  \n    MCO_RET\tmco_db_json_export(\t mco_trans_h t, \n                    void *stream_handle, \n                    mco_stream_write output_stream_writer );\n  Arguments Description This function exports ...",t:"mco_db_json_export"},"664":{i:0.000162004662371558,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/save_metadata.htm",a:"Save the database internal data layout definition to an external file. Prototype  \n    MCO_RET\tmco_db_save_metadata(\t/*IN*/ void * stream_handle,\n                     /*IN*/ mco_stream_write output_stream_writer,\n                     /*IN*/ mco_db_h db, \n                     /*IN*/ mco_bool ...",t:"mco_db_save_metadata"},"665":{i:0.000142000490256332,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/save_dictionary.htm",a:"Save the database dictionary to an external file. Prototype  \n    MCO_RET\tmco_db_save_dictionary(\t/*IN*/ void * stream_handle,\n                      /*IN*/ mco_stream_write output_stream_writer,\n                      /*IN*/ mco_db_h db);\n  Arguments Description This function is used to stream the ...",t:"mco_db_save_dictionary"},"666":{i:0.0040170810613262,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/load.htm",a:"Load a database image from an external file. Prototype  \n    MCO_RET\tmco_db_load(\t/*IN*/ void *stream_handle,\n                /*IN*/ mco_stream_read input_stream_reader,\n                /*IN*/ const char * dbname,\n                /*IN*/ mco_dictionary_h dict,\n                /*IN*/ mco_device_t ...",t:"mco_db_load"},"667":{i:0.000704139533231165,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/class_load.htm",a:"Load an entire class from an external file. Prototype  \n    MCO_RET\tmco_class_load(\t/*IN*/ uint2 class_code,\n                  /*IN*/ void *stream_handle,\n                  /*IN*/ mco_stream_read input_stream_reader,\n                  /*IN*/ mco_db_h db );\n  Arguments Description This function loads ...",t:"mco_class_load"},"668":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/load_inmem.htm",a:"Load transient (in-memory) classes from an external file. This function has been deprecated in version 7.1. See mco_db_load() . Prototype  \n    MCO_RET\tmco_inmem_load(\t/*IN*/ void *stream_handle,\n                  /*IN*/ mco_stream_read input_stream_reader,\n                  /*IN*/ const char * ...",t:"mco_inmem_load DEPRECATED"},"669":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/load_disk.htm",a:"Load persistent classes from an external file.  This function has been deprecated in version 7.1. See mco_db_load() . Prototype  \n    MCO_RET\tmco_disk_load(\t/*IN*/ void *stream_handle,\n                 /*IN*/ mco_stream_read input_stream_reader,\n                 /*IN*/ const char * dbname,\n          ...",t:"mco_disk_load DEPRECATED"},"670":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/load_disk_file.htm",a:"DEPRECATED Load database files from persistent storage. This function has been deprecated in version 7.1. See  mco_db_load() . Prototype  \n    MCO_RET\tmco_disk_load_file(\t/*IN*/ char const * file_path,\n                    /*IN*/ void *stream_handle,\n                    /*IN*/ mco_stream_read ...",t:"mco_disk_load_file"},"671":{i:0.000466832634463209,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/sniffer.htm",a:"Inspect the state of all connections to an existing database. Prototype  \n    MCO_RET\tmco_db_sniffer(\t/*IN*/ mco_db_h db,\n                  /*IN*/ mco_sniffer_callback_t callback,\n                  /*IN*/ mco_sniffer_policy policy );\n  Arguments Description This function inspects the state of all ...",t:"mco_db_sniffer"},"672":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/recover_required.htm",a:"This function will determine from the log file if any databases connections were not properly closed. Prototype  \n    mco_bool\tmco_db_recover_required(\tconst char * log_file_path );\n  Arguments Description This function will determine from the log file if any databases connections were not properly ...",t:"mco_db_recover_required"},"673":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/recover.htm",a:"This function allows an application to explicitly recover a database. Prototype  \n    MCO_RET\tmco_db_recover(\t mco_db_h db, mco_db_h recover_db );\n  Arguments Description Normally a database that was not properly closed (perhaps after a system crash) will be recovered automatically during the ...",t:"mco_db_recover"},"674":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/recover_all.htm",a:"This function allows an application to explicitly recover any database connections that were not properly closed. Prototype  \n    MCO_RET\tmco_db_recover_all(\t mco_db_h db );\n  Arguments Description Normally a database that was not properly closed (perhaps after a system crash) will be recovered ...",t:"mco_db_recover_all"},"675":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/databases.htm",a:"List  all registered database names. Prototype  \n    MCO_RET\tmco_db_databases(\t/*OUT*/ char* lpBuffer,\n                   /*IN*/ mco_size32_t buffer_size,\n                   /*IN*/ mco_counter32_t skip_first);\n  Arguments Description This function returns a list of all database names registered in ...",t:"mco_db_databases"},"676":{i:0.000144664333384198,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/clean.htm",a:"Remove all objects from a database. Prototype  \n    MCO_RET\tmco_db_clean(\t/*IN*/ mco_db_h db);\n  Arguments Description This function removes all objects from an opened database. Return Codes Example  \n    Application snippet:\n        \n     \n    const char * dbname = \"SimpleDb\";\n     \n    int ...",t:"mco_db_clean"},"677":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/current_registry_size.htm",a:"Return the number of bytes required to load the database registry. Prototype  \n    unsigned int mco_current_registry_size( void );\n  Arguments Description This function returns the number of bytes required to load the database registry at runtime. Return Codes Example  \n    Application snippet:\n     ...",t:"mco_current_registry_size"},"678":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/get_last_autoid.htm",a:"Return the last generated value for an object of any database class that has an autoid field defined. Prototype  \n    MCO_RET mco_get_last_autoid( /*IN*/ mco_db_h db, \n                    /*OUT*/ uint8 * id );\n  Arguments Description Autoids are unique generated values within a database.  This ...",t:"mco_get_last_autoid"},"679":{i:0.00036008186806246,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/get_class_code.htm",a:"Return the class code of the object referenced by an object handle. Prototype  \n    MCO_RET mco_get_class_code( /*IN*/ void * object_handle, \n                    /*OUT*/ uint2 * code );\n  Arguments Description This function returns the class code of the object referenced by the object handle. It can ...",t:"mco_get_class_code"},"680":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/copy_handle.htm",a:"Copy an object handle. Prototype  \n    MCO_RET mco_copy_handle( /*IN*/ mco_db_h db, \n                  /*OUT*/ MCO_Hf * dst,\n                  /*IN*/ MCO_Hf * src );\n  Arguments Description Because object handles cannot be copied as a raw sequence of bytes (using memcpy() for example), this function ...",t:"mco_copy_handle"},"681":{i:0.000161058202155762,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/is_object_deleted.htm",a:"Determine if an object has been deleted within the current transaction. Prototype  \n    MCO_RET mco_is_object_deleted( /*IN*/ MCO_Hf * object_handle );\n  Arguments Description This function allows an application to determine if an object has been deleted within the current transaction. Return Codes ...",t:"mco_is_object_deleted"},"682":{i:0.000124380033450319,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/aio_start.htm",a:"Adjust Asynchronous IO parameters for persistent databases. Prototype  \n    MCO_RET\tmco_aio_start(\tmco_size_t max_queue_length, mco_size_t n_workers);\n  Arguments Description This function allows the application to adjust the AIO buffer size and number of threads used for AIO. It must be called ...",t:"mco_aio_start"},"683":{i:0.000177241706322102,u:"../Content/Programming/C/Core_Functions/Static/Database_Control/aio_stop.htm",a:"Stop Asynchronous IO. Prototype  \n    MCO_RET\tmco_aio_stop(\tvoid );\n  Arguments Description This function terminates AIO. It should be called when persistent database writes are finished if  AIO is enabled . Return Codes Example  \n    Application snippet:\n        \n     \n    const char * dbname = ...",t:"mco_aio_stop"},"684":{i:0.000138984720996937,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager.htm",a:"Disk Manager The following functions are used to manage the eXtremeDB persistent storage interface. ",t:"Disk Manager"},"685":{i:0.000807618658291293,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager/backup.htm",a:"Perform a file backup for a persistent database. For an overview see page  File_Backup Prototype  \n    MCO_RET\tmco_disk_backup(\t/*IN*/ mco_db_h db, \n                    /*IN*/ char const* db_backup_path, \n                    /*IN*/ char const* log_backup_path);\n  Arguments Description This function ...",t:"mco_disk_backup"},"686":{i:0.00024192288493635,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager/set_log_params.htm",a:"Set log parameters for the Disk Manager. Prototype  \n    MCO_RET\tmco_disk_set_log_params(\t/*IN*/ mco_db_h db, /*IN*/ mco_log_params_t* params);\n  Arguments Description This function sets logging parameters for the Disk Manager. See Setting Log Type and Parameters Return Codes Example  \n    ...",t:"mco_disk_set_log_params"},"687":{i:0.000132372416315713,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager/transaction_policy.htm",a:"Set the default  transaction commit policy for this connection. Prototype  \n    MCO_COMMIT_POLICY mco_disk_transaction_policy(\t/*IN*/ mco_db_h db, /*IN*/ MCO_COMMIT_POLICY policy);\n  Arguments Description This function sets the default transaction commit policy for this connection and returns the ...",t:"mco_disk_transaction_policy"},"688":{i:0.000278628987148916,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager/info.htm",a:"Return information about the current state of the Disk Manager. Prototype  \n    MCO_RET mco_disk_info(\t/*IN*/ mco_db_h db, /*OUT*/ mco_disk_info_t* info );\n  Arguments Description This function returns the information about the current state of the Disk Manager in the  mco_disk_info_t structure ...",t:"mco_disk_info"},"689":{i:0.000336224665924119,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager/flush.htm",a:"Explicitly flush the file system buffers. Prototype  \n    MCO_RET\tmco_disk_flush(\t/*IN*/ mco_db_h db );\n  Arguments Description This function allows applications to explicitly flush the file system buffers; it is most often used in conjunction with the MCO_COMMIT_NOSYNC Transaction Commit policy . ...",t:"mco_disk_flush"},"690":{i:0.000258811331862868,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager/fs_cache_warmup.htm",a:"Load database files into the file system cache. Prototype  \n    MCO_RET mco_disk_fs_cache_warmup( mco_db_h db ); \n  Arguments Description This function allows an application  to load database files into the file system cache which can give up to 8x performance improvement for traversal of database ...",t:"mco_disk_fs_cache_warmup"},"691":{i:0.000236061839924248,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager/get_cache_info.htm",a:"Get runtime disk manager  cache  statistics. Prototype  \n    mco_disk_cache_info_t*\tmco_disk_get_cache_info(\t/*IN*/ mco_db_h db );\n  Arguments Description This function allows applications to obtain runtime disk manager  cache  statistics in structure  mco_disk_cache_info_t , including cache hits ...",t:"mco_disk_get_cache_info"},"692":{i:0.000336389052402516,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager/enable_connection_cache.htm",a:"Enable or disable the  connection cache . Prototype  \n    mco_bool\tmco_disk_enable_connection_cache(\t/*IN*/ mco_db_h db, /*IN*/ mco_bool enable);\n  Arguments Description The runtime “pins” a predefined number of pages from the page pool for each connection. This is referred to as a “connection ...",t:"mco_disk_enable_connection_cache"},"693":{i:0.000236061839924248,u:"../Content/Programming/C/Core_Functions/Static/Disk_Manager/reset_connection_cache.htm",a:"Commit the  connection cache  (reset) to the database. Prototype  \n    MCO_RET\tmco_disk_reset_connection_cache(\t/*IN*/ mco_db_h db );\n  Arguments Description This function commits the connection cache (resets) to the database. The two functions  mco_disk_reset_connection_cache() and  ...",t:"mco_disk_reset_connection_cache"},});