/****************************************************************
 *                                                              *
 *  mcossl.h                                                    *
 *                                                              *
 *  This file is a part of the eXtremeDB source code.           *
 *                                                              *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.  *
 *                                                              *
 *  Secure networking API                                       *
 *                                                              *
 ****************************************************************/

#ifndef __MCOSSL_H__
#define __MCOSSL_H__

#include <mcospec.h>
#include <mcotime.h>
#include <mconet.h>


#ifdef __cplusplus
extern "C"
{
#endif

/* Disable SSL 2. */
#define MCO_SSL_OPT_NO_SSLV2                0x01

/* Disable SSL 3. */
#define MCO_SSL_OPT_NO_SSLV3                0x02

/* Disable TLS 1.0. */
#define MCO_SSL_OPT_NO_TLSV1_0              0x04

/* Disable TLS 1.1. */
#define MCO_SSL_OPT_NO_TLSV1_1              0x08

/* Disable TLS 1.2. */
#define MCO_SSL_OPT_NO_TLSV1_2              0x10

/* Disable compression. */
#define MCO_SSL_OPT_NO_COMPRESSION          0x20

/* Always create a new key when using ephemeral DH parameters. */
#define MCO_SSL_OPT_SINGLE_DH_USE           0x40


/* Disable peer verification. */
#define MCO_SSL_VERIFY_NONE                 0x00

/* Enable peer verification. */
#define MCO_SSL_VERIFY_PEER                 0x01

/* Fail verification if the peer does not send a certificate. */
#define MCO_SSL_VERIFY_FAIL_IF_NO_PEER_CERT 0x02

/* Only request the client's certificate on the initial handshake. */
#define MCO_SSL_VERIFY_CLIENT_ONCE          0x04


typedef struct mco_ssl_params_t_ {
    /* A string containing the list of ciphers for the SSL engine. */
    const char *cipher_list;

    /* Maximum size of the peer's certificate chain in bytes. */
    long max_cert_list;

    /* SSL options (a combination of MCO_SSL_OPT_* flags). */
    unsigned long options;

    /* PEM-encoded DH parameters for the ephemeral DH key exchange. */
    const char *tmp_dh;

    /* Peer verification mode (a combination of MCO_SSL_VERIFY_* flags). */
    unsigned int verify_mode;

    /* Maximum depth for the certificate chain verification. */
    int verify_depth;

    /* PEM-encoded certificate. */
    const char *certificate;

    /* PEM-encoded private key. */
    const char *private_key;

    /* Path to the PEM-encoded certificate file. */
    const char *certificate_file;

    /* Path to the PEM-encoded private key file. */
    const char *private_key_file;
} mco_ssl_params_t;

MCONET_API MCO_RET mco_ssl_init(void);
MCONET_API void mco_ssl_cleanup(void);
MCONET_API void mco_ssl_params_init(mco_ssl_params_t *params);

MCONET_API MCO_RET mco_ssl_load_verify_locations(const char *ca_file, const char *ca_path);


#ifdef __cplusplus
}
#endif

#endif /* __MCOSSL_H__ */
