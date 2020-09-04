/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#ifndef __MCO_SAMPLES_INITOS_H
#define __MCO_SAMPLES_INITOS_H

#ifdef __cplusplus
extern "C" {
#endif 

#if defined(_VXWORKS) || defined(_ECOS) || defined(_UCOS3)
  #define main sample_main
  int sample_main(int argc, char* argv[]);
#endif

#if defined(_ECOS)
#include <cyg/kernel/kapi.h>
#endif

/* OS initialization options. Set only if the sample uses regarding subsystems impliciltly. */
#define DEFAULT 0  /* nothing special, just basic OS initialization */
#define DISK    1  /* the sample is requiring disk and fs support */
#define NET     2  /* the sample is requiring network support */

#if defined(_ECOS) /* Only eCos requires OS initialization at the moment */
#define ECOS_FS_DEVICE "/dev/flash/1/0,0xa00000"
#define ECOS_FS_MOUNT_POINT "/"
#define ECOS_FS_TYPE "jffs2"

#ifdef _ECOS_NET_PRESENT
#include <network.h>
#endif

#ifdef FILE_PREFIX
#undef FILE_PREFIX
#endif
#define FILE_PREFIX ECOS_FS_MOUNT_POINT

void sample_os_initialize(int req_features);
void sample_os_shutdown();

#else
#define sample_os_initialize(skip)
#define sample_os_shutdown()
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __MCO_SAMPLES_COMMON_H */

