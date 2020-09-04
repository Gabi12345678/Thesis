define({"763":{i:0.000233287087103224,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/deinit.htm",a:"Release the calculator resources. Prototype  \n    void\tmco_calc_deinit(\t/*OUT*/ mco_calc_t * calc );\n  Arguments Description This function releases the calculator resources. It must be called before disconnecting from the database. Return Codes Example  \n    Application snippet:\n        \n     \n    ...",t:"mco_calc_deinit"},"764":{i:0.00115342489304418,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/reg_schema_classes.htm",a:"Register the classes enumerated in the  database schema. Prototype  \n    MCO_RET mco_calc_reg_schema_classes(\t/*IN*/ mco_calc_t * calc,\n                         /*IN*/ mco_calculator_h ch );\n  Arguments Description This function registers the classes enumerated in the database schema (and ...",t:"mco_calc_reg_schema_classes"},"765":{i:0.00115342489304418,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/register_classes.htm",a:"Registers the specified classes. Prototype  \n    void mco_calc_register_classes(\t/*IN*/ mco_calc_t * calc,\n                      /*IN*/ mco_cc_t classes );\n  Arguments Description This function registers the classes passed in the classes  parameter for generating runtime statistics with the  ...",t:"mco_calc_register_classes"},"766":{i:0.00116169793087751,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/class_reg.htm",a:"Registers the specified classes. Prototype  \n    void mco_calc_class_reg(\t/*IN*/ mco_calc_t * calc,\n                  /*IN*/ const char *cname,\n                  /*IN*/ uint2 class_code,\n                  /*IN*/ int n );\n  Arguments Description This function registers a class with the parameter ...",t:"mco_calc_class_reg"},"767":{i:0.00115342489304418,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/class_add.htm",a:"Registers the specified classes. Prototype  \n    void mco_calc_class_add(\t/*IN*/ mco_calc_t * calc,\n                  /*IN*/ mco_cc_t class );\n  Arguments Description This function registers the class passed in the class  parameter for generating runtime statistics with the calculator. (See  ...",t:"mco_calc_class_add"},"768":{i:0.00115342489304418,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/fill_db.htm",a:"Fill the database with objects of the registered classes. Prototype  \n    MCO_RET mco_calc_fill_db(\t/*IN*/ mco_calc_t * calc );\n  Arguments Description This function fills the database with objects of classes registered in the calculator passed in calc.  Return Codes Example  \n    Application ...",t:"mco_calc_fill_db"},"769":{i:0.00115342489304418,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/stat_collect.htm",a:"Collect memory statistics for the registered classes. Prototype  \n    MCO_RET mco_calc_stat_collect(\t/*IN*/ mco_calc_t * calc );\n  Arguments Description This function collects memory statistics for the classes registered in the database  calculator  after the database has been populated by calling ...",t:"mco_calc_stat_collect"},"770":{i:0.00110853712070437,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/get_calss_info.htm",a:"Collects memory statistics for the specified class. Prototype  \n    MCO_RET mco_calc_get_class_info(\t/*IN*/ mco_calc_t * calc,\n                       /*IN*/ mco_cc_t class,\n                       /*OUT*/ mco_cc_info_t * info);\n  Arguments Description This function collects memory information for a ...",t:"mco_calc_get_class_info"},"771":{i:0.00116169793087751,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/cinfo_browse.htm",a:"Browse through the calculator classes. Prototype  \n    MCO_RET mco_calc_cinfo_browse(\t/*IN*/ mco_calc_t * calc,\n                      /*IN*/ mco_calc_cinfo_h handler,\n                      /*IN*/ void * data);\n  Arguments Description This function browses through calculator classes, one class at ...",t:"mco_calc_cinfo_browse"},"772":{i:0.00116169793087751,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Calculator/iinfo_browse.htm",a:"Browse through the calculator indexes. Prototype  \n    MCO_RET mco_calc_iinfo_browse(\t/*IN*/ mco_calc_t * calc,\n                      /*IN*/ mco_calc_iinfo_h handler,\n                      /*IN*/ void * data);\n  Arguments Description This function browses through calculator indexes, one index at ...",t:"mco_calc_iinfo_browse"},"773":{i:0.000296111176713418,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Tracing_Framework/init_simple_handler.htm",a:"Enable tracing with a simple file-based locking handler. For an overview see page  Tracing Framework in C Prototype  \n    void mco_trace_init_simple_handler(\t/*IN*/ const_char * filename, \n                       /*IN*/ mco_trace_file_appender_options close_policy,\n                       /*IN*/ const ...",t:"mco_trace_init_simple_handler"},"774":{i:0.000479728075671481,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Tracing_Framework/_message_handler_fn.htm",a:"Callback function to handle trace messages. For an overview see page  Tracing Framework in C When any database operation triggers a trace message a user-define callback function is called which has the following prototype:  \n    typedef int (*mco_trace_message_handler_fn)( mco_severity severity, \n   ...",t:"mco_trace_message_handler_fn Callback"},"775":{i:0.000432035493687203,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Tracing_Framework/set_handler.htm",a:"Set a user-defined trace handler. For an overview see page  Tracing Framework in C Prototype  \n    void mco_trace_set_handler(mco_trace_message_handler_fn trace_fn);\n  Arguments Description This function registers a user-defined custom  handler  to be called when database operations trigger ...",t:"mco_trace_set_handler"},"776":{i:0.000296111176713418,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Tracing_Framework/set_modules_mask.htm",a:"Set  the eXtremeDB modules to be logged. For an overview see page  Tracing Framework in C Prototype  \n    uint8 mco_trace_set_modules_mask(uint8 new_mask);\n  Arguments Description This function determines the eXtremeDB modules to be logged. Please note argument new_mask is a bit mask, and its values ...",t:"mco_trace_set_modules_mask"},"777":{i:0.000296111176713418,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Tracing_Framework/set_level.htm",a:"mco_trace_set_level Set the trace severity level. For an overview see page  Tracing Framework in C Prototype  \n    mco_severity mco_trace_set_level(mco_severity new_level);\n  Arguments Description This function sets the severity level for tracing messages. Return Codes",t:"mco_trace_set_level"},"778":{i:0.00443344899106195,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/System_Wrappers.htm",a:"System heap and other necessary APIs are not always available in the given operating system. Consequently an application can use third-part memory allocation mechanisms. The eXtremeDB System Abstraction Layer (SAL) provides a set of platform independent functions to allow customization in these ...",t:"System Wrapper Functions"},"779":{i:0.000844589571794226,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/System_Wrappers/sys_dll_load.htm",a:"Load an external module (dll, so or etc.) library and return the entry point of the specified function. Prototype  \n    void* mco_sys_dll_load(char const* library, char const* func);\n  Arguments Description This function loads an external module (dll,so or etc.) library, finds in it the function ...",t:"mco_sys_dll_load"},"780":{i:0.000592692299209462,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/System_Wrappers/sys_dll_error.htm",a:"Return a system-depenedent explanation of an error during the call of  mco_sys_dll_load(). Prototype  \n    char const * mco_sys_dll_error(void);\n  Arguments Description This function returns a system-depenedent explanation of an error if one happened during the call of  mco_sys_dll_load(). It ...",t:"mco_sys_dll_error"},"781":{i:0.00274623034628286,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/System_Wrappers/sys_malloc.htm",a:"The wrapper for C function malloc() or its equivalent in the given system - allocate heap memory. For an overview see page  eXtremeDB System Wrappers Prototype  \n    void * mco_sys_malloc(\t/*IN*/ mco_size_t n_bytes );\n  Arguments Description This function allocates a properly aligned block of ...",t:"mco_sys_malloc"},"782":{i:0.000592692299209462,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/System_Wrappers/sys_realloc.htm",a:"The wrapper for C function realloc()or its equivalent in the given system  - reallocate heap memory. For an overview see page  eXtremeDB System Wrappers Prototype  \n    void * mco_sys_realloc(\t/*IN*/ void * p,\n                /*IN*/ mco_size_t new_size );\n  Arguments Description This function ...",t:"mco_sys_realloc"},"783":{i:0.000592692299209462,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/System_Wrappers/sys_calloc.htm",a:"The wrapper for C function calloc() or its equivalent in the given system - allocate an array of memory blocks. For an overview see page  eXtremeDB System Wrappers Prototype  \n    void * mco_sys_calloc(\t/*IN*/ uint4 n,\n               /*IN*/ mco_size_t size );\n  Arguments Description This function ...",t:"mco_sys_calloc"},"784":{i:0.000844589571794226,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/System_Wrappers/sys_valloc.htm",a:"The wrapper for C function valloc() or its equivalent in the given system - allocate a system-aligned block of memory. For an overview see page  eXtremeDB System Wrappers Prototype  \n    void * mco_sys_valloc( /*IN*/ mco_size_t size );\n  Arguments Description This function allocates a of  size bytes ...",t:"mco_sys_valloc"},"785":{i:0.000704864775187063,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/System_Wrappers/sys_free.htm",a:"The wrapper for C function free()or its equivalent in the given system  - to free a memory block. For an overview see page  eXtremeDB System Wrappers Prototype  \n    void mco_sys_free(\t/*IN*/ void * p );\n  Arguments Description This function frees a memory block. It is a wrapper for the standard C ...",t:"mco_sys_free"},"786":{i:0.000592692299209462,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/System_Wrappers/sys_vfree.htm",a:"The wrapper for C function vfree() or its equivalent in the given system - to free a memory block allocated by  mco_sys_valloc(). For an overview see page  eXtremeDB System Wrappers Prototype  \n    void mco_sys_vfree(\t/*IN*/ void * p );\n  Arguments Description This function frees a memory block ...",t:"mco_sys_vfree"},"787":{i:0.000196104328743694,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Feed_Handler.htm",a:"The Feed Handler API return codes and handle types are enumerated in the Feed Handler API  header files .  For an overview see page  Fee Handler API Overview Please use the links in the following table to view the detailed descriptions of each API function: Feed Handler Module API Functions Please ...",t:"Feed Handler C API"},"788":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/app_stop.htm",a:"Request the Feed Handler application to shutdown. Prototype  \n    void\tmco_fh_app_stop(void);\n  Arguments Description This function requests the Feed Handler application to shutdown and should only be called after the module execution has been started with mco_fh_mod_start() and before ...",t:"mco_fh_app_stop"},"789":{i:0.000282291817310171,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/metadata_add.htm",a:"Provide metadata to the feed handler. Prototype  \n    mco_fh_ret mco_fh_metadata_add(/*IN*/ mco_fh_db_h db, \n                        /*IN*/ const char *rec_type, \n                        /*IN*/ const char *field_name,\n                        /*IN*/ mco_int4 field_type, \n                        ...",t:"mco_fh_metadata_add"},"790":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/writer_create.htm",a:"Create a database writer object. Prototype  \n    mco_fh_writer_h mco_fh_writer_create(/*IN*/ mco_fh_db_h db, \n                            /*IN*/ const char *table_name)\n  Arguments Description This function returns the handle of a database writer for the specified table. Return Codes Example  \n    ...",t:"mco_fh_writer_create"},"791":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/writer_store_record.htm",a:"Writes a record to the database. Prototype  \n    mco_fh_ret mco_fh_writer_store_record(/*IN*/ mco_fh_writer_h wr, \n                            /*IN*/ mco_fh_record_h rec)\n  Arguments Description This function writes the passed record into the table to which this writer is assigned. Return Codes ...",t:"mco_fh_writer_store_record"},"792":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/writer_destroy.htm",a:"Destroy the writer object. Prototype  \n    mco_fh_ret mco_fh_writer_destroy(/*IN*/ mco_fh_writer_h wr);\n  Arguments Description This function destroys a writer object. Return Codes Example  \n     \n    ...\n    std::vector\u003cTableInfo\u003e::iterator it;\n    for (it = tables.begin(); it != tables.end(); ...",t:"mco_fh_writer_destroy"},"793":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/record_create.htm",a:"Create a database record. Prototype  \n    mco_fh_record_h mco_fh_record_create(mco_fh_writer_h wr)\n  Arguments Description This function returns the handle of a record in the table to which this writer is assigned. Return Codes Example  \n     \n    std::vector\u003cTableInfo\u003e::iterator it;\n    for (it = ...",t:"mco_fh_record_create"},"794":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/record_zero_out.htm",a:"Reset the record’s fields to zeroes. Prototype  \n    mco_fh_ret mco_fh_record_zero_out(/*IN*/ mco_fh_record_h rec);\n  Arguments Description This function resets the record’s fields to zeroes. Return Codes Example  \n     \n    ...\n    // retrieve next quote from the feed\n    FeedQuote *quote = ...",t:"mco_fh_record_zero_out"},"795":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/record_set_field_SCALAR_TYPE.htm",a:"Set the value of a record’s scalar field. Prototype  \n    mco_fh_ret mco_fh_record_set_field_SCALAR_TYPE(/*IN*/ mco_fh_record_h rec, \n                            /*IN*/ mco_offs_t field, \n                            /*IN*/ SCALAR_TYPE val);\n  Arguments Description This function sets the value of a ...",t:"mco_fh_record_set_field_SCALAR_TYPE"},"796":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/record_set_field_string.htm",a:"Set a record field’s C string value. Prototype  \n    mco_fh_ret mco_fh_record_set_field_string(/*IN*/ mco_fh_record_h rec,\n                            /*IN*/ mco_offs_t field, \n                            /*IN*/ const char *str);\n  Arguments Description This function sets a record field’s C string ...",t:"mco_fh_record_set_field_string"},"797":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/record_set_field_char_array.htm",a:"Set a record field’s C string value. Prototype  \n    mco_fh_ret mco_fh_record_set_field_char_array(mco_fh_record_h rec, mco_offs_t field,\n                            /*IN*/ mco_offs_t field, \n                            /*IN*/ const char *array,\n                            /*IN*/ mco_size_t len);\n  ...",t:"mco_fh_record_set_field_char_array"},"798":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/record_destroy.htm",a:"Destroy the record object. Prototype  \n    mco_fh_ret mco_fh_record_destroy(/*IN*/ mco_fh_record_h rec);\n  Arguments Description This function destroys a record object. Return Codes Example  \n     \n    ...\n    std::vector\u003cTableInfo\u003e::iterator it;\n    for (it = tables.begin(); it != tables.end(); ...",t:"mco_fh_record_destroy"},"799":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/config_get_node_type.htm",a:"Return the type of the configuration node. Prototype  \n    MCO_FH_CONFIG_NODE_TYPE mco_fh_config_get_node_type(/*IN*/ mco_fh_config_node_h n)\n  Arguments Description This function returns the  type  of the configuration node. Return Codes Example      \n    {\n        ...\n        mco_fh_config_node_h ...",t:"mco_fh_config_get_node_type"},"800":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/config_get_node_name.htm",a:"Return the name of the configuration node. Prototype  \n    const char *mco_fh_config_get_node_name(/*IN*/ mco_fh_config_node_h n)\n  Arguments Description This function returns the name of the configuration node. Return Codes Example  \n    const char *node_name = mco_fh_config_get_node_name(node);\n   ...",t:"mco_fh_config_get_node_name"},"801":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/config_node_get_SCALAR_TYPE.htm",a:"Retrieve the configuration node’s value as SCALAR_TYPE. Prototype  \n    mco_SCALAR_TYPE mco_fh_config_node_get_SCALAR_TYPE(/*IN*/ mco_fh_config_node_h n, \n                                    /*IN*/ SCALAR_TYPE def)\n  Arguments Description This function returns the value of the configuration node. If ...",t:"mco_fh_config_node_get_SCALAR_TYPE"},"802":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/config_node_get_child.htm",a:"Return the current node’s child node with the given key. Prototype  \n    mco_fh_config_node_h mco_fh_config_node_get_child(/*IN*/ mco_fh_config_node_h n, \n                                    /*IN*/ const char *key)\n  Arguments Description This function returns the current node’s child node with the ...",t:"mco_fh_config_node_get_child"},"803":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/config_node_get_vector_size.htm",a:"Return the array node’s element count. Prototype  \n    mco_size_t mco_fh_config_node_get_vector_size(/*IN*/ mco_fh_config_node_h n);\n  Arguments Description This function returns the array node’s element count. Return Codes Example  \n    {\n        for (mco_size_t i = 0; i \u003c ...",t:"mco_fh_config_node_get_vector_size"},"804":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/config_node_get_vector_element.htm",a:"Return the array node’s element at the specified index. Prototype  \n    mco_fh_config_node_h mco_fh_config_node_get_vector_element(/*IN*/ mco_fh_config_node_h n, \n                                        /*IN*/ mco_offs_t index)\n  Arguments Description This function returns the array node’s element ...",t:"mco_fh_config_node_get_vector_element"},"805":{i:0.000189193217762779,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/log.htm",a:"Write a message to the Feed Handler application log. Prototype  \n    void\tmco_fh_log(/*IN*/ MCO_FH_LOG_LEVEL level, \n                /*INH*/ const char *file, \n                /*IN*/ int line, \n                /*IN*/ const char *msg);\n  Arguments Description This function writes a message to the ...",t:"mco_fh_log"},"806":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_api_version.htm",a:"Return the version of ModAPI implemented by this module. Prototype  \n    mco_uint4 mco_fh_mod_api_version()\n  Arguments Description This function returns the version of ModAPI implemented by this module. Modules should return the value of the MCO_FH_MODAPI_VERSION macro defined in the fhmodapi.h ...",t:"mco_fh_mod_api_version"},"807":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_create.htm",a:"Create a module instance. Prototype  \n    mco_fh_ret mco_fh_mod_create(/*IN*/ mco_fh_config_node_h cfg, \n                    /*OUT*/ mco_fh_module_h *h)\n  Arguments Description This function creates a module instance. Return Codes Example  \n    mco_fh_ret mco_fh_mod_create(mco_fh_config_node_h cfg, ...",t:"mco_fh_mod_create"},"808":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_name.htm",a:"Return the name of the module. Prototype  \n    const char *mco_fh_mod_name(/*IN*/ mco_fh_module_h h)\n  Arguments Description This function returns the name of the module. Return Codes Example  \n    const char *mco_fh_mod_name(mco_fh_module_h h)\n    {\n        return \"Test Feed\";\n    }\n ",t:"mco_fh_mod_name"},"809":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_version.htm",a:"Return the version of the module. Prototype  \n    const char *mco_fh_mod_version(/*IN*/ mco_fh_module_h h)\n  Arguments Description This function returns the version of the module. Return Codes Example  \n    const char *mco_fh_mod_version(mco_fh_module_h h)\n    {\n        return \"1.0\";\n    }\n ",t:"mco_fh_mod_version"},"810":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_add_data_source.htm",a:"Add a data source from the module’s configuration section. Prototype  \n    mco_fh_ret mco_fh_mod_add_data_source(/*IN*/ mco_fh_module_h h, \n                            /*IN*/ const char *type, \n                            /*IN*/ const char *name)\n  Arguments Description This function adds a data ...",t:"mco_fh_mod_add_data_source"},"811":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_add_subscription.htm",a:"Add a subscription to the module’s subscription list. Prototype  \n    mco_fh_ret mco_fh_mod_add_subscription(/*IN*/ mco_fh_module_h h, \n                            /*IN*/ const char *source_name,\n                            /*IN*/ const char *data_domain, \n                            /*IN*/ const ...",t:"mco_fh_mod_add_subscription"},"812":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_init.htm",a:"Initialize the module. Prototype  \n    mco_fh_ret mco_fh_mod_init(/*IN*/ mco_fh_module_h h, \n                    /*IN*/ mco_fh_db_h db)\n  Arguments Description This function initializes the module. At this stage the module can connect to the services, retrieve metadata, dictionaries, etc. Return ...",t:"mco_fh_mod_init"},"813":{i:0.000366484351042431,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_fill_metadata.htm",a:"Request that the module provide metadata. Prototype  \n    mco_fh_ret mco_fh_mod_fill_metadata(/*IN*/ mco_fh_module_h *h)\n  Arguments Description This function requests that the module provide metadata using the  mco_fh_metadata_add() API call. If the module is unable to provide metadata, it should ...",t:"mco_fh_mod_fill_metadata"},"814":{i:0.000282291817310171,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_get_field_info.htm",a:"Return information (type, size, precision) about the specified table’s field. Prototype  \n    mco_fh_ret mco_fh_mod_get_field_info(/*IN*/ mco_fh_module_h h, \n                            /*IN*/ const char *rec_type, \n                            /*IN*/ const char *field_name,\n                          ...",t:"mco_fh_mod_get_field_info"},"815":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_add_table.htm",a:"Instruct the module to use the database table as specified. Prototype  \n    mco_fh_ret mco_fh_mod_add_table(/*IN*/ mco_fh_module_h h, \n                            /*IN*/ const char *rec_type, \n                            /*IN*/ const char *name,\n                            /*IN*/ const char ...",t:"mco_fh_mod_add_table"},"816":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_start.htm",a:"Request that the module begin execution. Prototype  \n    mco_fh_ret mco_fh_mod_start(/*IN*/ mco_fh_module_h h);\n  Arguments Description This function requests that the module begin execution. The module should launch separate threads and return control immediately. Return Codes Example  \n    ...",t:"mco_fh_mod_start"},"817":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_get_stats.htm",a:"Retrieve the module’s statistics for UI reporting. Prototype  \n    mco_fh_ret mco_fh_mod_get_stats(/*IN*/ mco_fh_module_h h,\n                        /*OUT*/ mco_uint8 *connected_at, \n                        /*OUT*/ mco_uint8 *num_msgs);\n  Arguments Description This function retrieves the module’s ...",t:"mco_fh_mod_get_stats"},"818":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_stop.htm",a:"Request that the module to stop execution. Prototype  \n    void mco_fh_mod_stop(/*IN*/ mco_fh_module_h h);\n  Arguments Description This function requests that the module stop. The module should stop all its threads and wait for them to finish. Return Codes Example  \n    void ...",t:"mco_fh_mod_stop"},"819":{i:0.000126535281782335,u:"../Content/Programming/C/Core_Functions/Static/Feed_Handler/mod_destroy.htm",a:"Request deallocation of all resources associated with the module’s handle. Prototype  \n    void mco_fh_mod_destroy(/*IN*/ mco_fh_module_h h);\n  Arguments Description This function requests deallocation of all resources associated with the module’s handle. Return Codes Example  \n    void ...",t:"mco_fh_mod_destroy"},"820":{i:0.00280830351574176,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Web_Services_C.htm",a:"eXtremeDB Web Services C API The mcorest library exposes a REST-like interface. Please see page  eXtremeDB Web Service Resources  for a list of the resources that are currently supported, as well as samples of the JSON responses.  For an overview see page  eXtremeDB Web Services The core library ...",t:" Web Services C API"},"821":{i:0.000229343418148259,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Web_Services/initialize.htm",a:"mcorest_initialize Initialize the MCO REST  server runtime.  For an overview see page  Web Services Prototype  \n  MCO_RET mcorest_initialize();\n  Arguments Description This function Initializes the MCO REST server runtime.  Return Codes",t:"mcorest_initialize"},"822":{i:0.000229343418148259,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Web_Services/svc_db_init.htm",a:"Initialize the database web service library. For an overview see page  Web Services Prototype  \n  MCO_RET mcorest_svc_db_init(mcorest_h rest);\n  Arguments Description This function Initializes the database web service library. The database web service provides access to the database content, ...",t:"mcorest_svc_db_init"},"823":{i:0.000229343418148259,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Web_Services/svc_perf_init.htm",a:"Initialize the perfmon web service library. For an overview see page  Web Services Prototype  \n  MCO_RET mcorest_svc_perf_init(mcorest_h rest);\n  Arguments Description This function Initializes the perfmon web service library. The perf web service provides access to the performance metrics database. ...",t:"mcorest_svc_perf_init"},"824":{i:0.000229343418148259,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Web_Services/svc_sql_init.htm",a:"Initialize the SQL web service library. For an overview see page  Web Services Prototype  \n  MCOR_RET mcorest_svc_sql_init(mcorest_h rest, void *engine);\n  Arguments Description This function Initializes the SQL web service library. The SQL web service enables the clients to run arbitrary SQL ...",t:"mcorest_svc_sql_init"},"825":{i:0.000229343418148259,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Web_Services/create.htm",a:"Create the MCO REST server instance and stores its handle in the rest parameter. For an overview see page  Web Services Prototype  \n  MCO_RET mcorest_create(const char *db_name, mcorest_h *rest);\n  Arguments Description This function creates the MCO REST server instance and stores its handle in the ...",t:"mcorest_create"},"826":{i:0.000229343418148259,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Web_Services/add_interface.htm",a:"Add a network interface to the MCO REST server instance. For an overview see page  Web Services Prototype  \n  MCO_RET mcorest_add_interface(mcorest_h rest, \n                  const char *addr, \n                  unsigned short port, \n                  void *ssl_params,\n                  ...",t:"mcorest_add_interface"},"827":{i:0.00020688533513975,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Web_Services/set_basic_auth.htm",a:"  Set Basic HTTP authentication parameters for the server instance. For an overview see page  Web Services Prototype  \n  MCO_RET mcorest_set_basic_auth(mcorest_h rest, \n                        const char *realm,\n                        const char *username, \n                        const char ...",t:"mcorest_set_basic_auth"},"828":{i:0.00020688533513975,u:"../Content/Programming/C/Core_Functions/Static/Auxiliary/Web_Services/reset_basic_auth.htm",a:"  Reset Basic HTTP authentication parameters and disables the authentication. For an overview see page  Web Services Prototype  \n  MCO_RET mcorest_reset_basic_auth(mcorest_h rest);\n  Arguments Description This function   resets the Basic HTTP authentication parameters and disables the ...",t:"mcorest_reset_basic_auth"},});