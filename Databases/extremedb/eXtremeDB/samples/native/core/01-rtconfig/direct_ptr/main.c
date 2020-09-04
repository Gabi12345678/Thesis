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

  /* start eXtremeDB runtime */
  mco_runtime_start();

  sample_header("Sample 'direct_ptr' displays runtime information for a Direct Pointer database.\n" );

  sample_show_runtime_info("");

  /* stop eXtremeDB runtime */
  mco_runtime_stop();

  sample_pause("\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}
