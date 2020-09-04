#ifndef MCO_CLUSTER_H__
#define MCO_CLUSTER_H__

#ifdef __cplusplus
extern "C"
{
#endif 

#include <mco.h>
#include <mcotime.h>
#include <mconet.h>
#include <mcorest.h>

#ifndef MCOCLUSTER_API
# ifdef _WIN32
#  ifdef MCOCLUSTER_DLL
#    define MCOCLUSTER_API __declspec(dllexport)
#  else
#    ifdef MCOCLUSTER_USE_DLL
#       define MCOCLUSTER_API __declspec(dllimport)
#    else
#       define MCOCLUSTER_API 
#    endif
#  endif
# else
#  define MCOCLUSTER_API
# endif
#endif


#define MCO_MAX_CLUSTER_ADDRESS_LEN 64

#define MCO_MAX_CLUSTER_SIZE 32 /* max nodes num */

#define INVALID_NODE_ID  ((uint2) -1)


typedef void (*mco_cluster_transsize_f) (uint4 trans_size);
typedef mco_bool (*mco_cluster_check_quorum_f) (uint2 *neighbor_ids, uint2 n_neighbors, void *param);

typedef struct mco_cluster_node_params_t_ {
    char  addr[MCO_MAX_CLUSTER_ADDRESS_LEN];
    uint4 qrank;
} mco_cluster_node_params_t;


typedef struct mco_cluster_window_t_
{
    uint4                      bsize;  /* in bytes */
    uint4                      length; /* in transactions */
    timer_unit                 timeout;
} mco_cluster_window_t;


typedef enum mco_cluster_notification_t_
{
    MCO_CLUSTER_NOTIFY_NODE_CONNECT = 0, 
    MCO_CLUSTER_NOTIFY_NODE_DISCONNECT, 
} mco_cluster_notification_t;

typedef struct mco_cluster_node_info_t_ {
    char  addr[MCO_MAX_CLUSTER_ADDRESS_LEN];
    uint4 qrank;
    uint2 node_id;
} mco_cluster_node_info_t;

typedef void(*mco_cluster_notifying)(mco_db_h db, mco_cluster_notification_t notification_code, mco_cluster_node_info_t *node_info, void *param, void *user_context);

typedef void* (*mco_cluster_connection_context_f) (void *user_param);

typedef struct mco_clnw_tcp_params_t_
{
    uint4                      so_sndbuf;
    uint4                      so_rcvbuf;
    timer_unit                 connect_timeout;
    timer_unit                 connect_interval;
    timer_unit                 keepalive_time;
    uint4                      keepalive_probes;
    MCO_SOCK_DOMAIN            socket_domain;
    int                        compression_level;
    void                      *ssl_params;
} mco_clnw_tcp_params_t;

#define MCO_CLUSTER_MPI_BUSYWAIT           0x1
#define MCO_CLUSTER_MPI_SERIALIZED_SEND    0x2

typedef struct mco_clnw_mpi_params_t_
{
    uint4                      flags;
    void                      *communicator;
} mco_clnw_mpi_params_t;

/* send transaction data to other nodes before mco_trans_commit_phase1() */
#define MCO_CLUSTER_MODE_EARLY_DATA_SEND   0x2
#define MCO_CLUSTER_MODE_BINARY_EVOLUTION  0x4
#define MCO_CLUSTER_MODE_START_DETACHED    0x8

typedef struct mco_cluster_params_t_
{
    mco_cluster_node_params_t       *nodes;
    uint2                            n_nodes;
    uint2                            node_id;
    uint2                            conn_pool_factor;
    uint4                            sync_msg_objects; /* max number of objects per message during synchronization */
    uint4                            sync_msg_size;    /* max size of message in bytes during synchronization */
    mco_cluster_transsize_f          transsize_callback;
    mco_cluster_check_quorum_f       check_quorum_func;
    void                            *check_quorum_param;
    mco_cluster_window_t             window;
    mco_cluster_notifying            notifying_callback;
    void                            *notifying_context;
    mco_cluster_connection_context_f connect_ctx_callback;
    void                            *connect_ctx_param;
    uint4                            cl_sndbuf;
    uint4                            cl_rcvbuf;
    uint4                            mode_mask;
    void*                            stream_handle;
    mco_stream_read                  input_stream_reader;
    union   {
        mco_clnw_tcp_params_t tcp;
        mco_clnw_mpi_params_t mpi;
    } nw;
} mco_cluster_params_t, *mco_cluster_params_h;

typedef struct mco_cluster_info_t_
{
    uint4 rtrans_commit;
    uint4 rtrans_rback;
    uint8 bytes_sent;
    uint8 bytes_recv;
    uint2 n_active_nodes;
    uint2 node_id;
} mco_cluster_info_t, *mco_cluster_info_h;


MCOCLUSTER_API MCO_RET mco_cluster_init(void);
MCOCLUSTER_API MCO_RET mco_cluster_params_init(mco_cluster_params_t *cl_params);
MCOCLUSTER_API MCO_RET mco_cluster_db_open(const char* db_name, mco_dictionary_h dict, mco_device_t * devs, 
                        mco_size_t n_devs, mco_db_params_t * db_params, mco_cluster_params_t * cl_params);
MCOCLUSTER_API MCO_RET mco_cluster_attach(mco_db_h db, mco_cluster_params_t * cl_params);
MCOCLUSTER_API MCO_RET mco_cluster_listen(mco_db_h db);
MCOCLUSTER_API MCO_RET mco_cluster_barrier(mco_db_h db);
MCOCLUSTER_API MCO_RET mco_cluster_stop(mco_db_h db);
MCOCLUSTER_API MCO_RET mco_cluster_detach(mco_db_h db);
MCOCLUSTER_API MCO_RET mco_cluster_info(mco_db_h db, /* OUT */ mco_cluster_info_t *cl_info);

MCOCLUSTER_API MCO_RET mco_cluster_get_window_params(mco_db_h db, mco_cluster_window_t * window_params);
MCOCLUSTER_API MCO_RET mco_cluster_set_window_params(mco_db_h db, mco_cluster_window_t * window_params);
MCOCLUSTER_API MCO_RET mco_cluster_get_active_nodes(mco_db_h db, mco_cluster_node_info_t *nodes_info, uint2 *n_nodes_info);

MCOCLUSTER_API MCO_RET mco_cluster_discover(const char *conn_string, timer_unit timeout, mco_cluster_node_info_t *nodes_info, uint2 *n_nodes_info);
MCOCLUSTER_API MCO_RET mco_cluster_discover_ex(const char *conn_string, timer_unit timeout, mco_cluster_node_info_t *nodes_info, uint2 *n_nodes_info, void *ssl_params, int compression_level);

MCOCLUSTER_API const char *mco_cluster_nw_type(void);

MCOCLUSTER_API MCO_RET mco_cluster_scatter(mco_db_h db, uint2 *class_codes, uint2 n_class_codes, uint2 *node_ids, uint2 n_node_ids);
MCOCLUSTER_API MCO_RET mco_cluster_gather(mco_db_h db, uint2 *class_codes, uint2 n_class_codes, uint2 *node_ids, uint2 n_node_ids);

MCOCLUSTER_API MCO_RET mco_cluster_register_rest_service(mco_db_h db, const char *path, mcorest_h rest);

#ifdef __cplusplus
}
#endif 
#endif /* MCO_CLUSTER_H__ */
