/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include "common.h"

#ifdef _VXWORKS
#undef main

int main(int argc, char* argv[]){
    char* _argv [] = {"empty"};
    return sample_main(0,_argv);
}
#endif

#ifdef _ECOS
#undef main

static char ecos_stack[512*1024];
cyg_handle_t   mh;
cyg_thread     mt;

void ecos_main(cyg_addrword_t data){
    char* _argv [] = {"empty"};
    sample_main(0,_argv);
}

int main(int argc, char* argv[]){
    memset(ecos_stack,0,sizeof(ecos_stack));
    
    cyg_thread_create( cyg_thread_get_priority(cyg_thread_self()), \
         ecos_main, (cyg_addrword_t)0, "exdbmain", ecos_stack, sizeof(ecos_stack), &mh, &mt );
    cyg_thread_resume(mh);
    return 0;
}
#endif

#if defined(_ECOS) /* Only eCos requires OS initialization at the moment */
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_if.h>

#ifdef _ECOS_FS_PRESENT
#include <cyg/fileio/fileio.h>
#endif

#ifdef _ECOS_NET_PRESENT
#include <network.h>
#endif

void sample_os_shutdown() {

#ifdef _ECOS_FS_PRESENT
  int rc;
  rc = umount( ECOS_FS_MOUNT_POINT );
#endif

  /*try to reset the target */
#ifdef CYGACC_CALL_IF_RESET
  printf("\nResetting.\n");
  CYGACC_CALL_IF_DELAY_US(2*100000);       /* delay (2*100000) microseconds.*/
  printf("\n");
  CYGACC_CALL_IF_RESET();                  /* do board reset. this is a ecos HAL reset macro, platform independently.*/
  printf("!! oops, RESET not working on this platform\n");
#endif
}

void sample_os_initialize(int req_features) {

#ifdef _ECOS_FS_PRESENT
  int rc;
  rc = mount( ECOS_FS_DEVICE, ECOS_FS_MOUNT_POINT, ECOS_FS_TYPE );
  if (rc < 0) {
      printf( "Unable to mount filesystem: %s\n", strerror(errno) );
      sample_os_shutdown();
      exit(1);
  }
#else
  if ( (req_features & DISK ) == DISK ) {
      printf("This sample requires filesystem support.\n");
      sample_os_shutdown();
      exit(1);
  }
#endif

#ifdef _ECOS_NET_PRESENT
  init_all_network_interfaces();
#else
  if ( (req_features & NET) == NET ) {
      printf("This sample requires network support.\n");
      sample_os_shutdown();
      exit(1);
  }
#endif

}
#endif

#if defined(_UCOS3)
#undef main

#include  <app_cfg.h>
#include  <os.h>
#include  <lib_mem.h>
#include  <lib_math.h>

static  OS_TCB        App_TaskStartTCB;
static  CPU_STK_SIZE  App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  void  App_TaskStart (void *p_arg)
{
    OS_ERR      os_err;
    char* _argv [] = {"empty"};

    CPU_Init();
    Mem_Init();
    Math_Init();

    OS_CPU_SysTickInit();

    sample_main(0,_argv);

    while (DEF_TRUE) { }
}

int  main (void)
{
    OS_ERR  err;

    OSInit(&err);
    OSTaskCreate((OS_TCB     *)&App_TaskStartTCB,
                 (CPU_CHAR   *)"eXdb sample main task",
                 (OS_TASK_PTR ) App_TaskStart,
                 (void       *) 0,
                 (OS_PRIO     ) APP_CFG_TASK_START_PRIO,
                 (CPU_STK    *)&App_TaskStartStk[0],
                 (CPU_STK     )(APP_CFG_TASK_START_STK_SIZE / 10u),
                 (CPU_STK_SIZE) APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  ) 0,
                 (OS_TICK     ) 0,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSStart(&err);
    while(DEF_ON){ };
}

#endif

