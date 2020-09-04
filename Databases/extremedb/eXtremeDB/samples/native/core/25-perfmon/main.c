/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.    *
 *                                                              *
 ****************************************************************/

#include <mco.h>
#include <mcoperfmon.h>
#include <mcouda.h>
#include <common.h>
#include "perf.h"

#define METADICT_ENTRY_NUM      2


#ifdef DATABASE_SIZE
#undef DATABASE_SIZE
#endif

#define DATABASE_SIZE (128<<20)
#define  PERF_DATABASE_SEGMENT_SIZE ( 64<<20 )

void perform_ops(sample_task_t * descriptor);

char sample_descr[] = "Sample '25-perfmon' demonstrates using perfmormance monitor API.\n";
const char *dbName  = "hvddb";

mco_metadict_header_t *metadict;

static void errhandler(MCO_RET n)
{
  printf("\neXtremeDB fatal error: %d", n);
  getchar();
  dbg_exit( - 1);
}

uint4 exit_flag = 0;

MCO_RET init_metadict()
{
  unsigned int size;
  mco_metadict_size(2, &size);
  metadict = (mco_metadict_header_t *) malloc(size);
  return mco_metadict_init (metadict, size, 0);
}

unsigned short get_field_no(unsigned short dict_no, mco_uda_object_handle_t *obj, const char *field_name)
{
  mco_dict_field_info_t field_info;
  CHECK(mco_dict_field_name(metadict, dict_no, obj->struct_no, field_name, &field_info));

  return field_info.field_no;
}

unsigned short get_struct_no(unsigned short dict_no, const char *struct_name)
{
    MCO_RET rc;
  mco_dict_struct_info_t struct_info;
  rc = mco_dict_struct_name(metadict, dict_no, struct_name, &struct_info);
  printf ("Struct %s -> %d, rc = %d\n", struct_name, struct_info.struct_no, rc);
  return struct_info.struct_no;
}

unsigned short get_index_no(unsigned short dict_no, const char *struct_name, const char *index_name)
{
    MCO_RET rc;
  mco_dict_index_info_t index_info;
  rc = mco_dict_index_name(metadict, dict_no, get_struct_no(dict_no, struct_name), index_name, &index_info);
  printf ("Index %s.%s -> %d, rc=%d\n", struct_name, index_name, index_info.index_no, rc);
  return index_info.index_no;
}

void do_perfmon_queries(int n_queries)
{
    MCO_RET rc;
    const char *dbname = mco_perfmon_get_db_name();
    mco_db_h db;
    mco_trans_h t;
    mco_cursor_t csr;
    mco_uda_object_handle_t rec;
    mco_uda_value_t value;
    mco_metadict_entry_t *entry;
    int i;

    init_metadict();

    // Find perfmon database index (should be 1 as it initialized after main one */

    rc = mco_metadict_entry_name(metadict, mco_perfmon_get_db_name(), &entry);

    if (rc != MCO_S_OK) {
        printf("Could not find metadict entry: %d (%s)\n", rc, mco_strerror(rc));
        exit(1);
    }

    // Connect to perfmon database
    rc = mco_db_connect(dbname, &db);

    /* Sequentially read and output records */
    printf("\n\n Read perfmon records...\n");
    rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
    if ( MCO_S_OK != rc ) {
        printf("Could not start transaction\n");
    }

    /* Open cursor for counter list. It's not changed during application lifetime*/
    rc = mco_uda_cursor(t, get_struct_no(entry->dict_no, "PerfCounter"), get_index_no(entry->dict_no, "PerfCounter", "no_idx"), &csr);

    if (rc != MCO_S_OK) {
        printf("Could not open UDA cursor: rc=%d (%s)\n", rc, mco_strerror(rc));
        exit(-1);
    }

    for (rc = mco_cursor_first(t, &csr); rc == MCO_S_OK; rc = mco_cursor_next(t, &csr)) {
		uint1 dtype;
        char name[128];
        char descr[128];

		rc = mco_uda_from_cursor(t, &csr, &rec); /* Get object handle from cursor */
        value.type = MCO_DD_UINT2;
        rc = mco_uda_get(&rec, get_field_no(entry->dict_no, &rec, "no"), 0, &value);

        value.type = MCO_DD_STRING;
        value.v.p.p.c = name;
        value.v.p.size = sizeof(name);
        rc = mco_uda_get(&rec, get_field_no(entry->dict_no, &rec, "name"), 0, &value);

        value.type = MCO_DD_STRING;
        value.v.p.p.c = descr;
        value.v.p.size = sizeof(descr);
        rc = mco_uda_get(&rec, get_field_no(entry->dict_no, &rec, "descr"), 0, &value);

        value.type = MCO_DD_UINT1;
        rc = mco_uda_get(&rec, get_field_no(entry->dict_no, &rec, "datatype"), 0, &value);
        dtype = value.v.u1;

        printf ("  %s - %s type %d\n", name, descr, dtype);
    }

    CHECK (mco_trans_commit(t));
    
    for (i = 0; i < n_queries; i++) {

		sleep_msec(2000);

        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK != rc ) {
          printf("Could not start transaction\n");
        }
        
        /* Open scalar data readings */
        rc = mco_uda_cursor(t,
                            get_struct_no(entry->dict_no, "PerfReading"),
                            get_index_no(entry->dict_no, "PerfReading", "stamp_idx"), &csr);
        if (rc != MCO_S_OK) {
            printf("Could not open UDA data cursor: rc=%d (%s)\n", rc, mco_strerror(rc));
            i -= 1;
            continue;	    
        }

        
        for (rc = mco_cursor_first(t, &csr); rc == MCO_S_OK; rc = mco_cursor_next(t, &csr)) {
			uint8 stamp;
            rc = mco_uda_from_cursor(t, &csr, &rec);

            value.type = MCO_DD_UINT8;
            rc = mco_uda_get(&rec, get_field_no(entry->dict_no, &rec, "stamp"), 0, &value);
            stamp = value.v.u8;

            // Take MCO_PERF_CONNECTION_COUNT value

            value.type = MCO_DD_UINT4;     /* Set field's type */ 
            mco_uda_get(&rec, get_field_no(entry->dict_no, &rec, "values"), MCO_PERF_CONNECTION_COUNT, &value); /* Put value to object */

            printf ("Stamp: %lld, Connection count: %d\n", stamp, value.v.u4);
        }

        /* Open timing */
        rc = mco_uda_cursor(t,
                            get_struct_no(entry->dict_no, "PerfTimingReading"),
                            get_index_no(entry->dict_no, "PerfTimingReading", "stamp_idx"), &csr);
        if (rc != MCO_S_OK) {
            printf("Could not open UDA data cursor: rc=%d (%s)\n", rc, mco_strerror(rc));
            i -= 1;
            continue;	    
        }

        for (rc = mco_cursor_first(t, &csr); rc == MCO_S_OK; rc = mco_cursor_next(t, &csr)) {
			uint8 stamp;
            rc = mco_uda_from_cursor(t, &csr, &rec);

            value.type = MCO_DD_UINT8;
            rc = mco_uda_get(&rec, get_field_no(entry->dict_no, &rec, "stamp"), 0, &value);
            stamp = value.v.u8;

            // Take MCO_PERF_CONNECTION_COUNT value

            value.type = MCO_DD_UINT8;     /* Set field's type */ 
            CHECK(mco_uda_get(&rec, get_field_no(entry->dict_no, &rec, "avg"), MCO_PERF_COMMIT_TIME-MCO_PERF_TIMERS_START, &value)); /* Put value to object */

            printf ("Stamp: %lld, Average commit time: %lld\n", stamp, value.v.u8);
        }
        
        CHECK (mco_trans_commit(t));

    }

    rc = mco_db_disconnect(db);
}


