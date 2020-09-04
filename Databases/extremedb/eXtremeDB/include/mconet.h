/****************************************************************
 *                                                              *
 *  mconet.h                                                    *
 *                                                              *
 *  This file is a part of the eXtremeDB source code.           *
 *                                                              *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.  *
 *                                                              *
 *  Network abstraction API                                     * 
 *                                                              *
 ****************************************************************/

#ifndef __MCONET_H__
#define __MCONET_H__

#include <mcospec.h>
#include <mcotime.h>


#ifndef MCONET_API
# ifdef _WIN32
#  ifdef MCONET_DLL
#    define MCONET_API __declspec(dllexport)
#  else
#    ifdef MCONET_USE_DLL
#       define MCONET_API __declspec(dllimport)
#    else
#       define MCONET_API 
#    endif
#  endif
# else
#  define MCONET_API
# endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif 

typedef int mco_sock_handle_t;

extern char const* mco_unix_socket_dir;

typedef enum MCO_SOCK_OPEN_MODE_E_ {
    MCO_SOCK_NODELAY = 1,
    MCO_SOCK_DO_NOT_REUSE_ADDRESS = 2,
    MCO_SOCK_NON_BLOCKING = 4,
    MCO_SOCK_KEEP_ALIVE = 8,
    MCO_SOCK_DO_NOT_CLOEXEC = 16
} MCO_SOCK_OPEN_MODE;

typedef enum MCO_SOCK_DOMAIN_E_ {
    MCO_SOCK_INET_DOMAIN,    /* IPv4 or IPv6 */
    MCO_SOCK_INETV4_DOMAIN,  /* IPv4 only */
    MCO_SOCK_INETV6_DOMAIN,  /* IPv6 only */
    MCO_SOCK_LOCAL_DOMAIN,
    MCO_SOCK_SDP_DOMAIN
} MCO_SOCK_DOMAIN;

typedef enum MCO_SOCK_TYPE_E_ {
    MCO_SOCK_STREAM,
    MCO_SOCK_DGRAM,
    MCO_SOCK_MCAST,
} MCO_SOCK_TYPE;

typedef struct mco_sock_params_t_
{
    int                     mode;       /* bit set of MCO_SOCK_OPEN_MODE */
    int                     linger;
    int                     sndbuf;     /* size of socket send buffer in bytes */
    int                     rcvbuf;     /* size of socket receive buffer in bytes */
    int                     compression_level; /* level of compression */
    MCO_SOCK_DOMAIN         domain;     /* socket domain (may be flag in open_mode ?) */
    MCO_SOCK_TYPE           type;       /* TCP / UDP )may be flag in open_mode ? */
    const char             *mcast_addr; /* used only if type == MCO_SOCK_MCAST */
    void                   *ssl_params; /* used for SSL sockets when OpenSSL is supported */
    MCO_RET                 last_error; /* OUT : eXtremeDB's error code if mco_create_socket() has failed */
    int                     last_errno; /* OUT : value of errno if mco_create_socket() has failed */
} mco_sock_params_t;

typedef enum MCO_SOCK_EVENT_E_ {
    MCO_SOCK_CAN_READ = 1,
    MCO_SOCK_CAN_WRITE = 2,
    MCO_SOCK_CAN_ACCEPT = 4,
    MCO_SOCK_CONNECT_OK = 8,
    MCO_SOCK_CONNECT_FAILED = 16,
    MCO_SOCK_ACCEPT_OK = 32,
    MCO_SOCK_ACCEPT_FAILED = 64,
} MCO_SOCK_EVENT;


typedef struct mco_inetaddr_t_
{
    union {
        uint1 u1[16];
        uint2 u2[8];
        uint4 u4[4];
    };
    uint4 scope_id;
} mco_inetaddr_t;

#define MCO_NET_INETADDR_STRING_LEN 48

typedef struct mco_peer_info_t_
{
    mco_inetaddr_t ip;
    int            port;
    mco_size_t     rx_bytes;
    mco_size_t     tx_bytes;
} mco_peer_info_t;

typedef struct mco_socket_t { 
    mco_size_sig_t (*recv)(struct mco_socket_t* s, void* buf, mco_size_t min_bytes, mco_size_t max_bytes, timer_unit timeout);    
    mco_size_sig_t (*send)(struct mco_socket_t* s, void const* buf, mco_size_t min_bytes, mco_size_t max_bytes, timer_unit timeout);
    struct mco_socket_t* (*saccept)(struct mco_socket_t* s, timer_unit timeout);
    int (*listen)(struct mco_socket_t *s, int listen_queue_size);
    int (*connect)(struct mco_socket_t *s, char const* host, int port, timer_unit timeout);
    int (*get_handshake_event)(struct mco_socket_t *s, int events);
    int (*perform_handshake)(struct mco_socket_t *s, int events, timer_unit timeout);
    int (*has_pending_data)(struct mco_socket_t *s);
    int (*get_peer_info)(struct mco_socket_t* s, mco_peer_info_t *pinfo);
    int (*set_events)(struct mco_socket_t* s, int events); /* set MCO_SOCK_EVENT bits before mco_select_socket(). Returns bits before call */
    int (*check_events)(struct mco_socket_t* s, int events); /* check MCO_SOCK_EVENT bits after mco_select_socket() */
    int (*disconnect)(struct mco_socket_t* s);
    int (*shutdown)(struct mco_socket_t* s);
    struct mco_socket_t * (*duplicate)(struct mco_socket_t *s);
    void (*get_error_message)(struct mco_socket_t* s, char* buf, mco_size_t buf_size);

    mco_sock_handle_t handle;
    mco_sock_params_t params;
    int               events;
    int               revents;
    void             *user_data; /* arbitrary user data, can be used to link user data after mco_select_socket() */
    MCO_RET           last_error;
    int               last_errno;
} mco_socket_t;

typedef enum MCO_NET_CAPABILITIES_E_ {
    MCO_NET_CAP_SSL  = 1,
    MCO_NET_CAP_IPV6 = 2,
    MCO_NET_CAP_ZLIB = 4
} MCO_NET_CAPABILITIES;

MCONET_API extern void mco_socket_params_init(mco_sock_params_t *params); /* set default values for mco_sock_params_t */
MCONET_API extern mco_socket_t* mco_create_socket(char const* net_interface, int port, mco_sock_params_t *params); /* create socket and bind it to local address if specified */
MCONET_API extern int mco_select_socket(struct mco_socket_t* in[], struct mco_socket_t* out[], int n_sockets, timer_unit timeout);
MCONET_API extern unsigned long mco_net_get_capabilities(void);

MCONET_API extern MCO_RET mco_net_parse_address(const char *address, char *hostname, int hostname_len, int *port, const char **endptr);
MCONET_API extern int mco_net_ntoa(const mco_inetaddr_t *addr, char *str);
MCONET_API extern int mco_net_ntoa_ex(const mco_inetaddr_t *ipaddr, char *str, mco_bool v4mapped_as_ip4);

MCONET_API extern uint2 mco_ntohs(uint2 p);
MCONET_API extern uint2 mco_htons(uint2 p);
MCONET_API extern uint4 mco_ntohl(uint4 p);
MCONET_API extern uint4 mco_htonl(uint4 p);


#ifdef __cplusplus
}
#endif 

#endif
