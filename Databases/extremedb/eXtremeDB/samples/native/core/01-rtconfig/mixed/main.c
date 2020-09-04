/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <mco.h>
#include <common.h>

int main(int argc, char* argv[])
{ 
  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  sample_header("Sample 'mixed' demonstrates how to configure the linker and mcocomp flags\n"
                "to build an application for different runtime configurations using a database\n"
                "that defines both persistent and transient classes.\n" );
 
  sample_show_runtime_info("");

  /* Stop eXtremeDB runtime */
  mco_runtime_stop();

  sample_pause_end("\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}
