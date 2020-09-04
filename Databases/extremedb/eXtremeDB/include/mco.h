/****************************************************************
 *                                                              *
 *  mco.h                                                       *
 *                                                              *
 *  This file is a part of the eXtremeDB source code.           *
 *                                                              *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.  *
 *                                                              *
 *  eXtremeDB runtime public definitions and static API         *
 *                                                              *
 ****************************************************************/
#ifndef MCO_MCO_H__
    #define MCO_MCO_H__

/**
    This is the complete list of *_USE_DLL definitions required to correctly
    linking with DLL version of proper eXtremeDB library.
    These definitions are required if static constant or variable from such
    eXtremeDB DLL should be used.

    MCOBACKUP_USE_DLL
    MCOCRYPT_USE_DLL
    MCOCLUSTER_USE_DLL
    MCOCOMPLIB_USE_DLL
    MCODBCALC_USE_DLL
    MCOEWS_USE_DLL
    MCOHA_USE_DLL
    MCOHV_USE_DLL
    MCOHVPERF_USE_DLL
    MCOIOTCOMM_USE_DLL
    MCOIOTREPL_USE_DLL
    MCOITER_USE_DLL
    MCOLOG_USE_DLL
    MCONWTCP_USE_DLL
    MCONWUDP_USE_DLL
    MCONWPIP_USE_DLL
    MCONET_USE_DLL
    MCOPERF_USE_DLL
    MCOPLATFORM_USE_DLL
    MCOREST_USE_DLL
    MCORESTDB_USE_DLL
    MCORESTPERF_USE_DLL
    MCORESTSQL_USE_DLL
    MCORESTTRACE_USE_DLL
    MCORSQL_USE_DLL
    MCOSALDLOAD_USE_DLL
    MCOSERI_USE_DLL
    MCOSEQ_USE_DLL
    MCOSEQMATH_USE_DLL
    MCOSQL_USE_DLL
    MCOUDA_USE_DLL
    MCOJSER_USE_DLL
*/

#ifdef _WIN32
#  ifdef MCOSERI_DLL
#    define MCOSERI_API __declspec(dllexport)
#  else
#    ifdef MCOSERI_USE_DLL
#       define MCOSERI_API __declspec(dllimport)
#    else
#       define MCOSERI_API 
#    endif
#  endif
#else
#  define MCOSERI_API
#endif

#ifdef _WIN32
#  ifdef MCOSALDLOAD_DLL
#    define MCOSALDLOAD_API __declspec(dllexport)
#  else
#    ifdef MCOSALDLOAD_USE_DLL
#       define MCOSALDLOAD_API __declspec(dllimport)
#    else
#       define MCOSALDLOAD_API
#    endif
#  endif
#else
#  define MCOSALDLOAD_API
#endif


#ifdef _WIN32
#  ifdef MCOBACKUP_DLL
#    define MCOBACKUP_API __declspec(dllexport)
#  else
#    ifdef MCOBACKUP_USE_DLL
#       define MCOBACKUP_API __declspec(dllimport)
#    else
#       define MCOBACKUP_API 
#    endif
#  endif
#else
#  define MCOBACKUP_API
#endif

#ifdef _WIN32
#  ifdef MCOCRYPT_DLL
#    define MCOCRYPT_API __declspec(dllexport)
#  else
#    ifdef MCOCRYPT_USE_DLL
#       define MCOCRYPT_API __declspec(dllimport)
#    else
#       define MCOCRYPT_API 
#    endif
#  endif
#else
#  define MCOCRYPT_API
#endif

