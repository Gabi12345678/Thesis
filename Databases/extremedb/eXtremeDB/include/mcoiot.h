#ifndef MCO_IOT_H__
#define MCO_IOT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <mco.h>
#include <mconet.h>
#include <mcorest.h>

typedef mco_uint8 mco_iot_agent_id_t;
typedef mco_uint8 mco_iot_db_id_t;

#define MCO_IOT_ALL_AGENTS            0x0
#define MCO_IOT_SERVER_AGENT_ID       0x1
#define MCO_IOT_MAX_AGENT_ID          0xFFFFFFFFFFFFFFF0ULL /* 15 agent ID's reserved */
#define MCO_IOT_THIS_AGENT_ID         (MCO_IOT_MAX_AGENT_ID + 1)
#define MCO_IOT_ALL_UP_AGENTS         (MCO_IOT_MAX_AGENT_ID + 2)
#define MCO_IOT_ALL_DOWN_AGENTS       (MCO_IOT_MAX_AGENT_ID + 3)
#define MCO_IOT_INVALID_AGENT_ID      0xFFFFFFFFFFFFFFFFULL

#ifdef MCO_NO_FORWARDS
typedef void* mco_iot_comm_h;
typedef void* mco_iot_connection_h;
typedef void* mco_iot_replicator_h;
#else
typedef struct mco_iot_comm_t_* mco_iot_comm_h;
typedef struct mco_iot_connection_t_ *mco_iot_connection_h;
typedef struct mco_iot_replicator_t_* mco_iot_replicator_h;
#endif

#ifndef MCOIOTCOMM_API
# ifdef _WIN32
#  ifdef MCOIOTCOMM_DLL
#    define MCOIOTCOMM_API __declspec(dllexport)
#  else
    #    ifdef MCOIOTCOMM_USE_DLL
#       define MCOIOTCOMM_API __declspec(dllimport)
#    else
#       define MCOIOTCOMM_API
#    endif
#  endif
# else
#  define MCOIOTCOMM_API
# endif
#endif

#ifndef MCOIOTREPL_API
# ifdef _WIN32
#  ifdef MCOIOTREPL_DLL
#    define MCOIOTREPL_API __declspec(dllexport)
#  else
    #    ifdef MCOIOTREPL_USE_DLL
#       define MCOIOTREPL_API __declspec(dllimport)
#    else
#       define MCOIOTREPL_API 
#    endif
#  endif
# else
#  define MCOIOTREPL_API
# endif
#endif


/* Initialize IoT runtime */
MCOIOTREPL_API MCO_RET mco_iot_init(void);

/*********************************************************/
/************ IoT communicator definitions ***************/
/*********************************************************/

typedef struct mco_iot_comm_params_t_
{
    uint2              n_callback_threads;
    MCO_SOCK_DOMAIN    sock_domain;
    void              *ssl_params;
    timer_unit         send_timeout;
    timer_unit         recv_timeout;
    int                listen_queue_size;
    uint2              wakeup_port;
    int                compression_level;
} mco_iot_comm_params_t;

/* Initialize communicator's parameters */
MCOIOTCOMM_API MCO_RET mco_iot_comm_params_init(mco_iot_comm_params_t * params );
/* Create communicator */
MCOIOTCOMM_API MCO_RET mco_iot_comm_create(mco_iot_comm_params_t *params, mco_iot_comm_h *comm);

/* Start listening */
MCOIOTCOMM_API MCO_RET mco_iot_comm_listen(mco_iot_comm_h comm, const char *address, mco_sock_params_t *params);
/* Start connecting */
MCOIOTCOMM_API MCO_RET mco_iot_comm_connect_async(mco_iot_comm_h comm, const char *address, timer_unit timeout, unsigned int n_attempts, timer_unit interval, mco_sock_params_t *sock_params);

/* Stop listening */
MCOIOTCOMM_API MCO_RET mco_iot_comm_stop_listen(mco_iot_comm_h comm, const char *address);
/* Stop connecting */
MCOIOTCOMM_API MCO_RET mco_iot_comm_stop_connect(mco_iot_comm_h comm, const char *address);

MCOIOTCOMM_API mco_bool mco_iot_comm_is_stopped(mco_iot_comm_h comm);
MCOIOTCOMM_API int mco_iot_comm_process_connections(mco_iot_comm_h comm, timer_unit timeout);

