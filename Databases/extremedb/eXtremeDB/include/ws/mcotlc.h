/************************************************************
 *                                                          *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.*
 *                                                          *
 ************************************************************/

#ifndef _MCO_EWS_TRANSPORT_LAYER_TCPIP_H
#define _MCO_EWS_TRANSPORT_LAYER_TCPIP_H

#include <mcowscfg.h>

#ifdef _INTEGRITY
#include <INTEGRITY.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#else /* WIN32 */
#ifdef _VXWORKS
#include <ioLib.h>
#include <sockLib.h>
#include <hostLib.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef _VXWORKS
#include <sys/time.h>
#endif /*_VXWORKS*/
#endif /* WIN32 */

#include <mco.h>
#include <mconet.h>

/*
 *
 * Transport Layer
 *
 */
typedef struct tag_mcohs_tlc_addr
{
    struct in_addr addr;
    int port;
} mcohs_tlc_addr_t, *mcohs_tlc_addr_h;


typedef struct tag_mco_tlcs
{
    mco_socket_t *s;
    void *ssl_params;
    mcohs_tlc_addr_t peer_addr;

    char *input;
    unsigned int input_sz;

    unsigned int sent_sz;

#ifdef MCO_HTTP_BUFFERED_CONNECTIONS
    unsigned int flags;
    char *snd_buf;
    unsigned int snd_sz;
#endif /* MCO_HTTP_BUFFERED_CONNECTIONS */

} mco_tlcs_t, *mco_tlcs_h;

typedef void *mco_tlcs_keys_t;
typedef mco_tlcs_keys_t *mco_tlcs_keys_h;

typedef void *mcohs_tlcs_init_t;
typedef mcohs_tlcs_init_t *mcohs_tlcs_init_h;


#endif /* _MCO_EWS_TRANSPORT_LAYER_TCPIP_H */