int main(int argc, char** argv)
{
  MCO_RET rc;
  mco_runtime_info_t info;
  mco_db_h db;
  sample_memory_t sample_dev;
  mco_db_params_t db_params;
  void *perf_memory;
  mco_metadict_header_t *metadict;  /* Meta-dictionary header. Application must allocate memory for it. */
  unsigned int	metadict_size;
  sample_task_t worker_task;

  sample_os_initialize(DEFAULT);

  mco_get_runtime_info(&info);
  if (info.evaluation_version)
  {
    /* Signal worker thread to stop operations and exit */
    exit_flag = 2;
  }

  sample_header(sample_descr);

  /* Set fatal error handler */
  mco_error_set_handler(&errhandler);

  /* Start db engine */
  if (mco_runtime_start() != MCO_S_OK)
  {
    printf("\nUnable to start database engine\n");
    getchar();
    sample_os_shutdown();
    return(-1);
  }

  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size for persistent storage */

  rc = sample_open_database(dbName, perf_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 10, &sample_dev);
  sample_rc_check("Open database", rc);
  CHECK(rc);
  printf("\n");

  perf_memory = malloc(PERF_DATABASE_SEGMENT_SIZE);
  if (perf_memory == NULL) {
	  printf ("Not enough memory to create performance monitor database\n");
	  dbg_exit(1);
  }

  CHECK( mco_perfmon_init(perf_memory, PERF_DATABASE_SEGMENT_SIZE, NULL));
  CHECK( mco_perfmon_attach(dbName));

  rc = mco_db_connect(dbName, &db);
  if (MCO_S_OK != rc) {
      printf ("\nUnable to connect to database. Error %d (%s).", rc, mco_ret_string(rc, NULL));
      sample_os_shutdown();
      dbg_exit(rc);
  }

  mco_metadict_size(METADICT_ENTRY_NUM, &metadict_size);
  metadict = (mco_metadict_header_t *) (malloc(metadict_size));

  rc = mco_metadict_init (metadict, metadict_size, 0);
  if ( MCO_S_OK != rc ) {
      // Handle this error
      printf ("\nUnable to initialize metadict. Error %d (%s).", rc, mco_ret_string(rc, NULL));
      free(metadict);
      getchar();
      sample_os_shutdown();
      return(rc);
  }

  /* Perform operations on a database */
  sample_start_connected_task(&worker_task, perform_ops, dbName, &exit_flag); /* Start worker thread*/

  /* perform queries for perfmon database */
  
  do_perfmon_queries(10);

  /* Signal worker thread to stop operations and exit */
  exit_flag = 1;

  /* Wait for listener thread */
  sample_join_task(&worker_task);

  /* stop performance monitor */
  CHECK(mco_perfmon_detach(dbName));
  CHECK(mco_perfmon_close());

  free (perf_memory);


  /* Close the database */
  sample_close_database( dbName, &sample_dev);

  /* Shut down database engine */
  mco_runtime_stop();
  sample_os_shutdown();
  return 0;
}