/************* Communicator callbacks ******************/

/* Callback return codes */
#define IOT_CALLBACK_OK     0x0  /* normal */
#define IOT_CALLBACK_FAIL   0x1  /* error, disconnect the client */
#define IOT_CALLBACK_STOP   0x2  /* error, don't call sebsequent callbacks in the chain */

typedef struct mco_iot_read_stream_t_
{
    void            *handle;
    mco_stream_read  reader;
} mco_iot_read_stream_t;

typedef struct iot_ack_t_ {
    uint8     seq;
    uint8     sender_agent_id;
    uint8     sender_db_id;
    uint8     receiver_agent_id;
    uint8     timestamp;     /* ACKed TS for 'private' objects (with specified DST) */
    uint8     timestamp_cmn; /* ACKed TS for 'common' objects (with DST == ALL_AGENTS) */
    uint4     error_code;
    uint4     flags;
} iot_ack_t;

typedef struct iot_comm_callback_t_
{
    int (*onConnect)    (mco_iot_connection_h iotc, void *context);
    int (*onReceive)    (mco_iot_connection_h iotc, mco_iot_read_stream_t *stream, void *context);
    int (*onAck)        (mco_iot_connection_h iotc, const iot_ack_t *ack, void *context);
    int (*onDisconnect) (mco_iot_connection_h iotc, void *context);
    int (*onDestroy)    (mco_iot_connection_h iotc, void *context);
 } iot_comm_callback_t;

/* Register callbacks */
MCOIOTCOMM_API MCO_RET mco_iot_comm_register_callback(mco_iot_comm_h comm, iot_comm_callback_t *callback, void *context);
/* Unregister callbacks */
MCOIOTCOMM_API MCO_RET mco_iot_comm_unregister_callback(mco_iot_comm_h comm, void *context);


/* Stop communicator */
MCOIOTCOMM_API MCO_RET mco_iot_comm_stop(mco_iot_comm_h comm);
/* Destroy communicator */
MCOIOTCOMM_API MCO_RET mco_iot_comm_destroy(mco_iot_comm_h comm);


/*********************************************************/
/************ IoT replicator definitions *****************/
/*********************************************************/

typedef struct mco_iot_replicator_params_t_
{
    uint2      conn_pool_size;
    timer_unit wait_timeout;
} mco_iot_replicator_params_t;

/* Initialize replicator's parameters */
MCOIOTREPL_API void mco_iot_replicator_params_init(mco_iot_replicator_params_t *params);
/* Create replicator */
MCOIOTREPL_API MCO_RET mco_iot_replicator_create(mco_db_h db, mco_iot_comm_h comm, mco_iot_replicator_params_t *params, /* OUT */ mco_iot_replicator_h *repl);

/* Blocking connect */
MCOIOTREPL_API MCO_RET mco_iot_replicator_connect(mco_iot_replicator_h repl, const char *address, timer_unit timeout, mco_sock_params_t *sock_params);

/* Replicate data */
#define MCO_IOT_SYNC_PUSH       0x1   /* Send data to other agent(s) */
#define MCO_IOT_SYNC_PULL       0x2   /* Request data from other agent(s) */
#define MCO_IOT_SYNC_NONBLOCK   0x4   /* Don't block on send() operation */
#define MCO_IOT_SYNC_WAIT       0x8   /* Wait for replication completion */
#define MCO_IOT_SYNC_BOTH       (MCO_IOT_SYNC_PUSH | MCO_IOT_SYNC_PULL)

#define MCO_IOT_SYNC_DEEP_ACK   0x10  /* request ACK from the destination */
#define MCO_IOT_SYNC_RESEND     0x20  /* resend non-ACKed data */
#define MCO_IOT_SYNC_CHILDS     0x40  /* send data to agent and all its childs */

MCOIOTREPL_API MCO_RET mco_iot_replicator_sync(mco_iot_replicator_h repl, mco_iot_agent_id_t agent_id, int flags);

/* Remove needless data (sended and ack-ed by other side) */
MCOIOTREPL_API MCO_RET mco_iot_replicator_clear(mco_iot_replicator_h repl, mco_iot_agent_id_t agent_id);
/* Remove needless data in the specified class only */
MCOIOTREPL_API MCO_RET mco_iot_replicator_clear_class(mco_iot_replicator_h repl, mco_iot_agent_id_t agent_id, uint2 class_code);