#ifdef _WIN32
#  ifdef MCOJSER_DLL
#    define MCOJSER_API __declspec(dllexport)
#  else
#    ifdef MCOJSER_USE_DLL
#       define MCOJSER_API __declspec(dllimport)
#    else
#       define MCOJSER_API
#    endif
#  endif
#else
#  define MCOJSER_API
#endif

    #ifdef __cplusplus
        extern "C"
        {
        #endif

    #ifdef _INTEGRITY
    #include "mcosmint.h"
    #endif

        #include "mcospec.h"

        /**
            This list may be translated to text messages by function mco_strerror implemented in
            file target/mcostrerror/mcostrerror.c . Keep both lists in accordance.
        */
        typedef enum MCO_RET_E_ /* return codes */
        {
            /* status codes */
            MCO_S_OK                           = 0,   /* success  */
            MCO_S_BUSY                         = 1,   /* the instance is busy */
            MCO_S_OVERFLOW                     = 2,   /* tree overflow */
            MCO_S_UNDERFLOW                    = 3,   /* tree underflow */
            MCO_S_NOTFOUND                     = 10,  /* search operation failed */
            MCO_S_CURSOR_END                   = 11,  /* cursor cannot be moved */
            MCO_S_CURSOR_EMPTY                 = 12,  /* no objects in index */
            MCO_S_DUPLICATE                    = 13,  /* index restriction violated (duplicate) */
            MCO_S_EVENT_RELEASED               = 14,  /* waiting thread was released */
            MCO_S_DEAD_CONNECTION              = 15,  /* database connection is invalid */
            MCO_S_NULL_VALUE                   = 16,  /* field has NULL value */
            MCO_S_TL_INVDATA                   = 17,  /* broken record during recovering */
            MCO_S_TL_NOT_INITIALIZED           = 18,  /* TL is not initialized in current process */
            MCO_S_DEFERRED_DELETE              = 19,  /* memory segment is busy, will be deleted later */
            MCO_S_REST_CONN_ACCEPTED           = 20,  /* a connection was accepted */
            MCO_S_REST_CONN_FINISHED           = 21,  /* a request was handled successfully, and the connection was closed */
            MCO_S_REST_TIMEOUT                 = 22,  /* operation timed out */

            /* error codes and code bases */
            MCO_E_TRANS_NOT_ACTIVE             = 49,  /* transaction is not active */
            MCO_E_CORE                         = 50,
            MCO_E_INVALID_HANDLE               = 51,  /* normally invalid handle causes a fatal error. */
            MCO_E_NOMEM                        = 52,  /* no memory */
            MCO_E_ACCESS                       = 53,  /* attempt to use a read only transaction for a write operation */
            MCO_E_TRANSACT                     = 54,  /* transaction is in the error state */
            MCO_E_INDEXLIMIT                   = 55,  /* vector index out of bounds */
            MCO_E_EMPTYVECTOREL                = 56,  /* vector element was not set (at given index) */
            MCO_E_UNSUPPORTED                  = 57,  /* unsupported call  */
            MCO_E_EMPTYOPTIONAL                = 58,  /* an optional structure has not been set */
            MCO_E_EMPTYBLOB                    = 59,  /* attempt to read a null blob  */
            MCO_E_CURSOR_INVALID               = 60,  /* cursor is not valid */
            MCO_E_ILLEGAL_TYPE                 = 61,  /* search by oid: type is not expected */
            MCO_E_ILLEGAL_PARAM                = 62,  /* invalid parameter; e.g. illegal search operation type */
            MCO_E_CURSOR_MISMATCH              = 63,  /* cursor type and object type are incompatible */
            MCO_E_DELETED                      = 64,  /* attempt to update an object deleted in the current transaction */
            MCO_E_LONG_TRANSACTON              = 65,  /* transaction length is greater then MCO_TRN_MAXLENGTH */
            MCO_E_INSTANCE_DUPLICATE           = 66,  /* duplicate database instance */
            MCO_E_UPGRADE_FAILED               = 67,  /* transaction upgrade failed; a write transactions is in progress  */
            MCO_E_NOINSTANCE                   = 68,  /* database instance is not found */
            MCO_E_OPENED_SESSIONS              = 69,  /* failed to close database because it has open connections */
            MCO_E_PAGESIZE                     = 70,  /* page size is not acceptable */
            MCO_E_WRITE_STREAM                 = 71,  /* write stream failure */
            MCO_E_READ_STREAM                  = 72,  /* read stream failure  */
            MCO_E_LOAD_DICT                    = 73,  /* attempt to load an incompatible dictionary in db_load */
            MCO_E_LOAD_DATA                    = 74,  /* attempt to load a corrupted image in db_load */
            MCO_E_VERS_MISMATCH                = 75,  /* version mismatch */
            MCO_E_VOLUNTARY_NOT_EXIST          = 76,  /* voluntary index has not been created */
            MCO_E_EXCLUSIVE_MODE               = 77,  /* attempt to open a database that is already open in the exclusive mode */
            MCO_E_MAXEXTENDS                   = 78,  /* maximum number of extends reached */
            MCO_E_HIST_OBJECT                  = 79,  /* operation is illegal for the older version of the object */
            MCO_E_SHM_ERROR                    = 80,  /* failed attempt to create/attach to shared memory */
            MCO_E_NOTINIT                      = 81,  /* runtime was not initialized */
            MCO_E_SESLIMIT                     = 82,  /* sessions number limit reached */
            MCO_E_INSTANCES_LIMIT              = 83,  /* too many instances */
            MCO_E_MAXTRANSSIZE_LOCKED          = 84,  /* the maximum size of transaction cannot be changed */
            MCO_E_DEPRECATED                   = 85,  /* obsolete feature */
            MCO_E_NOUSERDEF_FUNCS              = 86,  /* DB has userdef indexes defined, but no custom functions registered */
            MCO_E_CONFLICT                     = 87,  /* MVCC conflict */
            MCO_E_INMEM_ONLY_RUNTIME           = 88,  /* there are persistent classes in the schema but runtime in-memory only */
            MCO_E_ISOLATION_LEVEL_NOT_SUPPORTED= 89,  /* requested isolation level is not supported */
            MCO_E_REGISTRY_UNABLE_CREATE_CONNECT=90,  /* unable to create a new registry */
            MCO_E_REGISTRY_UNABLE_CONNECT      = 91,  /* unable to connect to an existing registry */
            MCO_E_REGISTRY_INVALID_SYNC        = 92,  /* invalid sync in the registry detected */
            MCO_E_MDEV_RUNTIME_START           = 93,  /* unable to initialize memory device for the registry */
            MCO_E_SYNC_RUNTIME_START           = 94,  /* unable to start sync runtime */
            MCO_E_ALIGN_ERROR                  = 95,  /* bit range get/set align error */
            MCO_E_PINNED_VERSION_LIMIT         = 96,  /* pinned version number limit is reached */
            MCO_E_VERSION_NOT_PINNED           = 97,  /* version is not pinned */
            MCO_E_CURSOR_CLOSED                = 98,  /* cursor is closed */
            MCO_E_CONVERSION                   = 99,  /* Conversion error in binary schema evolution */

            MCO_E_DISK                         = 100, /* general disk error */
            MCO_E_DISK_OPEN                    = 101, /* unable to open persistent storage device */
            MCO_E_DISK_ALREADY_OPENED          = 102, /* persistent storage device already opened */
            MCO_E_DISK_NOT_OPENED              = 103, /* persistent storage device not opened */
            MCO_E_DISK_INVALID_PARAM           = 104, /* invalid disk parameters value */
            MCO_E_DISK_PAGE_ACCESS             = 105, /* attempt to access invalid disk page */
            MCO_E_DISK_OPERATION_NOT_ALLOWED   = 106, /* specified disk operation is not allowed */
            MCO_E_DISK_ALREADY_CONNECTED       = 107, /* persistent storage device is already connected */
            MCO_E_DISK_KEY_TOO_LONG            = 108, /* index key is too long */
            MCO_E_DISK_TOO_MANY_INDICES        = 109, /* too many indexes in persistent classes */
            MCO_E_DISK_TOO_MANY_CLASSES        = 110, /* too many persistent classes */
            MCO_E_DISK_SPACE_EXHAUSTED         = 111, /* persistent storage device is out of space */
            MCO_E_DISK_INCOMPATIBLE_LOG_TYPE   = 112, /* incompatible database log type */
            MCO_E_DISK_BAD_PAGE_SIZE           = 113, /* database page size is not acceptable */
            MCO_E_DISK_SYNC                    = 114, /* failed operation on sync primitive */
            MCO_E_DISK_PAGE_POOL_EXHAUSTED     = 115, /* too many pinned disk pages */
            MCO_E_DISK_CLOSE                   = 116, /* error closing persistent storage device */
            MCO_E_DISK_TRUNCATE                = 117, /* unable to truncate persistent storage device */
            MCO_E_DISK_SEEK                    = 118, /* unable to perform seek in persistent storage device */
            MCO_E_DISK_WRITE                   = 119, /* unable to write to persistent storage device */
            MCO_E_DISK_READ                    = 120, /* unable to read from persistent storage device */
            MCO_E_DISK_FLUSH                   = 121, /* unable to flush to persistent storage device */
            MCO_E_DISK_TOO_HIGH_TREE           = 122, /* index too big */
            MCO_E_DISK_VERSION_MISMATCH        = 123, /* version mismatch */
            MCO_E_DISK_CONFLICT                = 124, /* MVCC conflict */
            MCO_E_DISK_SCHEMA_CHANGED          = 125, /* database schema was changed */
            MCO_E_DISK_CRC_MISMATCH            = 126, /* CRC is not matched for the loaded page */
            MCO_E_DISK_TM_MISMATCH             = 127, /* Different transaction manager is used (MVCC/MURSIW) */
            MCO_E_DISK_DICT_LIMITS_MISMATCH    = 128, /* Different max_classes/max_indexes */
            MCO_E_DISK_BTREE_ALLOC             = 129, /* Failed allocate new B-Ttree page: could run out of disk, or out of page pool */
            MCO_E_DISK_CRC_CHECK_MODE_MATCH    = 130, /* Different CRC checking mode is used */

            MCO_E_XML                          = 200,
            MCO_E_XML_INVINT                   = 201, /* invalid integer */
            MCO_E_XML_INVFLT                   = 202, /* invalid float */
            MCO_E_XML_INTOVF                   = 203, /* integer overflow */
            MCO_E_XML_INVBASE                  = 204, /* invalid base for quad (10) */
            MCO_E_XML_BUFSMALL                 = 205, /* buffer too small for double in fixed point format */
            MCO_E_XML_VECTUNSUP                = 206, /* unsupported base type for vector */
            MCO_E_XML_INVPOLICY                = 207, /* invalid xml policy value */
            MCO_E_XML_INVCLASS                 = 208, /* object class different from xml class */
            MCO_E_XML_NO_OID                   = 209, /* first field in the xml object MUST be an OID */
            MCO_E_XML_INVOID                   = 210, /* invalid data in OID field (hex code) */
            MCO_E_XML_INVFLDNAME               = 211, /* invalid field name */
            MCO_E_XML_FLDNOTFOUND              = 212, /* specified field was not found */
            MCO_E_XML_INVENDTAG                = 213, /* invalid closing tag name */
            MCO_E_XML_UPDID                    = 214, /* cannot update OID or AUTOID */
            MCO_E_XML_INVASCII                 = 215, /* invalid xml coding in ascii string */
            MCO_E_XML_INCOMPL                  = 216, /* xml definition incomplete - closing tag not found */
            MCO_E_XML_ARRSMALL                 = 217, /* array is not large enough to hold all elements */
            MCO_E_XML_INVARREL                 = 218, /* invalid name of array element */
            MCO_E_XML_EXTRAXML                 = 219, /* extra xml found after parsing */
            MCO_E_XML_NOTWF                    = 220, /* xml not well-formed */
            MCO_E_XML_UNICODE                  = 221, /* bad unicode conversion */
            MCO_E_XML_NOINDEX                  = 222, /* some classes have no indexes, so database cannot be exported */

            MCO_E_NW                           = 300,
            MCO_E_NW_FATAL                     = 301, /* fatal error */
            MCO_E_NW_NOTSUPP                   = 302, /* network is not supported */
            MCO_E_NW_CLOSE_CHANNEL             = 303, /* error closing network channel **/
            MCO_E_NW_BUSY                      = 304, /* network busy (blocked by another listener?) */
            MCO_E_NW_ACCEPT                    = 305, /* accept failed */
            MCO_E_NW_TIMEOUT                   = 306, /* timeout value exceeded */
            MCO_E_NW_INVADDR                   = 307, /* invalid address specified */
            MCO_E_NW_NOMEM                     = 308, /* host name is too long */
            MCO_E_NW_CONNECT                   = 309, /* connect failed */
            MCO_E_NW_SENDERR                   = 310, /* send failed */
            MCO_E_NW_RECVERR                   = 311, /* receive failed */
            MCO_E_NW_CLOSED                    = 312, /* connection was closed (by the remote host) */
            MCO_E_NW_HANDSHAKE                 = 313, /* handshake failed **/
            MCO_E_NW_CLOSE_SOCKET              = 314, /* error closing socket */
            MCO_E_NW_CREATEPIPE                = 315, /* error creating pipe */
            MCO_E_NW_SOCKET                    = 316, /* socket error */
            MCO_E_NW_SOCKOPT                   = 317, /* setsockopt() error */
            MCO_E_NW_BIND                      = 318, /* bind error */
            MCO_E_NW_SOCKIOCTL                 = 319, /* ioctlsocket() error */
            MCO_E_NW_MAGIC                     = 320, /* invalid magic value */
            MCO_E_NW_INVMSGPARAM               = 321, /* invalid parameter for a message */
            MCO_E_NW_WRONGSEQ                  = 322, /* wrong sequence number for a message */
            MCO_E_NWMCAST_CLOSE_SOCKET         = 323, /* unable to close multicast socket*/
            MCO_E_NWMCAST_SOCKET               = 324, /* multicast socket error */
            MCO_E_NWMCAST_SOCKOPT              = 325, /* unable to set multicast socket option */
            MCO_E_NWMCAST_RECV                 = 326, /* unable to receive data from multicast socket */
            MCO_E_NWMCAST_BIND                 = 327, /* unable to bind multicast socket */
            MCO_E_NWMCAST_NBIO                 = 328, /* unable to ioctl multicast socket */
            MCO_E_NW_KILLED_BY_REPLICA         = 329, /* the master connection was killed by replica */
            MCO_E_NW_WOULDBLOCK                = 330, /* send/recv operation would block */
            MCO_E_NW_SELECT                    = 331, /* send/recv operation would block */
            MCO_E_NW_INVALID_PARAMETER         = 332, /* invalid parameter */

            MCO_E_HA                           = 400, /* HA base error code */
            MCO_E_HA_PROTOCOLERR               = 401, /* error in protocol */
            MCO_E_HA_TIMEOUT                   = 402, /* timeout */
            MCO_E_HA_IOERROR                   = 403, /* input/output error */
            MCO_E_HA_MAXREPLICAS               = 404, /* too many replicas requested */
            MCO_E_HA_INIT                      = 405, /* unable to initialize HA */
            MCO_E_HA_RECEIVE                   = 406, /* error receiving HA message **/
            MCO_E_HA_NO_AUTO_OID               = 407, /* no auto_oid index declared in the database schema */
            MCO_E_HA_NOT_INITIALIZED           = 408, /* HA was not initialized */
            MCO_E_HA_INVALID_MESSAGE           = 409, /* invalid HA message Id */
            MCO_E_HA_INVALID_PARAMETER         = 410, /* invalid parameter */
            MCO_E_HA_INVCHANNEL                = 411, /* invalid channel handler */
            MCO_E_HA_INCOMPATIBLE_MODES        = 412, /* incompatible HA mode */
            MCO_E_HA_CLOSE_TEMP                = 413, /* close temporary multicast channel */
            MCO_E_HA_MULTICAST_NOT_SUPP        = 414, /* multicast is not configured */
            MCO_E_HA_HOTSYNCH_NOT_SUPP         = 415, /* hot synchronization is not configured */
            MCO_E_HA_ASYNCH_NOT_SUPP           = 416, /* asynchronous replication is not configured */
            MCO_E_HA_NO_MEM                    = 417, /* not enough memory to create communication layer descriptors */
            MCO_E_HA_BAD_DESCRIPTOR            = 418, /* ha_t structure is not cleared before creation of base channel */
            MCO_E_HA_CANCEL                    = 419, /* connection was canceled */
            MCO_E_HA_WRONG_DB_MAGIC            = 420, /* wrong DB magic value */
            MCO_E_HA_COMMIT                    = 421, /* master commit error, break commit loop */
            MCO_E_HA_MANYREPLICAS              = 422, /* attempt to attach too many replicas */
            MCO_E_NOT_MASTER                   = 423, /* master-mode was not set */
            MCO_E_HA_STOPPED                   = 424, /* replication was stopped */
            MCO_E_HA_NOWRITETXN                = 425, /* read-write transactions are prohibited on replica */
            MCO_E_HA_PM_BUFFER                 = 426, /* page memory buffer error */
            MCO_E_HA_NOT_REPLICA               = 427, /* not Currently in replica-mode */
            MCO_E_HA_BAD_DICT                  = 428, /* master's db dictionary is incompatible with replica's */
            MCO_E_HA_BINEV_NOT_SUPP            = 429, /* binary schema evolution is not configured */
            MCO_E_HA_CHANNEL_NOT_REGISTERED    = 430, /* channel implementation is not registered */
            MCO_E_HA_DDL_NOT_SUPPORTED         = 431, /* DDL is not supported by HA */
            MCO_E_HA_NO_QUORUM                 = 432, /* Number of active replicas is less than master_params::quorum */
            MCO_S_HA_REPLICA_DETACH            = 433, /* Replica was detached during transaction commit */

            MCO_E_UDA                          = 500, /* UDA base error code */
            MCO_E_UDA_TOOMANY_ENTRIES          = 501, /* allocated entry num exceeded */
            MCO_E_UDA_NAME_TOO_LONG            = 502, /* long entry name */
            MCO_E_UDA_DUPLICATE                = 503, /* duplicate entry name */
            MCO_E_UDA_DICT_NOTFOUND            = 504, /* dictionary (entry) not found by dict_no, name or connection */
            MCO_E_UDA_STRUCT_NOTFOUND          = 505, /* structure not found by struct_no or name */
            MCO_E_UDA_FIELD_NOTFOUND           = 506, /* field not found by field_no or name */
            MCO_E_UDA_INDEX_NOTFOUND           = 507, /* index not found by index_no or name */
            MCO_E_UDA_IFIELD_NOTFOUND          = 508, /* indexed field not found by ifield_no or name */
            MCO_E_UDA_COLLATION_NOTFOUND       = 509, /* collation not found (by collation_no or by name) */
            MCO_E_UDA_STRUCT_NOT_CLASS         = 510, /* structure is not a class, so some operations are not allowed */
            MCO_E_UDA_WRONG_KEY_NUM            = 511, /* key number in lookup() and compare() differs from index spec */
            MCO_E_UDA_WRONG_KEY_TYPE           = 512, /* key types in lookup() and compare() differs from index spec */
            MCO_E_UDA_WRONG_OPCODE             = 513, /* invalid OPCODE (e.g. not MCO_EQ for hash index) */
            MCO_E_UDA_SCALAR                   = 514, /* attempt to get mco_uda_length() on scalar field */
            MCO_E_UDA_NOT_DYNAMIC              = 515, /* attempt to call mco_uda_field_alloc/free() for non-vector or optional struct field */
            MCO_E_UDA_WRONG_VALUE_TYPE         = 516, /* type of value and field are different */
            MCO_E_UDA_READONLY                 = 517, /* Attempt to call mco_uda_put() for read-only fields: oid, autoid, autooid */
            MCO_E_UDA_WRONG_CLASS_CODE         = 518, /* invalid class code */
            MCO_E_UDA_DICT_NOT_DIRECT          = 519, /* in mco_uda_db_open(), entry holds database pointer, not dictionary */
            MCO_E_UDA_INDEX_NOT_USERDEF        = 520, /* Attempt to call mco_uda_register_udf() for non-userdef index */
            MCO_E_UDA_EVENT_NOTFOUND           = 521, /* event not found (by event_no) */

            MCO_E_TL                           = 600, /* TL base error code */
            MCO_E_TL_INVAL                     = 601, /* invalid argument value */
            MCO_E_TL_ALREADY_STARTED           = 602, /* TL already started */
            MCO_E_TL_NOT_STARTED               = 603, /* TL is not started */
            MCO_E_TL_LOG_NOT_OPENED            = 604, /* LOG file is not opened */
            MCO_E_TL_INVFORMAT                 = 605, /* completely corrupted LOG file */
            MCO_E_TL_NOT_INITIALIZED           = 606, /* TL API function called when it is not initialized */
            MCO_E_TL_IO_ERROR                  = 607, /* input/output error */
            MCO_E_TL_NOT_ITERABLE              = 608, /* LOG file created without flag MCO_TRANSLOG_ITERABLE or unsupported transaction manager is applied */
            MCO_E_TL_TRANS_STARTED             = 609, /* function invoked inside transaction */
            MCO_E_TL_PIPE_USED                 = 610, /* pipe still opened by previous instance of reader */
            MCO_E_TL_PIPE_LOST                 = 611, /* pipe were closed in all of writers connections. Keep at least one opened between log start and stop */
            MCO_E_TL_PIPE_TERM                 = 612, /* reader abnormally terminated its job */
            MCO_E_TL_NO_AUTO_OID               = 613, /* no auto_oid index declared in the database schema */
            MCO_E_TL_NOT_APPLICABLE            = 614, /* the only objects related to events mask were written into the LOG file. So it can't be useful to restore database */
            MCO_E_TL_NO_DYNAMIC_PIPE           = 615, /* attempt to attach/detach pipe device for non-dynamic configuration */
            MCO_E_TL_SYNC                      = 616, /* failed operation on TL sync primitive */
            MCO_E_TL_PLAY_STOPPED              = 617, /* function mco_translog_play (iterate) has been terminated by calling of function mco_translog_play_stop */
            MCO_E_TL_PLAY_NOT_STARTED          = 618, /* function mco_translog_play (iterate) has not been started using current connection */

            MCO_E_SEQ_OUT_OF_ORDER             = 700, /* sequence item is out of order */
            MCO_E_SEQ_BOUNDED                  = 701, /* sequence iterator was already bounded */
            MCO_E_SEQ_LENGTH_MISMATCH          = 702, /* length of two sequences doesn't match */
            MCO_E_SEQ_NULL_VALUE               = 703, /* Sequence element is null */

            MCO_E_DDL_NOMEM                    = 800, /* dictionary can not fit in the reserved area */
            MCO_E_DDL_UNDEFINED_STRUCT         = 801, /* struct referenced by field is not yet defined */
            MCO_E_DDL_INVALID_TYPE             = 802, /* unsupported field type */
            MCO_E_DDL_FIELD_NOT_FOUND          = 803, /* reference field is not found in the class */
            MCO_E_DDL_INTERNAL_ERROR           = 804, /* internal error */
            MCO_E_DDL_MCOCOMP_INCOMPATIBILITY  = 805, /* dictionary was generated without -nosort option */
            MCO_E_DDL_TOO_MANY_CLASSES         = 806, /* limit for maximal number of classes in the dictionary is reached */
            MCO_E_DDL_TOO_MANY_INDEXES         = 807, /* limit for maximal number of indexes in the dictionary is reached */
            MCO_E_DDL_TOO_MANY_EVENTS          = 808, /* limit for maximal number of events in the dictionary is reached */

            MCO_E_CLUSTER                      = 900, /* cluster base error code */
            MCO_E_CLUSTER_NOT_INITIALIZED      = 901, /* non-cluster database */
            MCO_E_CLUSTER_INVALID_PARAMETER    = 902, /* invalid cluster parameters value */
            MCO_E_CLUSTER_STOPPED              = 903, /* replication was stopped */
            MCO_E_CLUSTER_PROTOCOLERR          = 904, /* replication protocol error */
            MCO_E_CLUSTER_NOQUORUM             = 905, /* no node's quorum */
            MCO_E_CLUSTER_BUSY                 = 906, /* can't stop cluster with active transactions */
            MCO_E_CLUSTER_INCOMPATIBLE_MODE    = 907, /* incompatible modes on different nodes */
            MCO_E_CLUSTER_SYNC                 = 908, /* error during synchronization */
            MCO_E_CLUSTER_INCOMPATIBLE_ARCH    = 909, /* incompatible architecture of different nodes */
            MCO_E_CLUSTER_DUPLICATE_NODEID     = 910, /* duplicate node ID */
            MCO_E_CLUSTER_DDL_NOT_SUPPORTED    = 911, /* DDL is not supported */
            MCO_E_SAL_RUNTIME_START            = 912, /* unable to start SAL runtime */

            MCO_E_EVAL                         = 999, /* evaluation version limitation */

            MCO_E_PERFMON                      = 1000, /* perfmon base error code */
            MCO_E_PERFMON_NOT_INITIALIZED      = 1001, /* perfmon is not initialized */
            MCO_E_PERFMON_ALREADY_INITIALIZED  = 1002, /* perfmon is already initialized */
            MCO_E_PERFMON_DB_NOT_DETACHED      = 1003, /* not all of monitored databases were detached */
            MCO_E_PERFMON_DB_NOT_ATTACHED      = 1004, /* no database with specified name was attached */

            MCO_E_SCHEMA_ERROR                 = 1005, /* schema compilation error, error callback called with more details */
            MCO_E_NO_DIRECT_ACCESS             = 1006, /* direct access to the structure is not possible */
            MCO_E_ENCRYPTION_NOT_SUPPORTED     = 1007, /* database was created without MCO_DB_INMEMORY_PROTECTION flag */
            MCO_E_NO_CIPHER_KEY                = 1008, /* database was created with MCO_DB_INMEMORY_ENCRYPTION flag but cipher key is not provided in mco_db_connect */
            MCO_E_TOO_HIGH_TREE                = 1009, /* limit for B-Tree height is reached */
            MCO_E_KEY_TOO_LONG                 = 1010, /* limit for B-Tree key length is reached */
            MCO_E_PATRICIA_TOO_DEEP            = 1011, /* PATRICIA trie depth limit reached */
            MCO_E_BTREE_CONFLICT               = 1012, /* B-TREE access CONFLICT */
            MCO_E_TMGR_MISMATCH                = 1013, /* Attempt to connect to shared memory database using different transaction manager */
            MCO_E_SCHEMA_CHANGED               = 1014, /* database schema was changed, so access through wrappers is not possible any more */
            MCO_E_LICENSE_INVALID              = 1015, /* license key invalid */
            MCO_E_BACKUP                       = 1016, /* backup base error */
            MCO_E_BACKUP_PROTOCOL              = 1017, /* backup protocol error */
            MCO_E_BACKUP_NOMEM                 = 1018, /* not enough memory for backup temporary data buffer */
            MCO_E_BACKUP_INVALID_PARAM         = 1019, /* invalid value of backup-related paramter(s) */
            MCO_E_BACKUP_INVALID_FILE          = 1020, /* invalid backup/map file */
            MCO_E_BACKUP_SNAPSHOT_ONLY         = 1021, /* incremental backup processing disabled or no initial snapshot created yet */
            MCO_E_INTERRUPTED                  = 1022, /* query execution was interrupted */
            MCO_E_TRANS_NOT_CLOSED             = 1023, /* transaction is not closed */
            MCO_E_VERIFICATION                 = 1100, /* verification error, look error code for details */

            MCO_E_IOT                          = 1200, /* IoT base error code */
            MCO_E_IOT_NOT_INITIALIZED          = 1201, /* IoT runtime was not initialized */
            MCO_E_IOT_INVALID_HANDLE           = 1202, /* Invalid or uninitialized IoT handle */
            MCO_E_IOT_WRONG_AGENT_ID           = 1203, /* Invalid or unusable agentID */
            MCO_E_IOT_AGENT_NOT_FOUND          = 1204, /* Agent with specified ID was not found */
            MCO_E_IOT_PROTOCOLERR              = 1205, /* IoT protocol error */
            MCO_E_IOT_TS_GAP                   = 1206, /* discontinuous timestamps detected */
            MCO_E_IOT_TS_OUTOFDATE             = 1207, /* repeated timestamps detected */
            MCO_S_IOT_NO_NEW_DATA              = 1208, /* no new data recevied */
            MCO_E_IOT_TOO_MANY_CONTEXTS        = 1209, /* max number of agent contexts exceeded */
            MCO_E_IOT_DUPLICATE_CALLBACK       = 1210, /* IoT callback already registered */
            MCO_E_IOT_CALLBACK_NOT_FOUND       = 1211, /* IoT callback was not registered */
            MCO_E_IOT_INCOMPATIBLE_MODE        = 1212, /* incompatible modes between agents */
            MCO_E_IOT_INCOMPATIBLE_LEVEL       = 1213, /* incompatible levels between agents */
            MCO_E_IOT_STOPPED                  = 1214, /* IoT runtime was stopped */
            MCO_E_IOT_TIMEOUT                  = 1215, /* Timeout for wait has expired */
            MCO_E_IOT_DDL_NOT_SUPPORTED        = 1216, /* DDL is not supported by IoT */

            MCO_E_REST                         = 1300, /* REST base error code */
            MCO_E_REST_SYSTEM                  = 1301, /* system error */
            MCO_E_REST_DB                      = 1302, /* database error */
            MCO_E_REST_PARAM                   = 1303, /* a function parameter is invalid */
            MCO_E_REST_HTTP                    = 1304, /* HTTP protocol error */
            MCO_E_REST_NOT_FOUND               = 1305, /* the requested resource was not found */
            MCO_E_REST_JSON                    = 1306, /* JSON encoding error */
            MCO_E_REST_INUSE                   = 1307, /* resource is in use */
            MCO_E_REST_EOF                     = 1308, /* end of file or stream reached */
            MCO_E_REST_ADDRNOTAVAIL            = 1309, /* the specified address is not available */

            MCO_E_JSER_NOINDEX                 = 1400, /* some classes have no indexes, so database cannot be exported */

            /* unrecoverable (fatal) error code bases */
            MCO_ERR_DB                         = 100000,   /* database */
            MCO_ERR_DICT                       = 110000,   /* dictionary */
            MCO_ERR_CURSOR                     = 120000,   /* cursor */
            MCO_ERR_PMBUF                      = 130000,   /* pm buffer */
            MCO_ERR_COMMON                     = 140000,   /* common routines */
            MCO_ERR_HEAP                       = 150000,   /* heap manager */
            MCO_ERR_OBJ                        = 160000,   /* object allocator */
            MCO_ERR_BLOB                       = 170000,   /* blob operation */
            MCO_ERR_FREC                       = 180000,   /* record allocator */
            MCO_ERR_VOLUNTARY                  = 190000,   /* voluntary index */
            MCO_ERR_LOADSAVE                   = 200000,   /* db save and load */
            MCO_ERR_PGMEM                      = 210000,   /* page memory */
            MCO_ERR_EV_SYN                     = 220000,   /* synchronous events */
            MCO_ERR_EV_ASYN                    = 230000,   /* async events */
            MCO_ERR_EV_W                       = 240000,   /* event wrappers */
            MCO_ERR_XML_W                      = 250000,   /* XML serialization */
            MCO_ERR_XML_SC                     = 260000,   /* XML schema */
            MCO_ERR_BTREE                      = 270000,   /* btree */
            MCO_ERR_HASH                       = 280000,   /* hash */
            MCO_ERR_RECOV                      = 290000,   /* recovery */
            MCO_ERR_FCOPY                      = 300000,   /* copy fields */
            MCO_ERR_INST                       = 330000,   /* db instance */
            MCO_ERR_TRN                        = 340000,   /* transaction */
            MCO_ERR_TMGR                       = 370000,   /* transaction manager */
            MCO_ERR_SYNC                       = 400000,   /* general sync */
            MCO_ERR_ORDER                      = 450000,   /* ordering and hash index */
            MCO_ERR_SEM                        = 460000,   /* semaphores */
            MCO_ERR_SHM                        = 470000,   /* shared memory */
            MCO_ERR_SER                        = 500000,   /* serialization */
            MCO_ERR_HA                         = 510000,   /* high availability */
            MCO_ERR_DB_NOMEM                   = 520000,   /* insufficient memory */
            MCO_ERR_OBJECT_HANDLE              = 530000,   /* invalid object handle */
            MCO_ERR_UNSUPPORTED_FLOAT          = 540000,   /* support of float type is disabled */
            MCO_ERR_UNSUPPORTED_DOUBLE         = 550000,   /* deprecated */

            MCO_ERR_DB_NOMEM_HASH              = 560000,   /* insufficient memory in hash index */
            MCO_ERR_DB_NOMEM_HEAP              = 570000,   /* insufficient memory in heap manager */
            MCO_ERR_DB_NOMEM_TRANS             = 580000,   /* insufficient memory in transaction manager */
            MCO_ERR_DB_NAMELONG                = 590000,   /* database name is too long */
            MCO_ERR_DB_VERS_MISMATCH           = 600000,   /* version of eXtremeDB runtime mismatch */
            MCO_ERR_RUNTIME                    = 610000,   /* invalid type of runtime library */
            MCO_ERR_INMEM_ONLY_RUNTIME         = 620000,   /* persistent class in the schema but inmem-only runtime */

            MCO_ERR_DISK                       = 700000,   /* general disk error */
            MCO_ERR_DISK_WRITE                 = 710000,   /* unable to write to persistent storage */
            MCO_ERR_DISK_READ                  = 720000,   /* unable to read from persistent storage */
            MCO_ERR_DISK_FLUSH                 = 730000,   /* unable to flush to persistent storage */
            MCO_ERR_DISK_CLOSE                 = 740000,   /* error closing persistent storage device */
            MCO_ERR_DISK_TRUNCATE              = 750000,   /* unable to truncate persistent storage device */
            MCO_ERR_DISK_SEEK                  = 760000,   /* unable to seek in persistent storage device */
            MCO_ERR_DISK_OPEN                  = 770000,   /* unable to open persistent storage device */
            MCO_ERR_DISK_ALREADY_OPENED        = 780000,   /* persistent storage device already opened */
            MCO_ERR_DISK_NOT_OPENED            = 790000,   /* persistent storage device was not opened */
            MCO_ERR_DISK_INVALID_PARAM         = 800000,   /* invalid parameter value */
            MCO_ERR_DISK_PAGE_ACCESS           = 810000,   /* page access fatal error */
            MCO_ERR_DISK_INTERNAL_ERROR        = 820000,   /* internal fatal error */
            MCO_ERR_DISK_OPERATION_NOT_ALLOWED = 830000,   /* operation not allowed fatal error */
            MCO_ERR_DISK_ALREADY_CONNECTED     = 840000,   /* persistent storage device already connected */
            MCO_ERR_DISK_TOO_MANY_INDICES      = 850000,   /* too many indexes in persistent classes */
            MCO_ERR_DISK_TOO_MANY_CLASSES      = 860000,   /* too many persistent classes */
            MCO_ERR_DISK_SPACE_EXHAUSTED       = 870000,   /* persistent storage device out of space */
            MCO_ERR_DISK_PAGE_POOL_EXHAUSTED   = 880000,   /* too many pinned disk pages */
            MCO_ERR_DISK_INCOMPATIBLE_LOG_TYPE = 890000,   /* incompatible database log type */
            MCO_ERR_DISK_BAD_PAGE_SIZE         = 900000,   /* page size is not acceptable */
            MCO_ERR_DISK_SYNC                  = 910000,   /* failed operation on sync. primitive */
            MCO_ERR_DISK_CRC                   = 920000,   /* wrong CRC code of disk page */
            MCO_ERR_DISK_FORMAT_MISMATCH       = 930000,   /* incompatible configuration changes affecting disk database format */
            MCO_ERR_CHECKPIN                   = 940000,   /* unbalanced pin/unpin */
            MCO_ERR_CONN                       = 950000,   /* connection processing */
            MCO_ERR_REGISTRY                   = 960000,   /* registry processing */
            MCO_ERR_INDEX                      = 970000,   /* index processing */
            MCO_ERR_VTMEM                      = 980000,   /* in-memory only runtime */
            MCO_ERR_VTDSK                      = 990000,   /* mixed runtime */
            MCO_ERR_RTREE                      = 1000000,  /* rtree index */
            MCO_ERR_UDA                        = 1010000,  /* Uniform Data Access */
            MCO_ERR_PTREE                      = 1020000,  /* Patricia tree index */
            MCO_ERR_TL                         = 1030000,  /* transaction log */
            MCO_ERR_CLUSTER                    = 1040000,  /* cluster */
            MCO_ERR_CLNWTCP                    = 1050000,  /* cluster TCP transport */
            MCO_ERR_SEQ                        = 1060000,  /* sequence errors */
            MCO_ERR_NESTED_TRANS_TRAP          = 1090000,  /* attempt to start nested transaction detected */
            MCO_ERR_PERFMON                    = 1100000,  /* performance monitoring */
            MCO_ERR_AIO                        = 1110000,  /* asynchronous IO */
            MCO_ERR_CLNWMPI                    = 1120000,  /* cluster MPI transport */
            MCO_ERR_DDL                        = 1130000,  /* DDL errors */
            MCO_ERR_SQL_EXCEPTION              = 1140000,  /* SQL exception */
            MCO_ERR_BACKUP                     = 1150000,  /* BACKUP feature */
            MCO_ERR_ACTIVE_TRANSACTION         = 1160000,  /* Attempt to disconnect an active transaction */
            MCO_ERR_NETWORK                    = 1170000,  /* Network errors */
            MCO_ERR_IOT_COMM                   = 1180000,  /* IoT communicator errors */
            MCO_ERR_IOT_REPL                   = 1190000,  /* IoT replicator errors */
            MCO_ERR_LAST                       = 1999999   /* the latest one */

        } MCO_RET; /* return code */

        #define mco_min_page_size    ( 16*sizeof(mco_offs_t))   /* min size of a page    */
        #define mco_cursor_size      (100*sizeof(mco_offs_t))   /* cursor structure size */
        #define mco_handle_size      ( 16*sizeof(mco_offs_t))   /* object handle size    */

        /* boolean type */
        typedef int mco_bool;
        #define MCO_YES 1
        #define MCO_NO  0

        #define mco_trans_size       20   /* max number of updated or deleted objects in one transaction */
        #define mco_handle_size_w    ( mco_handle_size / sizeof(MCO_PUB_STRUCT_BASE) )   /* defined in mcoquad.h */
        #define mco_cursor_size_w    ( mco_cursor_size / sizeof(MCO_PUB_STRUCT_BASE) )

        typedef MCO_PUB_STRUCT_BASE MCO_Hf[mco_handle_size_w];
        MCO_RET mco_get_class_code ( void * object_handle, uint2 * class_code);

        typedef struct mco_cursor_t_
        {
            MCO_PUB_STRUCT_BASE c[mco_cursor_size_w];
        } mco_cursor_t,  /* cursor (structure)            */
        *mco_cursor_h; /* cursor handle (pointer)       */

        #ifdef MCO_NO_FORWARDS

            typedef void* mco_trans_h;
            typedef void* mco_db_h;
            typedef void* mco_dictionary_h;
            typedef void* mco_calculator_h;
            typedef void* mco_collate_h;

        #else

            struct mco_trans_t_;
            typedef struct mco_trans_t_* mco_trans_h; /* transaction handle (pointer)  */

            struct mco_db_t_;
            typedef struct mco_db_t_* mco_db_h; /* database handle (pointer)     */

            struct mco_dictionary_t_;
            typedef struct mco_dictionary_t_* mco_dictionary_h; /* dictionary handle (pointer)   */

            struct mco_calculator_t_;
            typedef struct mco_calculator_t_* mco_calculator_h; /* calculator handle (pointer) */

            struct mco_collate_t_;
            typedef struct mco_collate_t_* mco_collate_h; /* custom collation handle (pointer) */
        #endif

        /* transaction priority values */
        /* Transaction Manager runs queued transaction according numeric value of their priorities */
        typedef enum MCO_TRANS_PRIORITY_E_
        {
            MCO_TRANS_IDLE       = -2,   /* Least important transactions */
            MCO_TRANS_BACKGROUND = -1,
            MCO_TRANS_FOREGROUND = 0,    /* Normal priority transactions */
            MCO_TRANS_HIGH       = 1,
            MCO_TRANS_ISR        = 2     /* Very Important Transactions */
        } MCO_TRANS_PRIORITY;

        typedef enum MCO_TRANS_ISOLATION_LEVEL_E_
        {
            MCO_DEFAULT_ISOLATION_LEVEL = 0x0, /* MCO_REPEATABLE_READ for MVCC,  MCO_SERIALIZABLE for GlobalLock/MURSIW */
            MCO_READ_COMMITTED          = 0x1, /* Guarantees that any data read was committed at the moment is read. It restricts the reader from seeing any uncommitted data. */
            MCO_REPEATABLE_READ         = 0x2, /* In addition to MCO_READ_COMMITTED guarantees that any data read cannot change, if the transaction reads the same data again,
                                                  it will find the previously read data in place, unchanged, and available to read. */
            MCO_SERIALIZABLE            = 0x4  /* Emulates serial transaction execution */
        } MCO_TRANS_ISOLATION_LEVEL;

        /* transaction type values */
        typedef enum MCO_TRANS_TYPE_E_
        {
            MCO_READ_ONLY = 0, MCO_UPDATE = 1, MCO_READ_WRITE = 2, MCO_EXCLUSIVE = 3
        } MCO_TRANS_TYPE;

		/* wait event types used for profiling */
		typedef enum MCO_WAIT_EVENT_E_
		{
			MCO_WAIT_EVENT_NONE,
			MCO_WAIT_EVENT_READ,
			MCO_WAIT_EVENT_WRITE,
			MCO_WAIT_EVENT_FLUSH,
			MCO_WAIT_EVENT_MURSIW_TM_LOCK,
			MCO_WAIT_EVENT_MURSIW_TRANS_LOCK,
			MCO_WAIT_EVENT_MURSIW_READER_LOCK,
			MCO_WAIT_EVENT_MURSIW_WRITER_LOCK,
			MCO_WAIT_EVENT_MVCC_CS_LOCK,
			MCO_WAIT_EVENT_MVCC_TM_LOCK,
			MCO_WAIT_EVENT_MVCC_PM_MUTEX,
			MCO_WAIT_EVENT_MVCC_EXCLUSIVE_LOCK,
			MCO_WAIT_EVENT_MVCC_SERIALIZABLE_LOCK,
			MCO_WAIT_EVENT_MVCC_TREE_READ_LOCK,
			MCO_WAIT_EVENT_MVCC_TREE_WRITE_LOCK,
			MCO_WAIT_EVENT_MVCC_LEAF_READ_LOCK,
			MCO_WAIT_EVENT_MVCC_LEAF_WRITE_LOCK,
			MCO_WAIT_EVENT_MVCC_BLOB_MUTEX,
			MCO_WAIT_EVENT_MVCC_HASH_WRITE_LOCK,
			MCO_WAIT_EVENT_MVCC_HASH_READ_LOCK,
			MCO_WAIT_EVENT_MVCC_KDTREE_WRITE_LOCK,
			MCO_WAIT_EVENT_MVCC_KDTREE_READ_LOCK,
			MCO_WAIT_EVENT_MVCC_PTREE_WRITE_LOCK,
			MCO_WAIT_EVENT_MVCC_PTREE_READ_LOCK,
			MCO_WAIT_EVENT_MVCC_RTREE_WRITE_LOCK,
			MCO_WAIT_EVENT_MVCC_RTREE_READ_LOCK,
			MCO_WAIT_EVENT_MVCC_FRA_MUTEX,
			MCO_WAIT_EVENT_MVCC_DB_MUTEX,
			MCO_WAIT_EVENT_MVCC_PAGE_LATCH,
			MCO_WAIT_EVENT_MVCC_TREE_LATCH,
			MCO_WAIT_EVENT_MVCC_BUNDLE_LATCH,
			MCO_WAIT_EVENT_MVCC_COMPRESSOR_LATCH,
			MCO_WAIT_EVENT_DM_MUTEX,
			MCO_WAIT_EVENT_RAW_PAGE,
			MCO_WAIT_EVENT_DM_ALLOC_MUTEX,
			MCO_WAIT_EVENT_EXCLUSIVE_COMMIT_LOCK,
			MCO_WAIT_EVENT_SHARED_COMMIT_LOCK,
			MCO_WAIT_EVENT_ASYNC_LOCK,
			MCO_WAIT_EVENT_ASYNC_MUTEX,
			MCO_WAIT_EVENT_HEADER_LOCK,
			MCO_WAIT_EVENT_WCHAR_BUF_MUTEX,
			MCO_WAIT_EVENT_SOCKET_RECV,
			MCO_WAIT_EVENT_SOCKET_SEND,
			MCO_WAIT_EVENT_LAST
		} MCO_WAIT_EVENT;

        /* scheduling policy for transactions with the same priority */
        typedef enum MCO_TRANS_SCHED_POLICY_E_
        {
            MCO_SCHED_FIFO = 0,                    /* First In First Out */
            MCO_SCHED_READER_FAVOR = 1,            /* Run read-only transactions first */
            MCO_SCHED_WRITER_FAVOR = 2             /* Run read-write transactions first */
        } MCO_TRANS_SCHED_POLICY;

         /* Device management */
        #define MCO_MEMORY_NULL                  0  /* an invalid device */
        #define MCO_MEMORY_CONV                  1  /* a conventional memory region (non-shared memory) */
        #define MCO_MEMORY_NAMED                 2  /* a named memory region (shared memory) */
        #define MCO_MEMORY_FILE                  3  /* a file device */
        #define MCO_MEMORY_MULTIFILE             4  /* a multifile device */
        #define MCO_MEMORY_RAID                  5   /* a RAID device */
        #define MCO_MEMORY_INT_DESC              6   /* an integer descripted region (shard memory) */
        #define MCO_MEMORY_CYCLIC_FILE_BUF       7   /* pair of files to organize cyclic buffer */

        #define MCO_MEMORY_ASSIGN_DATABASE       0   /* a memory region for meta- and user-data, indices and other database structures */
        #define MCO_MEMORY_ASSIGN_CACHE          1   /* a memory region for disk caching purposes */
        #define MCO_MEMORY_ASSIGN_PERSISTENT     2   /* a persistent storage device (file, multi-file) */
        #define MCO_MEMORY_ASSIGN_LOG            3   /* a log device (file, multi-file) */
        #define MCO_MEMORY_ASSIGN_HA_ASYNC_BUF   4   /* an async buffer device for HA */
        #define MCO_MEMORY_ASSIGN_PIPE_BUF       5   /* buffer for TL pipe */

        #ifdef MCO_CFG_MINIMAL
            #define MCO_MAX_MEMORY_NAME      32
            #define MCO_MAX_FILE_NAME        32
            #define MCO_MAX_MULTIFILE_NAME   32
        #else
            #define MCO_MAX_MEMORY_NAME      64
            #define MCO_MAX_FILE_NAME       256
            #define MCO_MAX_MULTIFILE_NAME   64
        #endif

        typedef struct mco_device_t_ {
            unsigned int type;       /* none, conv, named, file, raid, etc */
            unsigned int assignment; /* none, db-segment, cache-segment, db-file, log-file */
            mco_size_t   size;
            union {
                struct {
                    void * ptr;
                    int flags;
                } conv;
                struct {
                    char name[MCO_MAX_MEMORY_NAME];
                    unsigned int flags;
                    void * hint;
                } named;
                struct {
                    int flags;
                    char name[MCO_MAX_FILE_NAME];
                } file;
                struct {
                    int flags;
                    char name[MCO_MAX_MULTIFILE_NAME];
                    mco_offs_t segment_size;
                } multifile;
                struct {
                    int flags;
                    char name[MCO_MAX_MULTIFILE_NAME];
                    int level;
                    mco_offs_t offset;
                } raid;
                struct {
                    unsigned long handle;
                } idesc;
            } dev;
        } mco_device_t, *mco_device_h;

        /* Database management */

        MCO_RET mco_runtime_start(void);
        MCO_RET mco_runtime_start_ex(mco_bool disk, mco_bool mvcc, mco_bool shmem,
             		const char *sync_library, const char *memory_library, const char *runtime_path);
        MCO_RET mco_runtime_stop(void);
        unsigned int mco_current_registry_size(void);
        MCO_RET mco_close_all_instances(void); /* for multiprocessing only */
        MCO_RET mco_db_databases(char* lpBuffer, mco_size32_t buffer_size, mco_counter32_t skip_first);

        /* #ifdef MCO_CFG_USERDEF_IDX */
        typedef int2  (*mco_compare_userdef_f)           ( MCO_Hf * obj1, MCO_Hf * obj2);
        typedef int2  (*mco_compare_extkey_userdef_f)    ( MCO_Hf * obj1, void * key);
        /* Vector-based version:  */
        typedef int2  (*mco_compare_userdef_vb_f)        ( MCO_Hf * obj1, int index1, MCO_Hf * obj2, int index2);
        typedef int2  (*mco_compare_extkey_userdef_vb_f) ( MCO_Hf * obj1, int index1, void * key);

        typedef mco_hash_counter_t (*mco_hash_userdef_f)              ( MCO_Hf * obj);
        typedef mco_hash_counter_t (*mco_hash_extkey_userdef_f)       ( void * key);
        /* Vector-based version:  */
        typedef mco_hash_counter_t (*mco_hash_userdef_vb_f)           ( MCO_Hf * obj, int index);
        /* Special version (for UDA) */
        typedef mco_hash_counter_t (*mco_hash_extkey_userdef_spec_f)  ( void * key, mco_db_h db, uint2 index_num);

        #define MCO_UDF_SPECIAL_EXTHASH   0x1

        struct mco_userdef_funcs_t_ {
            union {
                mco_compare_userdef_f            nvb;
                mco_compare_userdef_vb_f         vb;
            } fcomp;
            union {
                mco_compare_extkey_userdef_f     nvb;
                mco_compare_extkey_userdef_vb_f  vb;
            } fcomp_ext;
            union {
                mco_hash_userdef_f               nvb;
                mco_hash_userdef_vb_f            vb;
            } fhash;
            union {
                mco_hash_extkey_userdef_f        u;
                mco_hash_extkey_userdef_spec_f   spec;
            } fhash_ext;
            union {
                uint4 flags;
                void  *user_context;
            } p;
        };
        typedef  struct mco_userdef_funcs_t_  mco_userdef_funcs_t;
        typedef  struct mco_userdef_funcs_t_ *mco_userdef_funcs_h;

        MCO_RET     mco_db_register_udf ( const char * db_name, mco_userdef_funcs_h udfs);
        /* #endif MCO_CFG_USERDEF_IDX */

        typedef int2 (*mco_compare_collation_f) ( mco_collate_h c1, uint2 len1, mco_collate_h c2, uint2 len2);
        typedef mco_hash_counter_t (*mco_hash_collation_f) ( mco_collate_h c, uint2 len);
        typedef struct mco_collation_funcs_t_ {
            mco_compare_collation_f comp;
            mco_hash_collation_f    hash;
        } mco_collation_funcs_t, *mco_collation_funcs_h;

        MCO_RET     mco_db_register_collations ( const char * db_name, mco_collation_funcs_h colls);

        typedef enum MCO_COMMIT_POLICY_E
        {
           MCO_COMMIT_SYNC_FLUSH, /* default  */
           MCO_COMMIT_BUFFERED,   /* runtime buffered transactions */
           MCO_COMMIT_DELAYED,    /* runtime buffered transactions, with commit criteria */
           MCO_COMMIT_NO_SYNC     /* changes are not synchronously written to the disk */
        } MCO_COMMIT_POLICY;

        #define MCO_PARAM_NOT_SPECIFIED 0
        typedef struct mco_log_params_t_
        {
            MCO_COMMIT_POLICY default_commit_policy;
            mco_offs_t        redo_log_limit;
            mco_offs_t        delayed_commit_threshold;
            mco_counter32_t   max_delayed_transactions;
            uint4             max_commit_delay; /* in milliseconds */
        } mco_log_params_t;

        typedef enum MCO_LOG_TYPE_ {
            NO_LOG, REDO_LOG, UNDO_LOG
        } MCO_LOG_TYPE;

        typedef enum MCO_DB_MODE_MASK_ {
            MCO_DB_MODE_MVCC_AUTO_VACUUM      = 0x01,
            MCO_DB_MODE_SMART_INDEX_INSERT    = 0x02,
            MCO_DB_OPEN_EXISTING              = 0x04,
            MCO_DB_USE_CRC_CHECK              = 0x08,
            MCO_DB_TRANSIENT                  = 0x10,
            MCO_DB_LAZY_MEM_INITIALIZATION    = 0x20,
            MCO_DB_MURSIW_DISK_COMMIT_OPTIMIZATION = 0x40,
            MCO_DB_BULK_WRITE_MODIFIED_PAGES = 0x80,
            MCO_DB_INDEX_PRELOAD = 0x0200,
            MCO_DB_DISABLE_NESTED_TRANSACTIONS = 0x0400,
            MCO_DB_DISABLE_IMPLICIT_ROLLBACK = 0x0800,
            MCO_DB_INMEMORY_PROTECTION = 0x1000,
            MCO_DB_INCLUSIVE_BTREE = 0x2000,
            MCO_DB_INMEMORY_COMPRESSION = 0x4000,
            MCO_DB_SEPARATE_BITMAP = 0x8000,
            MCO_DB_DISABLE_BTREE_REBALANCE_ON_DELETE = 0x10000,
            MCO_DB_AUTO_ROLLBACK_FIRST_PHASE = 0x20000,
            MCO_DB_MVCC_COMPATIBILITY_MODE = 0x40000,
            MCO_DB_DISABLE_PAGE_POOL_RESERVE = 0x80000,
            MCO_DB_REDO_LOG_OPTIMIZATION = 0x100000,
            MCO_DB_DISABLE_HOT_UPDATES = 0x200000,
            MCO_DB_SQL_AUTOCHECKPOINT = 0x400000,
            MCO_DB_MODE_READ_ONLY = 0x800000,
            MCO_DB_USE_AIO = 0x1000000,
            MCO_DB_INCREMENTAL_BACKUP = 0x2000000,
			MCO_DB_MVCC_TABLE_LEVEL_LOCKING = 0x4000000, /* by default fine-grain locking is used */
			MCO_DB_DISABLE_SMART_ALLOC = 0x8000000
        } MCO_DB_MODE_MASK;

        /* Compression masks applicable to compression_mask */
        typedef enum MCO_COMPRESSION_MASK_ {
            MCO_COMPRESSION_OBJ_HEAD    = 1,
            MCO_COMPRESSION_OBJ_NODE    = 2,
            MCO_COMPRESSION_BLOB_HEAD   = 64,
            MCO_COMPRESSION_BLOB_TAIL   = 128,
            MCO_COMPRESSION_FIXEDRECSET = 4096,
            MCO_COMPRESSION_ALL = MCO_COMPRESSION_OBJ_HEAD | MCO_COMPRESSION_OBJ_NODE | MCO_COMPRESSION_FIXEDRECSET
                                    | MCO_COMPRESSION_BLOB_HEAD | MCO_COMPRESSION_BLOB_TAIL
        } MCO_COMPRESSION_MASK;

        #define MCO_INFINITE_DATABASE_SIZE 0

        typedef struct mco_db_params_t_ {
            uint2            mark;                    /**/
            uint2            mem_page_size;           /* memory page size */
            uint4            disk_page_size;          /* disk page size, typical 4096 bytes, set to zero for in-memory only database */
            uint4            db_max_connections;      /* max number of connections */
            mco_offs_t       disk_max_database_size;  /* max size of persistent database */
            mco_offs_t       file_extension_quantum;  /* quantum of increasing size of database file (can help to reduce file fragmentation) */
            MCO_LOG_TYPE     db_log_type;             /* database logging method */
            uint2            connection_context_size; /* recovery connection context */

            uint2            hash_load_factor;        /* criteria of hash reallocation in percent of used items */
            uint2            index_optimistic_lock_threshold; /* maximum number of active write transactions when
                                                                 optimistic locking of B-Tree indexes can be performed */
            mco_log_params_t log_params;              /* initial log settings */

            int              mode_mask;               /* open mode mask */
            int              min_conn_local_pages;    /* minimal number of pages in per-connection allocator */
            int              max_conn_local_pages;    /* maximal number of pages in per-connection allocator */

            int              allocation_bitmap_caching_priority; /* priority of allocator bitmap pages in LRU page pool */
            int              index_caching_priority;  /* priority of indices pages in LRU page pool */
            int              object_caching_priority; /* priority of objects (non-blob) pages in LRU page pool */

            struct mco_ddl_dictionary_t_* ddl_dict;
            mco_size_t       ddl_dict_size;
            int              ddl_dict_flags;

            char*            cipher_key;             /* database encryption key */
            mco_bool         dynamic_hash;           /* dynamically extend hash table when number of items exceeds load factor */
            char*            license_key;            /* eXtremeDB license key */

            int              max_classes;            /* used to reserve space for classes in database header to allow dynamic table creation */
            int              max_indexes;            /* used to reserve space for indexes in database header to allow dynamic table creation */

            mco_size_t       autocompact_threshold;  /* size of object after reaching which mco_w_compact is called during transaction commit */

            MCO_TRANS_SCHED_POLICY trans_sched_policy; /* scheduling policy for transactions with the same priority */
            uint8            max_trans_time;         /* maximal transaction time, 0 to disable */
            int              max_active_pages;       /* maximal number of pages accessed by connection */
            int              page_hash_bundles;      /* number of bundles in encryption/protection/compression page hash */
            int              compression_level;      /* compression level: 0..9, 0 - no compression, -1: default compression level */
            int              compression_mask;       /* bitmap of page kinds which should be compressed */
            int              expected_compression_ratio;   /* use to allocate page map: virtual database space can not be larger than physical */
                                                           /* size compression ratio                                                           */
            uint1            btree_cursor_read_ahead_size; /* number of keys taken from leaf B-Tree page at each access, can not be larger     */
                                                           /* than MCO_CFG_CURSOR_BUF_SIZE                                                     */
            int              mvcc_bitmap_size;       /* MVCC bitmap size should be power of two */
            int              additional_heap_size;   /* heap memory used by various internal DB functionalities, mostly for buffering during   */
                                                     /* XML export/import and database save/load operations                                    */
			mco_size_t       cow_pagemap_size;       /* Size of copy-on-write MVCC pagemap (in bytes, one byte per disk page) */

            mco_size_t       backup_map_size;        /* size of backup counters array, bytes, power of two, ignored if disk_max_database_size set */
                                                     /* set to zero do disable backup feature */
            unsigned int     backup_min_pages;       /* number of pages for last exclusive pass of backup procedure, set to zero to disable treshold */
            unsigned int     backup_max_passes;      /* max number of passes before exclusive pass of backup procedure */
            char             backup_map_filename[MCO_MAX_FILE_NAME]; /* name of a file will be used to store backup temporary data on mco_db_close() call */
                                                     /* eXtremeDB will use filesystem wrapper linked into the process to store the data   */
                                                     /* optional, set zero for default "<persistent-storage>.bmap" file locate at the same   */
                                                     /* location as persistent storage file  */
            uint8            iot_agent_id;
            uint2            iot_level;
			uint4            file_backup_delay;      /* delay in milliseconds between writing backup blocks to minimize backup impact on performance */
        } mco_db_params_t;

        /* Set default values. Call this first then customize the params according to your requirements. */
        void    mco_db_params_init( mco_db_params_t * params );

        /* The general approach is to call mco_db_open_dev() and mco_db_extend_dev() to open or extend a database
        on specified memory devices with the specified params */
        MCO_RET mco_db_open_dev   ( const char* db_name, mco_dictionary_h dict, mco_device_t * devs, mco_size_t n_devs, mco_db_params_t * params );
        MCO_RET mco_db_extend_dev ( const char* db_name, mco_device_t * dev );
        MCO_RET mco_db_extend_dev_ctx ( const char* db_name, mco_device_t * dev, void const* context );
        MCO_RET mco_db_extend_dev_t ( mco_trans_h t, mco_device_t * dev );

        typedef void (*mco_detach_dev_callback_t)(const char *db_name, mco_device_t *dev, MCO_RET destroy_rc);
        MCO_RET mco_register_detach_dev_callback(mco_detach_dev_callback_t callback, mco_bool async_only);
        MCO_RET mco_db_detach_dev ( const char* db_name, mco_device_t * dev );
        MCO_RET mco_db_detach_dev_ctx ( const char* db_name, mco_device_t * dev, void const* context );

        /* The following are provided to open or extend a database on conventional memory and for backward compatibility. */
        MCO_RET mco_db_open       ( const char* db_name, mco_dictionary_h dict, void* mem_ptr, mco_size_t total_size, uint2 page_size );
        MCO_RET mco_db_extend     ( const char* db_name, void* mem_ptr, mco_size_t size);
        MCO_RET mco_db_extend_ctx ( const char* db_name, void* mem_ptr, mco_size_t size, void const* context );
        MCO_RET mco_db_extend_t   ( mco_trans_h t, void* mem_ptr, mco_size_t size);
        /* This function provides compatibility with 3.5 version and replaces combination of mco_db_open+mco_disk_open function calls */
        MCO_RET mco_db_open_disk( const char* db_name, mco_dictionary_h dict, void* mem_ptr, mco_size_t mem_size, uint2 mem_page_size,
                                  const char* db_file_path, const char* log_file_path, void* disk_cache_ptr, mco_size_t disk_cache_size,
                                  int disk_page_size, mco_offs_t max_database_size, MCO_LOG_TYPE log_type);
        /* close a database */
        MCO_RET mco_db_close      ( const char* db_name );

        /* terminate a database */
        MCO_RET mco_db_kill       ( const char* db_name );

        /* merge dictionaries */
        mco_dictionary_h mco_merge_dictionaries(mco_dictionary_h* dictionaries, int n_dicts, char* dict_buf, mco_size_t dict_buf_size);

        MCO_RET mco_dict_version(mco_db_h db, int *dict_version);

        /* connect to an existing database specifying a database context */
        MCO_RET mco_db_connect_ctx( const char* db_name, void const* context, /*OUT*/ mco_db_h* pdb);

        /* connect to an existing database without a connection context */
        MCO_RET mco_db_connect    ( const char* db_name, /*OUT*/ mco_db_h* pdb);

        /* get the context associated with a connection */
        void*   mco_db_connection_context( mco_db_h db );

        /* close a connection */
        MCO_RET mco_db_disconnect ( mco_db_h db );

        /* return connection numbers stats */
        MCO_RET mco_db_connection_info(const char* name, unsigned int * p_nconnections_max, unsigned int * p_nconnections_current );

        /* remove all objects from a database */
        MCO_RET mco_db_clean      ( mco_db_h db );

        MCO_RET mco_db_recover(mco_db_h db, mco_db_h recovered_connection);
        MCO_RET mco_db_recover_all(mco_db_h db);

        MCO_RET mco_db_free_pages(mco_db_h db,  /*OUT*/mco_size32_t* retvalue);
        MCO_RET mco_db_total_pages(mco_db_h db,  /*OUT*/mco_size32_t* retvalue);
        MCO_RET mco_db_page_size(mco_db_h db,  /*OUT*/uint2* retvalue);

        /**
         * This enum defines sniffer policy: when sniffer should call the user supplied callback function to check
         * the status of a connection.
         */
        typedef enum mco_sniffer_policy {
            MCO_SNIFFER_INSPECT_ACTIVE_CONNECTIONS,
            MCO_SNIFFER_INSPECT_ACTIVE_TRANSACTIONS,
            MCO_SNIFFER_INSPECT_HANGED_TRANSACTIONS

        } mco_sniffer_policy;
        /**
         * This typedef defines the prototype for a user defined callback function that detects dead connections.
         * This function must return MCO_S_DEAD_CONNECTION to force database recovery, MCO_S_OK if the connection is ok.
         * From the eXtremeDB runtime's point of view the two parameters "db" and "context" are opaque data.
         * (NB: "context" is typically supplied by calling function mco_db_connect_ctx() which fills in a data block
         * of size MCO_CFG_CONNECTION_CONEXT_SIZE).  Parameter "trans_no" is the number of active transaction
         * (0 if no active transaction).  The function must return MCO_S_DEAD_CONNECTION if the connection associated
         * with "context" is considered dead and should be recovered, MCO_S_OK otherwise. Any other return code will
         * cause interruption of mco_db_sniffer which will return this error code to the calling process.
         */
        typedef MCO_RET (*mco_sniffer_callback_t)(mco_db_h db, void* context, mco_trans_counter_t trans_no);
        /**
         * The same as mco_sniffer_callback_t but it is additionally possible to pass user-defined context into callback function
         */
        typedef MCO_RET (*mco_sniffer_callback_ex_t)(mco_db_h db, void* context, mco_trans_counter_t trans_no, void *user_context);
        /**
         * Call mco_db_sniffer() to inspect the status of all database connections using the user provided callback
         * function, and perform database recovery if dead connections were detected.
         * Parameter "db" is a connection handle, and "callback" is the user defined function implementing logic
         * (using OS, timeout, watchdogs, etc.) to detect dead database connections.  If this callback returns
         * MCO_S_DEAD_CONNECTION then recovery of the database will be performed according to the sniffer policy
         * specified in parameter "policy" (one of the sniffer policies defined in the mco_sniffer_policy enum).
         * This policy specifies for which connections sniffer will call the user's callback function.
         */
        MCO_RET mco_db_sniffer(mco_db_h db, mco_sniffer_callback_t callback, mco_sniffer_policy policy);
        MCO_RET mco_db_sniffer_ex(mco_db_h db, mco_sniffer_callback_ex_t callback, void *user_context, mco_sniffer_policy policy);

        /* Transaction management */
        MCO_RET mco_trans_start(mco_db_h db, MCO_TRANS_TYPE type, MCO_TRANS_PRIORITY pri,  /*OUT*/mco_trans_h* p_trans);
        MCO_RET mco_trans_start_ex(mco_db_h db, MCO_TRANS_TYPE type, MCO_TRANS_PRIORITY pri, MCO_TRANS_ISOLATION_LEVEL level, /*OUT*/mco_trans_h* p_trans);

        MCO_RET mco_trans_commit(mco_trans_h t);
        MCO_RET mco_trans_commit_phase1(mco_trans_h t);
        MCO_RET mco_trans_commit_phase2(mco_trans_h t);

        MCO_RET mco_trans_rollback(mco_trans_h t);
        MCO_RET mco_trans_upgrade(mco_trans_h t);

		typedef enum
		{
			MCO_TRANS_PREPREPARE,
			MCO_TRANS_POSTPREPARE,
			MCO_TRANS_PRECOMMIT,
			MCO_TRANS_POSTCOMMIT,
			MCO_TRANS_PRECOMMIT_PREPARED,
			MCO_TRANS_POSTCOMMIT_PREPARED,
			MCO_TRANS_ROLLBACK
		} mco_trans_state;

		typedef MCO_RET (*mco_trans_callback_t)(mco_trans_h trans, mco_trans_state state, void* arg);
		void mco_set_trans_callback(mco_db_h db, mco_trans_callback_t callback, void* arg);

#ifdef MCO_CFG_DEBUG_TRANSACTIONS
        MCO_RET mco_trans_start_debug(mco_db_h db, MCO_TRANS_TYPE type, MCO_TRANS_PRIORITY pri,  /*OUT*/mco_trans_h* p_trans, char const* file, char const* func, int line);
        MCO_RET mco_trans_start_ex_debug(mco_db_h db, MCO_TRANS_TYPE type, MCO_TRANS_PRIORITY pri, MCO_TRANS_ISOLATION_LEVEL level, /*OUT*/mco_trans_h* p_trans, char const* file, char const* func, int line);
        MCO_RET mco_trans_upgrade_debug(mco_trans_h t, char const* file, char const* func, int line);
        MCO_RET mco_trans_commit_debug(mco_trans_h t, char const* file, char const* func, int line);
        MCO_RET mco_trans_commit_phase1_debug(mco_trans_h t, char const* file, char const* func, int line);
        MCO_RET mco_trans_commit_phase2_debug(mco_trans_h t, char const* file, char const* func, int line);
        MCO_RET mco_trans_rollback_debug(mco_trans_h t, char const* file, char const* func, int line);

        void mco_trans_debug_skip_wrappers(int n);
			void mco_trans_debug_dump(mco_db_h db, mco_bool all);

        typedef enum {
            MCO_TRANS_DEBUG_DISABLED,
            MCO_TRANS_DEBUG_TRACE,
            MCO_TRANS_DEBUG_CHECK,
            MCO_TRANS_DEBUG_STORE
        } MCO_TRANS_DEBUG_LEVEL;

        void mco_trans_debug_level(MCO_TRANS_DEBUG_LEVEL level);


        #ifndef __FUNCTION__
        #define __FUNCTION__ ""
        #endif

        #define mco_trans_start(db, type, pri,  p_trans) mco_trans_start_debug((db),(type),(pri),(p_trans),__FILE__,__FUNCTION__,__LINE__)
        #define mco_trans_start_ex(db, type, pri, level, p_trans) mco_trans_start_ex_debug((db),(type),(pri),(level),(p_trans),__FILE__,__FUNCTION__,__LINE__)
        #define mco_trans_upgrade(t) mco_trans_upgrade_debug((t),__FILE__,__FUNCTION__,__LINE__)
        #define mco_trans_commit(t) mco_trans_commit_debug((t),__FILE__,__FUNCTION__,__LINE__)
        #define mco_trans_commit_phase1(t) mco_trans_commit_phase1_debug((t),__FILE__,__FUNCTION__,__LINE__)
        #define mco_trans_commit_phase2(t) mco_trans_commit_phase2_debug((t),__FILE__,__FUNCTION__,__LINE__)
        #define mco_trans_rollback(t) mco_trans_rollback_debug((t),__FILE__,__FUNCTION__,__LINE__)
#endif

        MCO_RET mco_trans_checkpoint(mco_trans_h t);
        MCO_RET mco_trans_type(mco_trans_h t,  /*OUT*/MCO_TRANS_TYPE* type);
        MCO_RET mco_trans_no(mco_trans_h t, mco_trans_counter_t* no);
        MCO_RET mco_trans_isolation_level(mco_trans_h t, MCO_TRANS_ISOLATION_LEVEL* isolation_level);

        int     mco_trans_get_supported_isolation_levels(void);
        int     mco_trans_set_unused_versions_limit(mco_db_h db, int max_archived_versions);

        mco_trans_counter_t mco_trans_set_cleanup_period(mco_db_h db, mco_trans_counter_t cleanup_period, mco_bool per_connection_cleanup);
        void    mco_trans_optimistic_threshold(mco_db_h db, int max_conflicts_percent, int disable_period);
        MCO_TRANS_ISOLATION_LEVEL mco_trans_set_default_isolation_level(mco_db_h db, MCO_TRANS_ISOLATION_LEVEL level);

		typedef enum {
			MCO_GC_DISABLED,
			MCO_GC_SELF_VERSIONS,
			MCO_GC_ON_MODIFICATION,
			MCO_GC_ALWAYS
		} MCO_GC_POLICY;
	    MCO_GC_POLICY mco_trans_set_gc_policy(mco_db_h con, MCO_GC_POLICY policy);

        int mco_get_last_unique_violation_index(mco_db_h con);

        MCO_RET mco_get_last_error(mco_trans_h t);

        MCO_RET mco_get_last_autoid(mco_db_h con, uint8* autoid);

        typedef enum MCO_TRANS_OBJ_STATE_
        {
            MCO_TRANS_OBJ_CREATED     = 0x01,
            MCO_TRANS_OBJ_DELETED     = 0x02,
            MCO_TRANS_OBJ_ALL_DELETED = 0x04
        } MCO_TRANS_OBJ_STATE;

        typedef MCO_RET (*mco_trans_iterator_callback_t)(mco_trans_h trans, MCO_Hf* obj, int cid, int obj_state, void* user_ctx);

        MCO_RET mco_trans_iterate(mco_trans_h trans, mco_trans_iterator_callback_t callback, void* user_ctx);

		void mco_trans_iterable(mco_db_h con, mco_bool enable);

        /* Cursor operators */
        typedef enum MCO_OPCODE_E_
        {
            MCO_NOP = 0,  /* No operation */
            MCO_LT = 1,   /* Values less-than sample */
            MCO_LE = 2,   /* Values less-or-equal to the sample */
            MCO_EQ = 3,   /* Value equal to the sample */
            MCO_GE = 4,   /* Values greater-than-or-equal-to the sample */
            MCO_GT = 5,   /* Values greater-than the sample */
            MCO_OVERLAP = 6,  /* rtree, overlapping regions */
            MCO_CONTAIN = 7,  /* rtree, containing regions */
            MCO_EX = 8,       /* patricia, exact match */
            MCO_BEST = 9,     /* patricia, the best, longest match */
            MCO_PREF = 10,    /* patricia, prefix match */
            MCO_NEXT_MATCH = 11, /* patricia, continue search instead of new search */
            MCO_NEIGHBOURHOOD = 12, /* rtree, list object distances from the sample */
            MCO_NEIGHBORHOOD = MCO_NEIGHBOURHOOD, /* American synonym */
            MCO_BELONG = MCO_LE /* patricia, sample belongs to value */
        } MCO_OPCODE;

        typedef union {
            uint1 u1;
            uint2 u2;
            uint4 u4;
            int1 i1;
            int2 i2;
            int4 i4;
            uint2 ptr_size;
            char c[3];
            nchar_t nc[3];
            #ifdef MCO_CFG_WCHAR_SUPPORT
            wchar_t wc[3];
            #endif
            #ifdef MCO_TARGET_FLOAT_SUPPORTED
            float flt;
            double dbl;
            #endif
            mco_int8 i8;
            uint8 u8;
            char none[8];
        } mco_value_union_t;


        /* Operand for full text search */
        typedef struct {
            void const*       kwd_ptr;
            mco_value_union_t kwd_val;
            mco_cursor_t cursor;
        } mco_search_kwd_t;

        /* cursor navigation */
        MCO_RET mco_cursor_check(mco_trans_h t, mco_cursor_h c);
        MCO_RET mco_cursor_first(mco_trans_h t, mco_cursor_h c);
        MCO_RET mco_cursor_last(mco_trans_h t, mco_cursor_h c);
        MCO_RET mco_cursor_next(mco_trans_h t, mco_cursor_h c);
        MCO_RET mco_cursor_prev(mco_trans_h t, mco_cursor_h c);
        MCO_RET mco_cursor_close(mco_trans_h t, mco_cursor_h c);
        MCO_RET mco_cursor_store(mco_trans_h t, mco_cursor_h c);

        /* cursor types */
        typedef enum MCO_CURSOR_TYPE_E_
        {
            MCO_LIST_CURSOR = 0, MCO_TREE_CURSOR = 1, MCO_HASH_CURSOR = 2, MCO_KDTREE_CURSOR = 3, MCO_PTREE_CURSOR = 4,
            MCO_RTREE_CURSOR = 5
        } MCO_CURSOR_TYPE;

        /* get cursor information */
        MCO_RET mco_cursor_type(mco_cursor_h c,  /*OUT*/MCO_CURSOR_TYPE* type);
        MCO_RET mco_cursor_get_class_code(mco_cursor_h c,  /*OUT*/uint2* rescode);

        /* runtime statistics */
        typedef struct mco_class_stat_t_
        {
            mco_counter_t objects_num;    /* Total number of instances of the class */
            mco_counter32_t versions_num; /* Number of instances of the class including all versions used at the moment (for MVCC) */
            mco_counter32_t core_pages;   /* Number of pages used for all data except blobs */
            mco_counter32_t blob_pages;   /* Number of pages used by blobs */
            mco_counter32_t seq_pages;    /* Number of pages used by sequences */
            mco_counter_t core_space;     /* Total number of bytes, not counting blobs */
        } mco_class_stat_t, * mco_class_stat_h;

        /* get runtime statistics for a class */
        MCO_RET mco_class_stat_get(mco_trans_h t, uint2 class_code,  /*OUT*/mco_class_stat_h stat);

        /* define index characteristics */
        #define MCO_IDXST_TYPE_MEM         0      /* In-memory index */
        #define MCO_IDXST_TYPE_DISK        1      /* Persistent index */
        #define MCO_IDXST_TYPE_MASK        1

        #define MCO_IDXST_NATURE_BTREE     0      /* binary tree index */
        #define MCO_IDXST_NATURE_PTREE     2      /* Patricia tree index*/
        #define MCO_IDXST_NATURE_KDTREE    4      /* KD-tree index */
        #define MCO_IDXST_NATURE_HASH      6      /* hash index */
        #define MCO_IDXST_NATURE_META      8      /* meta-index */
        #define MCO_IDXST_NATURE_RTREE     10     /* R-tree index */
        #define MCO_IDXST_NATURE_MASK      14

        #define MCO_IDXST_FEATURE_UNIQUE   0x10   /* Unique index */
        #define MCO_IDXST_FEATURE_UDF      0x20   /* User-defined index */
        #define MCO_IDXST_FEATURE_THICK    0x40   /* Thick index */
        #define MCO_IDXST_FEATURE_OPTIMIZED 0x80   /* Uses optimized compare & hash functions */

        #define MCO_IDXST_FUNCTION_REGULAR 0      /* Regular class index */
        #define MCO_IDXST_FUNCTION_OID     0x100   /* Object-ID meta-index */
        #define MCO_IDXST_FUNCTION_AUTOOID 0x200   /* Auto object-ID meta-index */
        #define MCO_IDXST_FUNCTION_AUTOID  0x400   /* Auto id meta-index*/
        #define MCO_IDXST_FUNCTION_LIST    0x800   /* List meta-index */
        #define MCO_IDXST_FUNCTION_MASK    0xF00

        typedef struct mco_index_stat_tree_t_ {
            mco_counter_t levels_num;             /* tree height */
            mco_counter_t duplicates_num;         /* number of duplicates, not implemented yet */
        } mco_index_stat_tree_t;

        typedef struct mco_index_stat_kdtree_t_ {
            mco_counter_t levels_num;             /* tree height */
        } mco_index_stat_kdtree_t;

        typedef struct mco_index_stat_hash_t_ {
            mco_counter_t avg_chain_length;       /* average number of keys in a basket */
            mco_counter_t max_chain_length;       /* maximum number of keys in a basket */
            mco_counter_t table_size;             /* size of hash table */
            mco_counter_t chains_num;             /* number of chains */
        } mco_index_stat_hash_t;

        typedef struct mco_index_stat_rtree_t_ {
            mco_counter_t levels_num;             /* tree height */
            mco_counter_t duplicates_num;         /* number of duplicates, not implemented yet */
        } mco_index_stat_rtree_t;

        typedef struct mco_index_stat_meta_t_ {
            mco_counter_t disk_pages_num;         /* number of disk pages */
        } mco_index_stat_meta_t;

        typedef struct mco_index_stat_t_ {
            uint2         class_code;             /* class code to which the index belongs */
            uint4         type;                   /* type, MCO_IDXST_ flags */
            char        * plabel;                 /* the index label */
            mco_counter_t keys_num;               /* number of keys in the index */
            mco_counter_t pages_num;              /* number of pages used by the index */
            mco_counter_t avg_cmp;                /* average number of comparisons needed to locate an element */
            mco_counter_t max_cmp;                /* maximum number of comparisons needed to locate an element */
            union {
                mco_index_stat_tree_t   btree;     /* specific btree stats */
                mco_index_stat_kdtree_t kdtree;    /* specific kdtree stats */
                mco_index_stat_hash_t   hash;      /* specific hash stats */
                mco_index_stat_tree_t   ptree;     /* specific patricia tree stats */
                mco_index_stat_rtree_t  rtree;     /* specific r-tree stats */
                mco_index_stat_meta_t   meta;      /* specific meta-index stats */
            } spec;
        } mco_index_stat_t, * mco_index_stat_h;

        /* get the number of indexes in the database */
        MCO_RET mco_index_stat_num( mco_trans_h t, uint2 * pnidx);
        /* get statistics for an index */
        MCO_RET mco_index_stat_get( mco_trans_h t, uint2   index, mco_index_stat_t * stat );
        /* get index size (numerb of keys) */
        MCO_RET mco_index_get_size( mco_trans_h t, uint2   index, mco_counter_t* size );

        /* unrecoverable error handler */
        typedef void(*mco_error_handler_f)(MCO_RET errcode);
        void mco_error_set_handler(mco_error_handler_f f);

        /**
         * Depending on the configuration, extended information (file, line) may be available.
         * If both error handlers are defined, the extended version is called first.
         */
        typedef void(*mco_error_handler_f_ex)(MCO_RET errcode, const char* file, int line);
        void mco_error_set_handler_ex(mco_error_handler_f_ex f);


        /* returns error code description */
        const char *mco_strerror(MCO_RET rc);

        /* System heap memory manager */
        void* mco_sys_malloc(mco_size_t size);
        void *mco_sys_realloc(void *mem, mco_size_t size);
        void* mco_sys_calloc(mco_size_t n_elems, mco_size_t elem_size);
        void mco_sys_free(void* ptr);
        void* mco_sys_valloc(mco_size_t size);
        void mco_sys_vfree(void* ptr);

        MCOSALDLOAD_API void* mco_sys_dll_load(char const* library, char const* func, mco_bool lazy);
        MCOSALDLOAD_API char const* mco_sys_dll_error(void);

        /* save/restore */
        /* abstract write and read streams interfaces; stream handle is a pointer to implementation-specific data */
        typedef mco_size_sig_t(*mco_stream_write)(void* stream_handle, const void* from, mco_size_t nbytes);
        typedef mco_size_sig_t(*mco_stream_read)(void* stream_handle, /*OUT*/void* to, mco_size_t max_nbytes);

        /* save the database metadata in JSON format suitable for xsql utility */
        MCO_RET mco_db_save_metadata(void* stream_handle, mco_stream_write output_stream_writer, mco_db_h db, mco_bool save_defaults);

        /* save the database schema */
        MCO_RET mco_db_save_dictionary(void* stream_handle, mco_stream_write output_stream_writer, mco_db_h db);

		void mco_begin_wait_event(mco_db_h con, MCO_WAIT_EVENT event);
		void mco_end_wait_event(mco_db_h con);

        /* save the db content to the output stream */
        MCOSERI_API MCO_RET mco_db_save(void* stream_handle, mco_stream_write output_stream_writer, mco_db_h db);

        /* create a db instance and load the content from the input stream - instead of mco_db_open() */
        MCOSERI_API MCO_RET mco_db_load(void* stream_handle, mco_stream_read input_stream_reader, const char* db_name, mco_dictionary_h dict,
                        mco_device_t *devices, uint2 n_devices, mco_db_params_t * db_params );

        /* save the table content to the output stream */
        MCOSERI_API MCO_RET mco_class_save(uint2 class_code, void* stream_handle, mco_stream_write output_stream_writer, mco_db_h db);

        /* load the table content from the stream */
        MCOSERI_API MCO_RET mco_class_load(uint2 class_code, void* stream_handle, mco_stream_read input_stream_reader, mco_db_h db);

        /* save the db content to the output stream in JSON format */
        MCOJSER_API MCO_RET mco_db_json_export(mco_trans_h t, void *stream_handle, mco_stream_write output_stream_writer);

        /* event handling */
        /* synchronous events: */
        typedef enum MCO_EVENT_TYPE_E_
        {
            MCO_EVENT_NEW,         /* A new instance created */
            MCO_EVENT_UPDATE,      /* An existing instance was updated, applied to single field */
            MCO_EVENT_DELETE,      /* An existing instance was deleted */
            MCO_EVENT_DELETE_ALL,  /* Delete_all operation was called for a class */
            MCO_EVENT_CHECKPOINT,  /* Modifications applied to an instance were checked in */
            MCO_EVENT_CLASS_UPDATE /* An existing instance was updated, applied to any of the fields */
        } MCO_EVENT_TYPE;

        typedef enum MCO_HANDLING_ORDER_E_
        {
            MCO_BEFORE_UPDATE, MCO_AFTER_UPDATE
        } MCO_HANDLING_ORDER;

        /* asynchronous events: handlers in different threads are signaled after commit. */
        MCO_RET mco_async_event_wait(mco_db_h dbh, int event_id);
        MCO_RET mco_async_event_release(mco_db_h dbh, int event_id);
        MCO_RET mco_async_event_release_all(mco_db_h dbh);

        typedef struct mco_runtime_info_t_
        {
            uint1 mco_version_major; /* eXtremeDB version */
            uint1 mco_version_minor; /* eXtremeDB version */
            uint2 mco_build_number;  /* eXtremeDB build number */

            uint1 mco_size_t;       /* sizeof mco_size_t variable, depends on x32/x64 architecture */
            uint1 mco_offs_t;       /* sizeof mco_offs_t variable, depends on x32/x64 architecture */
            uint1 uint4_supported;  /* uint4 datatype support */
            uint1 float_supported;  /* double/float datatype support */
            uint1 mco_checklevel;   /* runtime's checklevel, 0 for non-debug libraries */
            uint1 evaluation_version;       /* Evaluation runtime */
            uint1 large_database_supported; /* Large database support (>4G) for x32 runtime mode */
            uint1 collation_supported;      /* External collations (index comparision) support */
            uint1 heap31_supported;         /* eXtremeDB v3.1 application level heap feature support */
            uint1 bin_serialization_supported; /* Binary or XML serialization support */
            uint1 fixedrec_supported;          /* FIXEDREC alloations for constant-size objects support */
            uint1 statistics_supported;        /* Statistic collection enabled */
            uint1 events_supported;            /* Synch/Asynch events support */
            uint1 save_load_supported;         /* Database image save/load support */
            uint1 object_initialization_supported; /* Support of new object fields initialization (instead of zeros) support */
            uint1 direct_index_field_access_supported;  /* Use values of indexed fields from the indexes instead of object bodies optimization (persistent objects) */
            uint1 multiprocess_access_supported;        /* conventional or shared memory driver lined into the program  */
            uint1 object_repack_supported;              /* Support of dynamic object repack feature. Obsolete. Left for compatibility */
            uint1 transaction_logging_supported;        /* Transaaction Logging (TL) feature support */
            uint1 cluster_supported;                    /* Cluster (CL) feature support */
            uint1 high_availability_supported;          /* High Avalaibility (HA) fature support */
            uint1 iot_supported;                        /* IoT feature support */
            uint1 ha_multicast_supported;               /* HA Multi-cast support */
            uint1 ha_incremental_replication_supported; /* HA incremental replication support */
            uint1 binary_schema_evalution_supported;    /* Binary schema evaluation support */
            uint1 unicode_supported;                    /* Unicode (nchar/nstring datatype) support */
            uint1 wchar_supported;                      /* WChar (wchar/wstring datatype) support. Required C/C++ runtime */
            uint1 recovery_supported;                   /* Recovery spoort. Obsolete, left for compatibility */
            uint1 disk_supported;                       /* Support for persistent objects (vtmem/vtdsk runtime) */
            uint1 direct_pointers_supported;            /* Direct- or offset-based pointers in the database (DPTR/OFFS runtime) */
            uint1 persistent_object_supported;          /* Object persistence support */
            uint1 xml_import_export_supported;          /* XML database export/import support */
            uint1 user_defined_index_supported;         /* User-defined index (comparision) support */
            uint1 multifile_supported;                  /* Multi-file persistent storage support */
            uint1 multifile_descriptor_supported;       /* Enable Multi-file descriptor file (mfd-file) support for multi-file persistent storage feature */
            uint1 two_phase_commit_supported;           /* Two-phase commit support */
            uint1 rtree_supported;                      /* Include rtree index implementation code into runtime */
            uint1 tree_based_hash;                      /* tree-based hash implementation (for internal debugging) */
            uint1 tmgr_mvcc_async_cleanup;              /*  */
            uint1 concurent_disk_btree;                 /* Always on, obsolete, left for compatibility */
            uint1 open_cursor_goto_first;               /* Initial positioning of new cursors to the first element if enabled */
            uint1 smart_index_insert;                   /* Pre-sort data records in a voluntaty index creation procedure */
            uint1 btree_leaf_lock;                      /* Fine-grain locking of btree indexes */
            uint1 null_statistics;                      /* Always on, obsolete, left for compatibility */
            uint1 implicit_runtime_start;               /* Always on, obsolete, left for compatibility */
            uint1 bufferized_sync_iostream;             /* Buffer transmission in HA synchronization procedure if enabled */
            uint1 async_replication;                    /* Asynchronus HA replication enabled */
            uint1 fast_transaction_list;                /* tree index in transaction list implementation enabled (MURSIW) */
            uint1 extendable_dirty_page_bitmap;         /* Persistent storage's dirty pages map extends */
            uint1 mursiw_policy;                        /* MURSIW TM behavior: 0 - Priority-based transaction queue, 1 - Sequential transaction queue, 2 - Global sync, 3 - RW-Lock */
            uint1 sync_capabilities;                    /* Sync. primitives implementation capabilities, bit mask: 1 - single-process, 2 - multi-process capable primitives */
            uint1 char_comparison_policy;               /* Compare chars as: 0 - Default, 1 - as signed, 2 - as unsigned */
            uint4 stream_buffer_size;                   /* Size of buffer used in streaming code in HA / CL / TL etc. (Also for Stream Reader and Stream Writer functions.) */
            uint1 max_db_instances;                     /* Max. number of database instances supported by the runtime */
            uint1 max_db_name_length;                   /* Max. length of a database name supported by the runtime */
            int   max_extends;                          /* Max. number of extends of database memory area */
            uint4 tl_page_buffer_size;                  /* Size of internal buffers that TL operates */
            uint2 ha_max_replicas;                      /* Max. number of HA replicas at a time */
            uint4 ha_transmit_buffer_size;              /* Size of buffer used in HA to send data from commit, default is 2K */
            uint4 ha_syncronization_buffer_size;        /* HA synchronization stream IO buffer size, default is 2K */
            uint4 default_redo_log_limit;               /* Default size of REDO log length limit, default is 16M */
            uint1 mvcc_critical_sections;               /* Number of critical sections used by MVCC, it is better to be a prime number */
            uint1 mvcc_per_index_locks;                 /* Number of locks used by index, it is better to be a prime number */
            uint2 con_disk_page_cache_size;             /* Size of per-connection page cache. The runtime pre-allocates some pages to speedup page allocation when needed */
            uint1 small_con_cache_threshold;            /* Use hashed per-connection cache if the con_disk_page_cache_size > small_con_cache_threshold; use an array otherwise */
            uint4 extendable_dirty_page_bitmap_limit;   /* Max bitmap size if extendable dirty page bitmap enabled */
            uint1 max_vista_sessions;                   /* Max number of Vista sessions (windows vista only) */
            uint1 concurrent_write_transactions;        /* Does current Transaction Manager support concurrent write transactions or not */
            uint1 encryption_support;                   /* Database encryption enabled */
            uint1 backup_support;                       /* Database backup enabled */
            const char *mco_revision;                   /* String eXtremeDB version number */

/* Parameter named for backward compatibility */
#define mco_shm_supported multiprocess_access_supported
#define mco_save_load_supported save_load_supported
#define mco_disk_supported disk_supported
#define mco_multithreaded sync_capabilities
#define mco_fixedrec_supported fixedrec_supported
#define mco_xml_supported xml_import_export_supported
#define mco_stat_supported statistics_supported
#define mco_events_supported events_supported
#define mco_versioning_supported evaluation_version
#define mco_evaluation_version evaluation_version
#define mco_recovery_supported recovery_supported
#define mco_rtree_supported rtree_supported
#define mco_unicode_supported unicode_supported
#define mco_wchar_supported wchar_supported
#define mco_rtl_supported wchar_supported
#define mco_sql_supported wchar_supported
#define mco_disk_supported disk_supported
#define mco_direct_pointers direct_pointers_supported

        } mco_runtime_info_t;


        /* get eXtremeDB runtime configuration info. */
        void mco_get_runtime_info( /*OUT*/mco_runtime_info_t* pinf);

        int mco_get_max_connections(mco_db_h db);

        /* global runtime options, specified per process */
        #define MCO_RT_OPTION_EXTHEAPS           0    /* Deprecated, left for compatibility, do not use */
        #define MCO_RT_WINDOWS_SHM_OPT           1
        #define MCO_RT_OPTION_MARK_LAST_OBJ      2
        #define MCO_RT_OPTION_UNIX_SHM_MASK      3
        #define MCO_RT_POSIX_SHM_OPT             4
        #define MCO_RT_CRC_ALGORITHM             5
        #define MCO_RT_MAX_PATRICIA_DEPTH        6
        #define MCO_RT_MAX_DYNAMIC_PIPES         7
        #define MCO_RT_OPTION_CLASS_LOAD_MERGE   8
        #define MCO_RT_OPTION_DATETIME_PRECISION 9    /* Time resolution (precision): 1 - seconds, 1000 - milliseconds, 1000000 - microseconds,... */
        #define MCO_RT_OPTION_LAST               10

        /* Posix shared memory options */
        #define MCO_RT_POSIX_SHM_ANONYMOUS    1
        #define MCO_RT_POSIX_SHM_SHARED       2
        #define MCO_RT_POSIX_SHM_HUGETLB      4

        /* Windows shared memory and sync. primitives naming and security descriptor options */
        #define MCO_RT_WINDOWS_SHM_PREFIX_GLOBAL        0      /* use Global prefix. Shared objects are visible in all sessions */
        #define MCO_RT_WINDOWS_SHM_PREFIX_LOCAL         1      /* use Local prefix, Shared object are visible in current session only */
        #define MCO_RT_WINDOWS_SHM_PREFIX_SESSIONS      2      /* Vista sessions fix */
        #define MCO_RT_WINDOWS_SHM_PREFIX_NONE          3      /* Default, no prefix */
        #define MCO_RT_WINDOWS_SHM_PREFIX_MASK          3

        #define MCO_RT_WINDOWS_SHM_SEC_DESCR_EMPTY      0      /* every one can access */
        #define MCO_RT_WINDOWS_SHM_SEC_DESCR_NULL       4      /* default, use windows default policy */
        #define MCO_RT_WINDOWS_SHM_SEC_DESCR_SAMEUSER   8      /* only the same user can access */
        #define MCO_RT_WINDOWS_SHM_SEC_DESCR_MASK       12
        /* * Default is MCO_RT_WINDOWS_SHM_PREFIX_NONE|MCO_RT_WINDOWS_SHM_SEC_DESCR_NULL combination to make database visible and accessible
             from only local session
           * Use MCO_RT_WINDOWS_SHM_PREFIX_GLOBAL|MCO_RT_WINDOWS_SHM_SEC_DESCR_EMPTY to make your database accessible from all sessions and
             for all users
           * Use MCO_RT_WINDOWS_SHM_PREFIX_GLOBAL|MCO_RT_WINDOWS_SHM_SEC_DESCR_SAMEUSER to make your database accessible from all sessions
             of the same user, ex. a service running under UserA and a desktop program running under UserA
           * Please note MCO_RT_WINDOWS_SHM_PREFIX_GLOBAL is usable for Services only, desktops sessions are able to create databases using
             MCO_RT_WINDOWS_SHM_PREFIX_LOCAL or MCO_RT_WINDOWS_SHM_PREFIX_NONE modes and to connect to databases opened with MCO_RT_WINDOWS_SHM_PREFIX_GLOBAL
             with matching security settings.
        */

        #define MCO_RT_CRC32_NONE     0 /* do not calculate CRC */
        #define MCO_RT_CRC32          1 /* implementation of CRC used prior release 15523 2014-07-17 */
        #define MCO_RT_CRC32_FAST     2 /* calculate CRC only for firld and last 8-byte word of data */
        #define MCO_RT_CRC32_OLD      3 /* implementation of CRC used prior release 12823 2013-01-28 */
        #define MCO_RT_CRC32C         4 /* CRC32C hardware&software implementations used after release 15523 2014-07-17 */

        void mco_runtime_setoption(int option, int value);
        int  mco_runtime_getoption(int option);
        int* mco_runtime_get_options_arr(void);

        enum mco_file_open_flags
        {
            MCO_FILE_OPEN_DEFAULT = 0,
            MCO_FILE_OPEN_READ_ONLY = 1,
            MCO_FILE_OPEN_TRUNCATE = 2,
            MCO_FILE_OPEN_NO_BUFFERING = 4,
            MCO_FILE_OPEN_EXISTING = 8,
            MCO_FILE_OPEN_TEMPORARY = 16,
            MCO_FILE_OPEN_FSYNC_FIX = 32,
            MCO_FILE_OPEN_SUBPARTITION = 64, /* use "offset" field of dev.raid structure */
            MCO_FILE_OPEN_FSYNC_AIO_BARRIER = 128, /* wait completion of all AIO requests and perform synchronous fsync */
            MCO_FILE_OPEN_COMPRESSED = 256, /* compressed file */
            MCO_FILE_OPEN_LOCK = 512, /* lock file to prevent concurrent access to the file by some other process */
            MCO_FILE_OPEN_NO_READ_BUFFERING = 1024, /* use fadvice to mark read page as not need for file system to prevent double buffering */
            MCO_FILE_OPEN_NO_WRITE_BUFFERING = 2048 /* use fadvice to mark thrown page as not need for file system to prevent double buffering */
        };


       /**
         * Set transaction log parameters:
         * A value of 0 means that corresponding parameter should not be changed.
         * Also depending on the log mode specified, some of parameters may be ignored.
         * Parameter "con" is a database connection and "params" are log parameters.
         * Returns an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_set_log_params(mco_db_h con, mco_log_params_t* params);

        /**
         * Repair disk database after failure.
         * If using the MVCC transaction manager, in the case of a crash, a disk database can contain
         * undeleted old versions and working copies.  Their presence will not break database consistency
         * and doesn't prevent the normal working of an application. However, detecting such objects
         * requires a complete scan of the database.  This is why automatic recovery doesn't perform such
         * a repair automatically.
         * It can performed explicitly by calling this function, or by setting mvcc_auto_repair to MCO_YES
         * in the mco_disk_open_params_t when calling mco_disk_open_ex().
         *
         * Note that mco_disk_database_vacuum requires exclusive access to the database - no other
         * operations can be performed on the database until completion of this function.
         */
        MCO_RET mco_disk_database_vacuum(mco_db_h con);

        /**
         * Save the disk cache to the specified file
         * Parameter "con" is the database connection, "file_path" is the path to the file
         * where the cache will be saved.
         * Returns an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_save_cache(mco_db_h con, char const* file_path);

        /**
         * Online backup of disk database: save database and log files to the specified location without blocking database
         * @param con database connection
         * @param db_backup_path path to the created database backup file
         * @param log_backup_path path to the created database log file
         * @return an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_backup(mco_db_h con, char const* db_backup_path, char const* log_backup_path);

        /**
         * Loads database file into the file system cache
         * @param con database connection
         * @return an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_fs_cache_warmup(mco_db_h con);

        /**
         * Reset a per-connection cache of pages.  This cache is automatically reset
         * when a transaction is committed and in case of a disconnect.
         * But if there are many active connections with long transactions, a
         * large number of pinned pages can cause the page pool space to be exhausted.
         * In such cases caching can be explicitly reset or completely prohibited using
         * the mco_disk_enable_connection_cache function
         * Parameter "con" is the database connection.
         * Returns an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_reset_connection_cache(mco_db_h con);

        /**
         * Enable or disable the per-connection cache of pages.  This cache is automatically reset
         * when a transaction is committed and in case of a disconnect.
         * But if there are many active connections with long live transaction
         * large number of pinned pages can cause the page pool space to be exhausted.
         * In such cases caching can be prohibited using this function or the cache
         * should be periodically reset using mco_disk_reset_connection_cache function.
         * Parameter "con" is the database connection. Parameter "enabled" is set to MCO_YES to enable
         * caching, MCO_NO to disable caching.
         * Returns the previous status of per-connection caching.
         */
        mco_bool mco_disk_enable_connection_cache(mco_db_h con, mco_bool enabled);

        /**
         * Change the transaction commit policy for the specified database connection.
         * Parameter "con" is the database connection.  Parameter "policy" is set to MCO_COMMIT_NO_SYNC,
         * MCO_COMMIT_SYNC_FLUSH,  MCO_COMMIT_BUFFERED or MCO_COMMIT_DELAYED.
         * Returns the previous commit policy.
         */
        MCO_COMMIT_POLICY mco_disk_transaction_policy(mco_db_h con, MCO_COMMIT_POLICY policy);

        typedef struct mco_disk_cache_info_t_ {
          mco_counter_t connection_cache_hits; /* The number of connection cache hits since the database was opened */
          mco_counter_t cache_hits; /* The number of disk manager cache hits since the database was opened */
          mco_counter_t cache_misses; /* The number of disk manager cache misses */
          mco_counter_t allocated_pages; /* The disk manager cache size in pages */
          mco_counter_t used_pages; /* The number of used pages in the disk cache */
            mco_counter32_t  pinned_pages; /* Number of pinned pages.  Pinned pages can't be swapped out (Must be 32 bit to be able to use atomic  */
            mco_counter_t modified_pages; /* Number of pages in the cache modified by active transactions (as opposed to dirty pages, below).
                                             The number of modified pages is always less than or equal to the number of dirty pages. */
            mco_counter_t dirty_pages; /* Number of dirty pages In the MCO_DELAYED_COMMIT mode, it is all the pages that have not been committed to the database yet. */
            mco_counter_t copied_pages; /* Number of copies of original pages (REDO_LOG+MCO_COMMIT_DELAYED).
                                           The number of extra pages allocated from the page pool in support of the MCO_COMMIT_DELAYED mode.
                                           In order to roll back a transaction in this mode, the database runtime saves images of the modified pages
                                           prior to the transaction start. These copies are kept in the page pool,
                                           thus extra pages are used. When the transaction is committed all these copies are removed. */
            mco_counter_t write_delayed_pages; /* Number of write delayed pages */
            mco_counter_t subsequent_reads;    /* Number of reads of sequentially located paged */
        } mco_disk_cache_info_t;


        const mco_disk_cache_info_t* mco_disk_get_cache_info(mco_db_h con);
		MCO_RET mco_disk_cache_usage(mco_db_h db, mco_size_t* classes);

		#define MCO_DISK_FRAG_PROFILE_SIZE 16

		typedef struct {
			mco_size_t allocated;
			mco_size_t used;
			mco_counter_t profile[MCO_DISK_FRAG_PROFILE_SIZE];
		} mco_disk_alloc_info_t;


		MCO_RET mco_disk_get_alloc_info(mco_db_h con, mco_disk_alloc_info_t* unaligned, mco_disk_alloc_info_t* aligned);

		MCO_RET mco_disk_get_allocated_space(mco_db_h db, mco_offs_t* allocated, mco_offs_t* used);

		/**
         * Check that there are no pinned pages in disk cache.
         * Parameter "con" is the database connection.
         * Returns an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_check_cache(mco_db_h con);

        #ifdef MCO_DEBUG_DISK_CACHE
            #define MCO_DISK_CHECK_CACHE(con) mco_disk_check_cache(con)
        #else
            #define MCO_DISK_CHECK_CACHE(con)
        #endif

        typedef struct mco_disk_info
        {
            mco_offs_t data_file_size;     /* Total size of database storage */
            mco_offs_t log_file_size;      /* Total size of database log */
            mco_offs_t used_database_size; /* Size of used part of database storage */
        } mco_disk_info_t;

        /**
         * Get information about disk usage.
         * Parameter "con" is the database connection, "info" is a handle for returned information.
         * Returns an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_info(mco_db_h con, mco_disk_info_t* info);

        /**
         * Flush to the disk all changes made by committed transactions.
         * This method can be used to flush changes when the MCO_COMMIT_BUFFERED transaction policy is used.
         * Parameter "con" is the database connection.
         * Returns an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_flush(mco_db_h con);

        /**
         * When using two phase commit, this function should be called before opening the database
         * after a system or application crash to confirm a possible pre-committed transaction.
         * Parameter "log_file_path" is the path to the transaction log file, "log_type" is the type of
         * log: either REDO_LOG or UNDO_LOG.
         * Returns an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_confirm_transaction(char const* log_file_path, MCO_LOG_TYPE log_type);

        /**
         * When using two phase commit, this function should be called before opening the database
         * after a system or application crash to reject a possible pre-committed transaction.
         * Parameter "log_file_path" is the path to the transaction log file, "log_type" is the type of
         * log: either REDO_LOG or UNDO_LOG.
         * Returns an error code or MCO_S_OK.
         */
        MCO_RET mco_disk_reject_transaction(char const* log_file_path, MCO_LOG_TYPE log_type);

      /*
       */
        MCO_RET mco_disk_load_cache(mco_db_h con, char const* file_path);
      /*
       */
        MCO_RET mco_disk_save_cache(mco_db_h con, char const* file_path);

        /* Filesystem wrapper interface */
        /**
         * Structure representing abstract file interface
         */
        typedef struct mco_file
        {
            /**
             * Non zero if file supports concurrent access by multiple threads (pread/pwrite operations)
             */
            mco_bool is_reentrant;

            /**
             * Write data to the specified position in file. This function must be reentrant if "is_reentrant" is true.
             * Parameter "self" is a pointer to this structure, "offs" are offsets (positions) within the file,
             * "buf" is a buffer with data to be written, "size" is the number of bytes to be written.
             * Returns the number of bytes actually written or -1 in case of error.
             */
            mco_size_sig_t(*pwrite)(struct mco_file* self, mco_offs_t offs, void* buf, mco_size_t size);

            /**
             * Read data from the specified position in file. This function must be reentrant if "is_reentrant" is true.
             * Parameter "self" is a pointer to this structure, "offs" are offsets (positions) within the file,
             * "buf" is a buffer to receive the data, "size" is the number of bytes to be read.
             * Returns the number of bytes actually read or -1 in case of error.
             */
            mco_size_sig_t(*pread)(struct mco_file* self, mco_offs_t offs, void* buf, mco_size_t size);

            /**
             * Write data to the current position in file.
             * Parameter "self" is a pointer to this structure, "buf" is a buffer with data to be written,
             * "size" is the number of bytes to be written.
             * Returns the number of bytes actually written or -1 in case of error.
             */
            mco_size_sig_t(*write)(struct mco_file* self, void* buf, mco_size_t size);

            /**
             * Set current position in the file.
             * Parameter "self" is a pointer to this structure, "pos" is the new current position in the file
             * used by subsequent read/write operations.
             * Returns 0 if successful or -1 in case of error.
             */
            int(*seek)(struct mco_file* self, mco_offs_t pos);

            /**
             * Truncate file to the specified size.
             * Parameter "self" is a pointer to this structure, "size" is the new size of the file.
             * Returns 0 if successful or -1 in case of error.
             */
            int(*truncate)(struct mco_file* self, mco_offs_t size);

            /**
             * Read data from the current position in file.
             * Parameter "self" is a pointer to this structure, "buf" is a buffer to receive the data,
             * "size" is the number of bytes to be read.
             * Returns the number of bytes actually read or -1 in case of error.
             */
            mco_size_sig_t(*read)(struct mco_file* self, void* buf, mco_size_t size);

            /**
             * Read raw data (do not perform decompression and encryption of data)
			 * from the specified position in file.
			 * This function must be reentrant if "is_reentrant" is true.
             * Parameter "self" is a pointer to this structure, "offs" are offsets (positions) within the file,
             * "buf" is a buffer to receive the data, "size" is the number of bytes to be read.
             * Returns the number of bytes actually read or -1 in case of error.
             */
            mco_size_sig_t(*rawread)(struct mco_file* self, mco_offs_t offs, void* buf, mco_size_t size);

            /**
             * Flush data from OS file system buffers to the disk.
             * Parameter "self" is a pointer to this structure.
             * Returns 0 if successful or -1 in case of error.
             */
            int(*flush)(struct mco_file* self);

            /**
             * Get the size of the file.
             * Parameter "self" is a pointer to this structure, "length" is the address of a
             * variable to receive the file size.
             * Returns 0 if successful or -1 in case of error.
             */
            int(*size)(struct mco_file* self, mco_offs_t* length);

            /**
             * Close the file.
             * Parameter "self" is a pointer to this structure.
             * Returns 0 if successful or -1 in case of error.
             */
            int(*close)(struct mco_file* self);

            /**
             * Provide hint to file system whether this region will be used in near future or not
             * Parameter "self" is a pointer to this structure, "pos" is position of region within the file,
             * "size" is a size of region, will_need hint to file system cache
             * Returns 0 if successful or -1 in case of error.
             */
            int(*advise)(struct mco_file* self, mco_offs_t pos, mco_size_t size, mco_bool will_need);
        } mco_file_t, * mco_file_h;

        extern mco_file_h mco_open_file(char const* file_path, int flags, mco_db_params_t* db_params);

        extern mco_file_h mco_aio_open_file(char const* file_path, int flags, mco_file_h impl, mco_db_params_t* db_params);

		extern mco_file_h mco_aio_get_file(mco_file_h aio_file);

		extern void mco_aio_set_file(mco_file_h aio_file, mco_file_h impl_file);

		extern void mco_aio_initialize(void);

        /* asynchronous IO API */
        typedef enum {
            MCO_AIO_PWRITE,
            MCO_AIO_WRITE,
            MCO_AIO_SEEK,
            MCO_AIO_FLUSH,
            MCO_AIO_TRUNCATE,
            MCO_AIO_PREFETCH,
			MCO_AIO_CLOSE
        } mco_aio_op_t;

        typedef enum {
            MCO_AIO_ACT_ABORT,   /* call mco_stop */
            MCO_AIO_ACT_RETRY,   /* retry system call */
            MCO_AIO_ACT_IGNORE,  /* ignore error and continue work */
            MCO_AIO_ACT_SHUTDOWN /* reject all subsequent write request: them will return error */
        } mco_aio_action_t;

        typedef mco_aio_action_t (*mco_aio_error_handler_f)(mco_aio_op_t op, int err, mco_offs_t offs, mco_size_t size);
		/*
		 * Start AIO with specified number of workers and queue length limit.
		 * If n_workers is 0, then separate worker will be created for each file
		 */
		void mco_aio_start(mco_size_t max_queue_length, mco_size_t n_workers);
        void mco_aio_stop(void);
        mco_bool mco_aio_wait(void);
        mco_aio_error_handler_f mco_aio_register_error_handler(mco_aio_error_handler_f handler);
        mco_aio_action_t mco_aio_default_error_handler(mco_aio_op_t op, int err, mco_offs_t offs, mco_size_t size);

        /* direct structs helpers */
        #define  structsz(n)    ((unsigned int)sizeof(n))
        #define  fieldsz(s,f)   ((unsigned int)sizeof(((s*)0)->f))
        #define  fieldoffs(s,f) ((mco_size_t)(&((s*)0)->f))
        #define  alignsz(s,n) (((s)+(n)-1)&(~(n-1)))

        /* Enum helpers */
        #define _read_enum_(NUM,esz,etype,val,of1,of2) do{\
             uint##NUM tval = (uint##NUM)val;\
             res =mco_w_b##NUM##_get2((mco_objhandle_h) handle,of1,of2,&tval);\
             val = (etype)tval;\
        }while(0)

        #define _read_enum(esz,etype,val,of1,of2) do{\
            switch(esz)\
            {\
            case 1:\
                _read_enum_(1,esz,etype,val,of1,of2);\
                break;\
            case 2:\
                _read_enum_(2,esz,etype,val,of1,of2);\
                break;\
            case 4:\
                _read_enum_(4,esz,etype,val,of1,of2);\
                break;\
            default:\
                return MCO_E_UNSUPPORTED;\
                break;\
            }\
        }while (0)

        #define _write_enum_(NUM,esz,val,of1,of2,iaw) do{\
            uint##NUM tval = (uint##NUM)val;\
            res =mco_w_b##NUM##_put2((mco_objhandle_h) handle,of1,of2,iaw,tval);\
        }while(0)

        #define _write_enum(esz,val,of1,of2,iaw) do{\
            switch(esz)\
            {\
            case 1:\
            _write_enum_(1,esz,val,of1,of2,iaw);\
            break;\
            case 2:\
            _write_enum_(2,esz,val,of1,of2,iaw);\
            break;\
            case 4:\
            _write_enum_(4,esz,val,of1,of2,iaw);\
            break;\
            default:\
            return MCO_E_UNSUPPORTED;\
            break;\
            }\
        }while (0)


        #define _write_enum_ev_(NUM,esz,val,of1,of2,iaw,evt) do{\
            uint##NUM tval = (uint##NUM)val;\
            res =mco_w_b##NUM##_put_ev2((mco_objhandle_h) handle,of1,of2,iaw,tval,evt);\
        }while(0)

        #define _write_enum_ev(esz,val,of1,of2,iaw,evt) do{\
            switch(esz)\
            {\
            case 1:\
            _write_enum_ev_(1,esz,val,of1,of2,iaw,evt);\
            break;\
            case 2:\
            _write_enum_ev_(2,esz,val,of1,of2,iaw,evt);\
            break;\
            case 4:\
            _write_enum_ev_(4,esz,val,of1,of2,iaw,evt);\
            break;\
            default:\
                return MCO_E_UNSUPPORTED;\
            break;\
            }\
        }while (0)

        #define _set_enum(pext,NUM,fname) do{\
        pext->field_type =( NUM >= 4)?3:NUM; \
        if (pext->field_type == 3) pext->v.u4 = fname; \
        else if (pext->field_type == 2) pext->v.u2 = fname; \
        else pext->v.u1 = fname; \
        }while(0)

        mco_bool mco_db_recovery_required(char const* log_file_path);

        typedef enum mco_alloc_callback_threshold_kind_t { /* report when ... */
            MCO_ALLOC_PERCENT_USED, /* specified percent value of the database size will be reached */
            MCO_ALLOC_FREE_LEFT,    /* specified percent value of the database free space will be reached */
            MCO_ALLOC_DELTA_USED,   /* specified amount of memory will be allocated at once */
            MCO_ALLOC_USED          /* database memory usage will reach treshold */
        } mco_alloc_callback_threshold_kind_t;

        typedef MCO_RET (*mco_alloc_callback_t)(mco_db_h db, mco_offs_t used); /* used - Current memory usage in bytes */

        void mco_register_alloc_callback(mco_db_h db,
                                         mco_alloc_callback_t callback,
                                         mco_alloc_callback_threshold_kind_t threshold_kind,
                                         mco_offs_t threshold_value);

        void mco_register_disk_alloc_callback(mco_db_h db,
                                              mco_alloc_callback_t callback,
                                              mco_alloc_callback_threshold_kind_t threshold_kind,
                                              mco_offs_t threshold_value);


        MCO_RET mco_copy_handle(mco_db_h db, MCO_Hf* dst, MCO_Hf const* src);
        MCO_RET mco_assign_handle(MCO_Hf* dst, MCO_Hf const* src);
        MCO_RET mco_is_object_deleted( MCO_Hf* object_handle );

        MCO_RET mco_db_schema_evolution(mco_db_h db, struct mco_ddl_dictionary_t_* new_ddl_dict, int altered_class_no, int dropped_index_no);

		typedef enum {
			MCO_BACKUP_TYPE_AUTO,
			MCO_BACKUP_TYPE_SNAPSHOT,
			MCO_BACKUP_TYPE_INCREMENTAL
		} mco_backup_type;

		typedef enum {
			MCO_BACKUP_FLAG_COMPRESSED = 1,
			MCO_BACKUP_FLAG_ENCRYPTED  = 2
		} mco_backup_flags;

		#define MCO_MAX_BACKUP_DBNAME_SIZE 16
		#define MCO_MAX_BACKUP_LABEL_SIZE  16
		#define MCO_MAX_BACKUP_CIPHER_SIZE 256

    typedef struct mco_backup_info_t {
		  uint4 magic; /* magic number to identify backup record header */
      uint4 protocol_version; /* backup protocol identification */
      uint4 type; /* type of the backup record: snapshot or incremental */
      uint4 flags; /* backup flags */
      uint4 backup_no; /* sequential number */
			uint8 timestamp; /* backup timestamp */
			uint8 size;   /* size of backup segment including header (mco_backup_info_t) */
			uint8 offset; /* offset in file */
			uint4 crc; /* backup record checksum */
      mco_trans_counter_t trans_no; /* last transaction number */
      uint4 mem_page_size; /* size of memory page of the database */
      uint4 disk_page_size; /* size of persistent storage page of the database */
      mco_counter_t n_pages_total; /* total number of memory pages */
      char db_name[MCO_MAX_BACKUP_DBNAME_SIZE]; /* name of the database */
      char label[MCO_MAX_BACKUP_LABEL_SIZE]; /* backup record label */
    } mco_backup_info_t;

        #define MCO_BACKUP_PHASE_1 1
        #define MCO_BACKUP_PHASE_2 2
        typedef MCO_RET (*mco_backup_info_proc_t)   ( uint4 phase, const mco_backup_info_t * info, void * param );
        typedef MCO_RET (*mco_backup_chunk_proc_t)  ( uint4 chunk_id, const char * mem, uint4 mem_sz, void * param );
        typedef MCO_RET (*mco_restore_chunk_proc_t) ( uint4* chunk_id, char ** mem, uint4 * mem_sz, void * param );

        MCO_RET mco_backup (
                    mco_db_h connection,
                    char const* label,                    /* backup label */
                    mco_backup_type backup_type,             /* MCO_BACKUP_TYPE_XXXX */
                    mco_backup_info_proc_t proc_info,     /* callback routine for backup information header */
                    mco_backup_chunk_proc_t proc_chunk,   /* callback routine for data chunk */
                    void * proc_arg                       /* argument to be passed into the callback routines */
        );

        MCO_RET mco_restore (
                    mco_db_h connection,
                    const mco_backup_info_t * info,        /* backup information header */
                    mco_restore_chunk_proc_t proc_chunk,   /* callback routine for data chunk */
                    void * proc_arg                        /* argument to be passed into the callback routines */
        );
        MCO_RET mco_backup_incremental_processing_get( mco_db_h connection, mco_bool * enabled );
        MCO_RET mco_backup_incremental_processing_set( mco_db_h connection, mco_bool enable );

		MCOBACKUP_API MCO_RET mco_backup_info (uint4 phase, mco_backup_info_t const* info, void* param);
		MCOBACKUP_API MCO_RET mco_backup_chunk(uint4 chunk_id, char const* chunk, uint4 chunk_size, void* param);
		MCOBACKUP_API MCO_RET mco_restore_chunk(uint4 * chunk_id, char** pchunk, uint4* pchunk_size, void * param);

		MCOBACKUP_API MCO_RET mco_backup_create(mco_db_h con, char const* file_name, char const* label, mco_backup_type type, int compression_level, char const* cipher, char* err_buf, unsigned int * err_buf_sz);
		MCOBACKUP_API MCO_RET mco_backup_restore(mco_db_h con, char const* file_name, char const* label, char const* cipher, char* err_buf, unsigned int * err_buf_sz);
		MCOBACKUP_API MCO_RET mco_backup_verify(mco_db_h con, char const* file_name, char const* label, char const* cipher, char* err_buf, unsigned int * err_buf_sz);
		MCOBACKUP_API MCO_RET mco_backup_list(mco_db_h con, char const* file, mco_size_t* backup_list_size, mco_backup_info_t** backup_list, char* err_buf, unsigned int * err_buf_sz);

        MCOBACKUP_API void mco_backup_dummy(void);

	typedef enum   tag_mco_backup_progress_info_phase_ {
		MCO_BACKUP_PROGRESS_IDLE,       /* no backup activity */
		MCO_BACKUP_PROGRESS_LOOPING,    /* making loops over the database collecting the changes */
		MCO_BACKUP_PROGRESS_FINALIZING, /* last loop, finalizing the backup record */
		MCO_RESTORE_PROGRESS_LOOPING,   /* reading backup record chunks and applying them to the database */
	} mco_backup_progress_info_phase_t;

	typedef struct tag_mco_backup_progress_info_ {
		mco_backup_progress_info_phase_t phase;
		mco_size_t   max_passes;
		mco_size_t   max_steps;
		mco_size_t   pass_no;
		mco_size_t   step_no;	
	} mco_backup_progress_info_t;

	MCOBACKUP_API MCO_RET mco_backup_progress_info( mco_db_h connection, mco_backup_progress_info_t * pinfo );

	typedef struct tag_mco_async_backup_progress_info_ {
		mco_backup_progress_info_t core_info;
		char const *               file_name;
		char const *               last_label;
		mco_backup_type            type;
		int                        compression_level;
		MCO_RET                    last_error_code;
		char const *               last_error_msg;
	} mco_async_backup_progress_info_t;

	/* Starts async. backup task using a connection provided. Creates it's own connection to the database with no context (see sniffer) */
	MCOBACKUP_API MCO_RET mco_async_backup_start         ( mco_db_h connection, unsigned int backup_cycle_delay_msec,
                                                               char const* file_name, char const* label, mco_backup_type type, int compression_level, char const* cipher);
	MCOBACKUP_API MCO_RET mco_async_backup_stop          ( mco_db_h connection, int force );
	MCOBACKUP_API MCO_RET mco_async_backup_progress_info ( mco_db_h connection, mco_async_backup_progress_info_t * info );

	MCOBACKUP_API MCO_RET mco_async_restore_start         ( mco_db_h connection, char const* file_name, char const* label, char const* cipher);
	MCOBACKUP_API MCO_RET mco_async_restore_stop          ( mco_db_h connection, int force );

 	    /*
		 * Encryption functions
		 */

        /* Encryption algorithm */
		#define MCO_CFG_NO_ENCRYPTION      0
		#define MCO_CFG_RC4_OLD_ENCRYPTION 1 /* RC4 encryption without page offset as salt: for backward compatibility only */
		#define MCO_CFG_RC4_ENCRYPTION     2
		#define MCO_CFG_AES_ENCRYPTION     3

		#ifdef MCO_CFG_MINIMAL
			#define MCO_CFG_CRYPT_CTX_SIZE   (8)
			#define MCO_CFG_CRYPT_STATE_SIZE (8)
		#else
			#define MCO_CFG_CRYPT_CTX_SIZE   (64*4*2 + 4*2)
			#define MCO_CFG_CRYPT_STATE_SIZE (256 + 4*2)
		#endif

		typedef struct {
			uint1 data[MCO_CFG_CRYPT_CTX_SIZE];
		} mco_crypt_ctx_t;

		typedef struct {
			uint1 data[MCO_CFG_CRYPT_STATE_SIZE];
		} mco_crypt_state_t;

        MCOCRYPT_API uint1 mco_crypt_algorithm(void);
		MCOCRYPT_API void mco_crypt_init_ctx(mco_crypt_ctx_t* crypt_ctx, const char *cipher_key);
		MCOCRYPT_API void mco_crypt_block(mco_crypt_ctx_t const* crypt_ctx, uint1* block, uint4 block_size, uint8 block_offs);
		MCOCRYPT_API void mco_crypt_init_state(mco_crypt_ctx_t const* crypt_ctx, mco_crypt_state_t* state);
		MCOCRYPT_API void mco_crypt_stream(mco_crypt_ctx_t const* crypt_ctx, mco_crypt_state_t* state, uint1* buf, uint4 size);


        /*
            *   Tracing interface
            */

        typedef enum {
            MCO_TRACE_FATAL = 0,
            MCO_TRACE_ERROR, 
            MCO_TRACE_WARNING, 
            MCO_TRACE_NOTICE, 
            MCO_TRACE_INFO, 
            MCO_TRACE_DEBUG,
            MCO_TRACE_VERBOSE 
        } mco_severity;

        typedef enum {
            MCO_TRACE_BSE,
            MCO_TRACE_LAYOUT,
            MCO_TRACE_ORDER,
            MCO_TRACE_DISK,
            MCO_TRACE_TMGR,
            MCO_TRACE_CONN,
            MCO_TRACE_DBINST,
            MCO_TRACE_ALLOCATOR,
            MCO_TRACE_INDEX,
            MCO_TRACE_BTREE,
            MCO_TRACE_KDTREE,
            MCO_TRACE_PTREE,
            MCO_TRACE_RTREE,
            MCO_TRACE_TRIGRAM,
            MCO_TRACE_HASH,
            MCO_TRACE_PATTERN,
            MCO_TRACE_CRYPT,
            MCO_TRACE_ZIP,
            MCO_TRACE_CURSOR,
            MCO_TRACE_WRAPPER,
            MCO_TRACE_XML,
            MCO_TRACE_BLOB,
            MCO_TRACE_DDL,
            MCO_TRACE_SQL,
            MCO_TRACE_RSQL,
            MCO_TRACE_TL,
            MCO_TRACE_SEQ,
            MCO_TRACE_FILE,
            MCO_TRACE_SYNC,
            MCO_TRACE_BACKUP,
            MCO_TRACE_HA,
            MCO_TRACE_CLUSTER,
            MCO_TRACE_ODBC,
            MCO_TRACE_PYTHON,
            MCO_TRACE_LUA,
            MCO_TRACE_JNI,
            MCO_TRACE_SAL,
            MCO_TRACE_NETWORK,
            MCO_TRACE_SHMEM,
            MCO_TRACE_TRACE,     
            MCO_TRACE_PERF,
            MCO_TRACE_UDA,
            MCO_TRACE_XSQL,
            MCO_TRACE_IOT,
            MCO_TRACE_TRANSCHECK,
            MCO_TRACE_TRANSLOG,
            MCO_TRACE_JSER
        } mco_trace_module;

#define MCO_TRACE_MASK_ALL             0xFFFFFFFFFFFFFFFF

        /* Trace handler function type */
        typedef int (*mco_trace_message_handler_fn)(mco_severity severity, mco_trace_module module, const char *file, int line, const char *fmt, ...);

        /*
        *  Set synchronous trace handler
        */
        void mco_trace_set_handler(mco_trace_message_handler_fn trace_fn);

        /*
        *      Set trace level
        */
        mco_severity mco_trace_set_level(mco_severity new_level);

        /* 
        *   Set trace modules mask
        */
        uint8 mco_trace_set_modules_mask(uint8 new_mask);


        /*
        * Load eXtremeDB core shared libraries
        */
        MCO_RET mco_core_load(mco_bool disk, mco_bool mvcc, mco_bool shmem,
            const char *sync_library, const char *memory_library, const char *runtime_path);

		/*
		*  Check id eXtremeDB core loaded shared libraries
		*/
		mco_bool mco_core_is_loaded(void);
#ifdef __cplusplus
        }
    #endif

#endif


