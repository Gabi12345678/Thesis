/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "initializedb.h"

using namespace McoSql;

char sample_descr[] = 
"Sample 'allocator' demonstrates use of a dynamic memory allocator\n"
"using the eXtremeSQL C++ API.\n";

// Define custom memory management functions.  Note that this implementation uses
// C runtime functions malloc() and free(), but they could use pre-allocated or static
// memory and manage this memory block as desired.
void* buffer;
void * mem_alloc( size_t n_bytes )
{
  buffer = (void*)malloc( n_bytes );
  printf("\n\n\tmem_alloc(): %u bytes allocated at address 0x%" INT8_FORMAT "x\n",
                (unsigned int)n_bytes, (int64_t)buffer);
  return buffer;
}

void mem_free( void * buf )
{
  printf("\n\tmem_free(): memory freed at address 0x%" INT8_FORMAT "x\n", (int64_t)buf);
  free(buf);
}

const size_t MAX_LINE_LENGTH = 2048;
const char * db_name = "initializedb";

int main(int argc, char* argv[])
{
  McoSqlEngine engine; 

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);
       
  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, initializedb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
  printf("\tDatabase opened ... ");

  // Dynamic memory allocator is created and set by default
  
  // Set Dynamic memory allocator to custom functions mem_alloc and mem_free 
  StdAllocator allocator( mem_alloc, mem_free ); 
  printf("\n\tCustom Dynamic allocator set using custom functions... ");
  
  /* Do normal database processing ... */

  engine.close();
  printf("\n\tDatabase closed ... ");

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}
