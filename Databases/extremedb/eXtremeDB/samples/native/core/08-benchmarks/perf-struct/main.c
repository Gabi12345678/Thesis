/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mco.h>
#include <common.h>
#include "structdb.h"
#include "create_objects.h"
#include "hash_search.h"
#include "tree_search.h"
#include "sequential.h"
#include "search_delete.h"

char sample_descr[] = "Sample 'perf_struct' demonstrates eXtremeDB performance for basic operations\n"
                      "using simple and direct structs.\n\n";
char explanation[] =  "\n\tFirst N objects are inserted into a class, a hash index is created\n"
                      "\ton insertion, then used for searches.  Then a tree index is used\n"
                      "\tfor searches and then for a sequential scan.\n"
                      "\tFinally, the hash index is used to search and remove all objects.\n\n";

const char * db_name = "structdb";
const uint4 nObjects  = 1500000;
const uint4 nSearches = 1500000;
const uint4 nInsertsPerTransaction = 1000;
const uint4 nDeletesPerTransaction = 1000;
#define N_CLASSES 2
const char * className[N_CLASSES] = { "classA", "classB" };
const char * structType[N_CLASSES] = { "simple", "direct" };
#define N_STEPS 5
const char * stepFunction[N_STEPS] = { "createObjects",  "hashSearch", "treeSearch" "scan", "searchDelete" };

// Forward declarations
void timeStep( mco_db_h db, int iStep );

#undef DATABASE_SIZE
#undef MEMORY_PAGE_SIZE
#define DATABASE_SIZE        (1100 * 1024 * 1024)
#define MEMORY_PAGE_SIZE      256

int main(int argc, char* argv[])
{
  mco_db_h db = 0;
  sample_memory_t dbmem;
  int i = 0;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);
  printf(explanation);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  CHECK(mco_runtime_start());

  /* Open databases */
  CHECK(sample_open_database( db_name, structdb_get_dictionary(), DATABASE_SIZE, 0, MEMORY_PAGE_SIZE, 0, 1, &dbmem ));

  CHECK(mco_db_connect(db_name, &db));
  for ( i=0; i < N_STEPS; i++)
  {
    timeStep( db, i );
  }

  CHECK(mco_db_disconnect(db));

  CHECK(sample_close_database(db_name, &dbmem));

  CHECK(mco_runtime_stop());

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

// Set-up and time each class type for one of the steps
void timeStep( mco_db_h db, int iStep )
{
  int i = 0;
  time_t start_time;

  // Print the step description
  switch ( iStep )
  {
    case 0:
      printf("Insert %ld objects...\n", nObjects );
      break;

    case 1:
      printf("Perform %ld Hash searches ...\n", nSearches );
      break;

    case 2:
      printf("Perform %ld Tree searches ...\n", nSearches );
      break;

    case 3:
      printf("Sequential scan ...\n" );
      break;

    case 4:
      printf("Search and Delete all objects one by one...\n");
      break;
  }

  // For each class type, print the class type and time the step
  for (i = 0; i < N_CLASSES; i++)
  {
    printf("\t%s (%s structs) ...", className[i], structType[i] );
    printf("%s", ( 8 < strlen( structType[i] ) ? "\t" : "\t\t" ) );
    start_time = sample_msec();
    switch ( iStep )
    {
      case 0:
        createObjects( db, i, nObjects, nInsertsPerTransaction );
        break;

      case 1:
        hashSearch( db, i, nSearches );
        break;

      case 2:
        treeSearch( db, i, nSearches );
        break;

      case 3:
        sequentialScan( db, i );
        break;

      case 4:
        searchDelete( db, i, nObjects, nDeletesPerTransaction );
        break;

      default:
        CHECK(MCO_E_ILLEGAL_PARAM);
        break;
    }  
    printf("%d milliseconds\n", (int)(sample_msec() - start_time) );
  }
}