/* Enumerate all active connections */
typedef mco_bool (*mco_iot_agent_enumerator_t)(mco_iot_replicator_h repl, mco_iot_connection_h iotc, void *context);
MCOIOTREPL_API MCO_RET mco_iot_replicator_enum_agents(mco_iot_replicator_h repl, mco_iot_agent_enumerator_t enumerator, void *context);

/* Get my agentID */
MCOIOTREPL_API MCO_RET mco_iot_replicator_get_agent_id(mco_iot_replicator_h repl, mco_iot_agent_id_t *agent_id);
/* Get my level */
MCOIOTREPL_API MCO_RET mco_iot_replicator_get_level(mco_iot_replicator_h repl, uint2 *level);
/* Change my level (only for routers) */
MCOIOTREPL_API MCO_RET mco_iot_replicator_set_level(mco_iot_replicator_h repl, uint2 level);
/* Get communicator by replicator */
MCOIOTREPL_API mco_iot_comm_h  mco_iot_replicator_get_comm(mco_iot_replicator_h repl);

MCOIOTREPL_API MCO_RET mco_iot_replicator_register_rest_service(mco_iot_replicator_h repl, const char *path, mcorest_h rest);

/* Stop replicator */
MCOIOTREPL_API MCO_RET mco_iot_replicator_stop(mco_iot_replicator_h repl);
/* Destroy replicator */
MCOIOTREPL_API MCO_RET mco_iot_replicator_destroy(mco_iot_replicator_h repl);


/*********************************************************/
/************ IoT connection definitions *****************/
/*********************************************************/

/* Find & iterate over active connections */
MCOIOTCOMM_API mco_iot_connection_h  mco_iot_comm_find_conn(mco_iot_comm_h comm, mco_iot_agent_id_t agent_id);
MCOIOTCOMM_API mco_iot_connection_h  mco_iot_comm_next_conn(mco_iot_comm_h comm, mco_iot_connection_h prev);
MCOIOTCOMM_API MCO_RET               mco_iot_conn_release(mco_iot_connection_h iotc);

/* Get agentID for connection */
MCOIOTCOMM_API mco_iot_agent_id_t mco_iot_conn_get_agent_id(const mco_iot_connection_h iotc);

/* Get connection string (for listen() or connect() sockets only */
MCOIOTCOMM_API const char* mco_iot_conn_get_address(mco_iot_connection_h iotc);

/* Get statistic for connection */
typedef struct mco_iot_connection_stat_t_
{
    mco_inetaddr_t     inetaddr;                /* peer IP */
    int                port;                    /* peer port */
    uint8              sent_bytes;              /* TX bytes */
    uint8              recv_bytes;              /* RX bytes */
    uint2              sent_compression_ratio;  /* TX compression ratio in percents */
    uint2              recv_compression_ratio;  /* RX compression ratio in percents */
} mco_iot_connection_stat_t;

MCOIOTCOMM_API MCO_RET mco_iot_conn_get_stat(mco_iot_connection_h iotc, mco_iot_connection_stat_t *stat);

/* Get reason of disconnect/destroy */
typedef enum mco_iot_stop_reason_t_
{
    MCO_IOT_STOP_REASON_NONE = 0,
    MCO_IOT_STOP_REASON_NETWORK_ERROR,
    MCO_IOT_STOP_REASON_CALLBACK_FAILED,
    MCO_IOT_STOP_REASON_IOT_STOPPED,
    MCO_IOT_STOP_REASON_USER_REQUEST
} mco_iot_stop_reason_t;

MCOIOTCOMM_API void mco_iot_conn_get_error(mco_iot_connection_h iotc, mco_iot_stop_reason_t *reason, MCO_RET *last_error, int *last_errno, char *message, int message_len);

/* Get communicator by connection */
MCOIOTCOMM_API mco_iot_comm_h mco_iot_conn_get_comm(const mco_iot_connection_h iotc);

/* Disconnect active connection */
MCOIOTCOMM_API MCO_RET mco_iot_conn_disconnect(mco_iot_connection_h iotc);

#ifdef __cplusplus
}
#endif
#endif
