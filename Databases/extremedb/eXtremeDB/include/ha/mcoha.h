/*******************************************************************
 *                                                                 *
 *  mcoha.h                                                          *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 ***************************************************************** */
#ifndef MCO_HA_H_
#define MCO_HA_H_

#ifdef __cplusplus
extern "C"
{
#endif 

#include <mco.h>
#include <mcotime.h>
#include <mconet.h>
#include <mcorest.h>

#if defined WIN32 || defined _WIN32
#include <winsock2.h>
#else
typedef int SOCKET;
#endif

#ifndef MCOHA_API
# ifdef _WIN32
#  ifdef MCOHA_DLL
#    define MCOHA_API __declspec(dllexport)
#  else
    #    ifdef MCOHA_USE_DLL
#       define MCOHA_API __declspec(dllimport)
#    else
#       define MCOHA_API 
#    endif
#  endif
# else
#  define MCOHA_API
# endif
#endif

#ifndef MCONWTCP_API
# ifdef _WIN32
#  ifdef MCONWTCP_DLL
#    define MCONWTCP_API __declspec(dllexport)
#  else
#    ifdef MCONWTCP_USE_DLL
#       define MCONWTCP_API __declspec(dllimport)
#    else
#       define MCONWTCP_API 
#    endif
#  endif
# else
#  define MCONWTCP_API
# endif
#endif

#ifndef MCONWUDP_API
# ifdef _WIN32
#  ifdef MCONWUDP_DLL
#    define MCONWUDP_API __declspec(dllexport)
#  else
#    ifdef MCONWUDP_USE_DLL
#       define MCONWUDP_API __declspec(dllimport)
#    else
#       define MCONWUDP_API 
#    endif
#  endif
# else
#  define MCONWUDP_API
# endif
#endif

#ifndef MCONWPIP_API
# ifdef _WIN32
#  ifdef MCONWPIP_DLL
#    define MCONWPIP_API __declspec(dllexport)
#  else
#    ifdef MCONWPIP_USE_DLL
#       define MCONWPIP_API __declspec(dllimport)
#    else
#       define MCONWPIP_API 
#    endif
#  endif
# else
#  define MCONWPIP_API
# endif
#endif

/* initilize HA subsystem */
MCOHA_API MCO_RET mco_HA_start( void );
MCOHA_API MCO_RET mco_HA_channel_implementation_add(const void * vt_table);

MCONWTCP_API const void *mco_nw_tcpip_vt( void);
MCONWUDP_API  const void *mco_nw_udpip_vt( void);
MCONWPIP_API const void *mco_nw_pipe_vt(  void );

typedef void *mco_channel_h;

#define DEFAULT_INITIAL_TIMEOUT          1000*60  /**< time to establish the initial connection and the initial */
#define DEFAULT_MASTER_COMMIT_TIMEOUT    1000*10  /**< time to disconnect a replica from the master */
#define DEFAULT_REPLICA_COMMIT_TIMEOUT   1000*10  /**< total database update time at the replica side */
#define DEFAULT_WAIT_FOR_DATA_TIMEOUT    1000*10  /**< amount of time a replica waits for the next transaction. */
#define DEFAULT_SYNCH_TIMEOUT            1000*10  /**< amount of time a replica waits for the synchronization */
#define DEFAULT_DETACH_TIMEOUT           1000*10  /**< amount of time a replica waits for the synchronization */
#define DEFAULT_MULTICAST_IP_ADDR        "224.0.0.1"
#define DEFAULT_HOTSYNC_MSG_OBJECTS      100
#define DEFAULT_HOTSYNC_MSG_SIZE         (100 * 1024)
#define DEFAULT_COMPRESSION_LEVEL        0
/* ==================================== Master API =================================== */

/* modes */
#define MCO_MASTER_MODE                  0x1
#define MCO_HAMODE_MULTIPROCESS_COMMIT   0x2
#define MCO_HAMODE_ASYNCH                0x4
#define MCO_HAMODE_MCAST                 0x8
#define MCO_HAMODE_MCAST_RELIABLE        0x10
#define MCO_HAMODE_HOTSYNCH              0x20
#define MCO_HAMODE_STATEFUL_REPLICATION  0x40
#define MCO_HAMODE_BINEVOLUTION          0x80

/* detach replica callback parameter */
typedef struct ha_error_
{
    MCO_RET errcode; 
    mco_channel_h channel;
    uint2 channel_index;
    void *context;
} ha_error_t, * ha_error_h;

/* detach handler prototype */
typedef void(*mco_HA_ErrorHandler)(ha_error_h HAerror);

typedef struct 
{
    uint4 mode_flags; /* < HA modes & options flags */
    uint2 max_number_of_replicas; 
    mco_device_t async_databuf; /* for MCO_HAMODE_ASYNC */
    uint4 trans_log_length; /* for STATEFUL_REPLICATION */
    timer_unit commit_timeout; /* to send transaction data to replica(s) */
    timer_unit initial_timeout; /* attach_replica() timeout (initial synchronization) */
    timer_unit synch_timeout;  /* to wait ACK from replica(s) */
    timer_unit detach_timeout;  /* to detach replica(s) */
    uint2 mcast_port; /* for multicast */
    const char *mcast_addr;
    mco_HA_ErrorHandler errhandler; /* detach replica callback */
    void  *errhandler_ctx; /* detach replica callback context */
    uint4  hotsync_msg_objects;
    uint4  hotsync_msg_size;
    uint4  hotsync_delay;
    uint8  initial_ha_sequencer;
    void  *ssl_params;
    mco_bool disable_writes_on_kill;
    uint2  quorum;
    int compression_level;
} mco_HA_master_params_t;


/* set default values for master_params */
MCOHA_API void mco_HA_master_params_init(mco_HA_master_params_t *master_params);

/* set parameters for master database */
MCOHA_API MCO_RET mco_HA_set_master_params(mco_db_h db, mco_HA_master_params_t* master_params);

/* attach replicas (sep. thread, in the loop) */
MCOHA_API MCO_RET mco_HA_attach_replica(mco_db_h db, const char* masterport, timer_unit connect_timeout);

/* for ASYNC replication, sep. thread */
MCOHA_API MCO_RET mco_HA_async_send_data_to_replicas(mco_db_h dbh);

/* for shared commit, sep.thread */
MCOHA_API MCO_RET mco_HA_trans_commit_synch(mco_db_h db);

MCOHA_API MCO_RET mco_HA_keep_alive(mco_db_h db, timer_unit timeout);

/* set mcast addr & port if multiple channels defined */
MCOHA_API MCO_RET mco_HA_set_mcast_addr(mco_db_h db, const char *prefix, const char *mcast_addr, uint2 mcast_port);

/* get replica's channel */
MCOHA_API uint2 mco_HA_get_number_of_replicas(mco_db_h db);
MCOHA_API mco_channel_h mco_HA_get_io_channel(mco_db_h db, int2 index);
MCOHA_API uint4 mco_HA_get_ip_addr(mco_channel_h chan); /* deprecated */
MCOHA_API void mco_HA_get_inet_addr(mco_channel_h chan, mco_inetaddr_t *addr);
MCOHA_API uint2 mco_HA_get_port(mco_channel_h chan);

/* detach replica(s) */
MCOHA_API MCO_RET mco_HA_detach_replica(mco_db_h db, mco_channel_h ch);
MCOHA_API MCO_RET mco_HA_detach_all_replicas(mco_db_h db);

/* stop replication */
MCOHA_API MCO_RET mco_HA_stop(mco_db_h db);

/* transaction window API */
MCOHA_API MCO_RET mco_HA_set_trans_window_size(mco_trans_h t, uint4 window_size);
MCOHA_API MCO_RET mco_HA_commit_window(mco_db_h db);

MCOHA_API uint4 mco_HA_get_mode(mco_db_h db);

MCOHA_API MCO_RET mco_HA_set_quorum(mco_db_h db, uint2 quorum);

MCOHA_API int2 allocate_index();
MCOHA_API void deallocate_index(int2 index);

/* ==================================== Replica API =================================== */

#define MCO_HAMODE_ALLOW_CANCEL           0x2000
#define MCO_HAMODE_FORCE_MASTER           0x4000
#define MCO_HAMODE_REPLICA_NOTIFICATION   0x8000
#define MCO_HAMODE_FORCE_SYNC             0x10000
#define MCO_HAMODE_SEND_RESTLIST          0x20000
#define MCO_HAMODE_EXPLICIT_WRITE_ACCESS  0x40000

typedef enum mco_ha_notification_t
{
    MCO_REPL_NOTIFY_CONNECTED = 0,  /**< "connected" notification */
    MCO_REPL_NOTIFY_CONNECT_FAILED,  /**< "connect failed" notification */
    MCO_REPL_NOTIFY_DB_EQUAL,  /**< "no need to load DB" notification */
    MCO_REPL_NOTIFY_DB_LOAD_BEGIN,  /**< "begin loading DB" notification */
    MCO_REPL_NOTIFY_DB_LOAD_FAILED,  /**< "loading failed" notification, "param1" of notification callback contains MCO_RET code */
    MCO_REPL_NOTIFY_DB_LOAD_OK,  /**< "succesful loading" notification, "param1" of notification callback contains MCO_E_HA_REPLICA_STOP_REASON code */
    MCO_REPL_NOTIFY_COMMIT_FAILED,  /**< "commit failed" notification, "param1" of notification callback contains MCO_RET code */
    MCO_REPL_NOTIFY_REPLICA_STOPPED,  /**< "stopped" notification, */
    MCO_REPL_NOTIFY_DB_CREATION_FAILED,  /**< "database creation failed" notification, "param1" of notification callback contains MCO_RET code */
    MCO_REPL_NOTIFY_HOTSYNC,  /**< "begining of hot synchronization" notification */
    MCO_REPL_NOTIFY_EOHOTSYNC, /**< "end of hot synchronization" notification */
    MCO_REPL_NOTIFY_STATEFUL_SYNC,  /**< "begining of stateful replication" notification */
    MCO_REPL_NOTIFY_STATEFUL_SYNC_END, /**< "end of stateful replication" notification */
    MCO_REPL_NOTIFY_MASTER_DB_EXTENDED, /**< master database was extended by mco_db_extend_dev() call */
    MCO_REPL_NOTIFY_MASTER_DB_CLEANED, /**< master database was cleared by mco_db_clean() call */
    MCO_REPL_NOTIFY_SCHEMA_CHANGED   /**< master execute DDL statement */
} MCO_HA_NOTIFICATION;

typedef void(*mco_ha_notifying)(uint2 notification_code, uint4 param1, void* param2, void* pcontext);

typedef struct 
{
    uint4 mode_flags;
    timer_unit initial_timeout; /* < initial sync timeout */
    timer_unit commit_timeout; /* read transaction data */
    timer_unit wait_data_timeout; /* < amount of time replica waits for the next commit */
    mco_ha_notifying notifying_callback; /* < pointer to notification callback routine */
    void* notifying_context; /* < user-defined argument for notification callback routine */
    uint2 repeat_counter; /* < counter of attempts to repeat the commit */
    uint2 mcast_port;
    const char *mcast_addr;
    const char* cancelpoint_addr;
    MCO_COMMIT_POLICY initial_commit_policy; /* disk commit policy during initial synchronization */
    uint4             initial_objs_in_trans; /* number of objects per transaction during initial synchronization */
    void *ssl_params;
    mco_trans_iterator_callback_t iterator;
    void                         *iterator_context;
    uint4         batch_commit_length; /* in transactions */
    uint4         batch_commit_bsize;   /* in bytes */
    timer_unit    batch_commit_period; /* in milliseconds */
    int           compression_level;
} mco_HA_replica_params_t;

MCOHA_API void mco_HA_replica_params_init(mco_HA_replica_params_t *replica_params);

typedef enum
{
    MCO_HA_REPLICA_CONNECTION_ABORTED, MCO_HA_REPLICA_MASTER_REQUESTED_DISCONNECT,
        MCO_HA_REPLICA_HANDSHAKE_FAILED, MCO_HA_REPLICA_STOPPED_BY_LOCAL_REQUEST, 
    MCO_HA_REPLICA_BECOMES_MASTER, MCO_HA_REPLICA_ITERATOR_ERROR
} MCO_E_HA_REPLICA_STOP_REASON;

MCOHA_API MCO_RET mco_HA_attach_master(mco_db_h db, const char* conn_string, const mco_HA_replica_params_t* params,
                                     MCO_E_HA_REPLICA_STOP_REASON* stop_reason, timer_unit timeout);

MCOHA_API MCO_RET mco_HA_replica_stop(mco_db_h db);

/* cancel API */
MCOHA_API MCO_RET mco_HA_cancel(const char *cancel_addr, timer_unit timeout);
MCOHA_API MCO_RET mco_HA_accept_cancellation_socket(const char* cancel_port, timer_unit timeout, mco_channel_h *chan); /* Deprecated */

MCOHA_API MCO_RET mco_HA_create_cancel_socket(mco_db_h db, const char *cancel_port, /* OUT */ SOCKET *cancel_socket);
MCOHA_API MCO_RET mco_HA_destroy_cancel_socket(mco_db_h db, SOCKET cancel_socket);

/* ===================================================================================================================== */

/* network usage statistics */
typedef struct mco_channel_info_t_
{
    char* protocol_type; /* nw transport protocol type name */
    mco_counter_t BytesSent; /* number of bytes sent */
    mco_counter_t BytesRcvd; /* number of bytes received */
    mco_bool support_unreliable; /* true, if transport support unreliable data transfer */
    mco_bool async_mode; /* true, if replica operates in ASYNC mode */
} mco_channel_info_t, *mco_channel_info_h;

MCOHA_API mco_channel_info_h mco_HA_get_channel_info(mco_channel_h ch);

MCOHA_API MCO_RET mco_get_current_auto_oid(mco_trans_h t,  /* OUT */mco_uquad* retvalue);

MCOHA_API MCO_RET mco_HA_get_sequencer(mco_db_h db, /* OUT */ uint8 *seq);

MCOHA_API MCO_RET mco_HA_enable_filter(mco_db_h db, mco_bool enabled);

MCOHA_API MCO_RET mco_HA_register_rest_service(mco_db_h db, const char *path, mcorest_h rest);

#ifdef __cplusplus
}
#endif 

#endif
