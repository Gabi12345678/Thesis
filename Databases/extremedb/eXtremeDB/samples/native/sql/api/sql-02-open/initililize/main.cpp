/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "initializedb.h"

// Define global McoSQL engine 
using namespace McoSql;
McoSqlEngine engine; 

char sample_descr[] = 
"Sample 'initialize' demonstrates basic application initialization\n"
"using the eXtremeSQL C++ API.\n";

const size_t MAX_LINE_LENGTH = 2048;
const char * db_name = "initializedb";

int main(int argc, char* argv[])
{
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);
       
  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, initializedb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
   
  printf("\tDatabase opened ... ");

  /* Do normal database processing ... */

  engine.close();
  printf("\n\tDatabase closed ... ");

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}
