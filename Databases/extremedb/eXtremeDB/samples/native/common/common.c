/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include "common.h"
#include "mcotime.h"

MCO_RET sample_open_database( 
           const char * db_name,  /* name of the database */
           mco_dictionary_h dict, /* pointer to schema */
           mco_size_t db_sz,      /* size of memory segment for in-mem part of the db */
           mco_size_t cache_sz,   /* size of cache segment for persistent storage */
           uint2 mem_pg_sz,       /* size of memory page */
           uint2 pstorage_pg_sz,  /* size of persistent storage page */
           uint2 max_conn_no,     /* max. number of connections */
           sample_memory_t * pdev)
{
    return sample_load_database(0, 0, db_name, dict, db_sz, cache_sz, mem_pg_sz, pstorage_pg_sz, max_conn_no, pdev);
}

MCO_RET sample_load_database( 
           void* stream_handle,   /* database input stream handle */
           mco_stream_read input_stream_reader, /* database input stream reader */
           const char * db_name,  /* name of the database */
           mco_dictionary_h dict, /* pointer to schema */
           mco_size_t db_sz,      /* size of memory segment for in-mem part of the db */
           mco_size_t cache_sz,   /* size of cache segment for persistent storage */
           uint2 mem_pg_sz,       /* size of memory page */
           uint2 pstorage_pg_sz,  /* size of persistent storage page */
           uint2 max_conn_no,     /* max. number of connections */
           sample_memory_t * pdev)
{   
    MCO_RET            rc;
    mco_runtime_info_t info;
    mco_db_params_t    db_params;

    pdev->n_dev = 1;
    
    /* get runtime info */
    mco_get_runtime_info(&info);
   
    /* setup memory device as a plain conventional memory region */
    pdev->dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* assign the device as a main database memory */
    pdev->dev[0].size       = db_sz;                          /* set the device size */
    if (info.mco_shm_supported) {
#ifdef _INTEGRITY
        pdev->dev[0].type       = MCO_MEMORY_INT_DESC;        /* set the device as a shared named memory device */
        pdev->dev[0].dev.idesc.handle= 13;                    /* see app_integrate.h */
#else 
        pdev->dev[0].type       = MCO_MEMORY_NAMED;           /* set the device as a shared named memory device */
        sprintf( pdev->dev[0].dev.named.name, "%s-db", db_name ); /* set memory name */
        pdev->dev[0].dev.named.flags = 0;                     /* zero flags */
        pdev->dev[0].dev.named.hint  = (info.direct_pointers_supported) ? DATABASE_MAP_ADDRESS : 0; /* set mapping address or null it */
#endif
    } else {
        pdev->dev[0].type       = MCO_MEMORY_CONV;            /* set the device as a conventional memory device */
        pdev->dev[0].dev.conv.ptr = (void*)malloc( db_sz );   /* allocate memory and set device pointer */
    }

    if (info.mco_disk_supported) {
        /* setup memory device as a convention memory region for cache */
        pdev->dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;        /* assign the device as a cache */
        pdev->dev[1].size       = cache_sz;
        if (info.mco_shm_supported) {
#ifdef _INTEGRITY
            pdev->dev[1].type       = MCO_MEMORY_INT_DESC;        /* set the device as a shared named memory device */
            pdev->dev[1].dev.idesc.handle= 19;                    /* see app_integrate.h */
#else 
            pdev->dev[1].type       = MCO_MEMORY_NAMED;           /* set the device as a shared named memory device */
            sprintf( pdev->dev[1].dev.named.name, "%s-cache", db_name ); /* set memory name */
            pdev->dev[1].dev.named.flags = 0;                     /* zero flags */
            pdev->dev[1].dev.named.hint  = 0;                     /* set mapping address or null it */
#endif
        } else {
            pdev->dev[1].type       = MCO_MEMORY_CONV;            /* set the device as a conventional memory device */
            pdev->dev[1].dev.conv.ptr = (void*)malloc( cache_sz );/* allocate memory and set device pointer */
        }
        
        /* setup memory device for main database storage */
        pdev->dev[2].type       = MCO_MEMORY_FILE;                /* set the device as a file device */
        pdev->dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;   /* assign the device as a main database persistent storage */
        sprintf( pdev->dev[2].dev.file.name, FILE_PREFIX "%s.dbs", db_name ); /* name the device */
        pdev->dev[2].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* set the device flags */
    
        /* setup memory device for database log */
        pdev->dev[3].type       = MCO_MEMORY_FILE;                /* set the device as a file device */
        pdev->dev[3].assignment = MCO_MEMORY_ASSIGN_LOG;          /* assign the device as a log */
        sprintf( pdev->dev[3].dev.file.name, FILE_PREFIX "%s.log", db_name ); /* name the device */
        pdev->dev[3].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* set the device flags */
    
        /*unlink(pdev->dev[2].dev.file.name);*/
        /*unlink(pdev->dev[3].dev.file.name);*/

        pdev->n_dev            += 3;

        if (pstorage_pg_sz == 0) { 
            pstorage_pg_sz = 4096;
        }
    } else {
        pdev->dev[1].assignment = MCO_MEMORY_NULL;
        pdev->dev[2].assignment = MCO_MEMORY_NULL;
        pdev->dev[3].assignment = MCO_MEMORY_NULL;
    }
    
    /* initialize and customize the database parameters */
    mco_db_params_init ( &db_params );             /* initialize the params with default values */

    db_params.mem_page_size      = mem_pg_sz;      /* set page size for in-memory part */
    db_params.disk_page_size     = pstorage_pg_sz; /* set page size for persistent storage */
    db_params.db_max_connections = max_conn_no;    /* set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
    db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
    
    if ( stream_handle )
    {
#ifdef MCO_HAS_SERIALIZATION
        /* open a database on the device with given params and load stream */
        rc = mco_db_load(stream_handle, input_stream_reader, db_name, dict, pdev->dev, pdev->n_dev, &db_params );
#else
        rc = MCO_E_UNSUPPORTED;
#endif
    }
    else
    {
        /* open a database on the device with given params */
        rc = mco_db_open_dev(db_name, dict, pdev->dev, pdev->n_dev, &db_params );
    }
    if ( rc != MCO_S_OK && !info.mco_shm_supported ) {
        free( pdev->dev[0].dev.conv.ptr );
        if (info.mco_disk_supported) {
            free( pdev->dev[1].dev.conv.ptr );
        }
        pdev->dev[0].assignment = MCO_MEMORY_NULL;
        pdev->dev[1].assignment = MCO_MEMORY_NULL;
        pdev->dev[2].assignment = MCO_MEMORY_NULL;
        pdev->dev[3].assignment = MCO_MEMORY_NULL;
    }
    
    return rc;
}

MCO_RET sample_close_database( const char * db_name, sample_memory_t * pdev ) {
  unsigned int i;
  MCO_RET rc;

  rc = mco_db_close( db_name );

  for ( i=0; i<pdev->n_dev; i++ ) {
      if (pdev->dev[i].type == MCO_MEMORY_CONV ) {
          free( pdev->dev[i].dev.conv.ptr );
      }
  }
  return rc;
}

const char * mco_ret_string( MCO_RET rc, int * is_error ) {
  if ( is_error ) {
    (*is_error) = rc >= MCO_E_TRANS_NOT_ACTIVE;
  }

  return mco_strerror( rc );
}

/* sample task control */
unsigned int sample_start_task( sample_task_t * descr, sample_task_proc_t task, void * param ) {
    
    return sample_start_connected_task( descr, task, 0, param );
}

void start_thread_task (void * v)
{
    sample_task_t * descr = (sample_task_t *)v;

    descr->stopped       = 0;

    if ( descr->db_name != 0 )
    {
        /* connect to the db */
        if ( MCO_S_OK != mco_db_connect( descr->db_name, &descr->db_connection ) )
        {
            descr->stopped       = 2;
            descr->db_connection = 0;
            return;
        }
    } else {
        descr->db_connection = 0;
    }

    descr->proc( descr );

    /* disconnect from the db */
    if ( descr->db_connection != 0 ) {
        mco_db_disconnect( descr->db_connection );
        descr->db_connection = 0;
    }
}

unsigned int sample_start_connected_task( sample_task_t * descr, sample_task_proc_t task, const char * db_name, void * param ) {
    memset( descr, 0, sizeof(*descr));
    descr->param         = param;
    descr->proc          = task;
    descr->db_name       = db_name;
    descr->t = mco_create_thread(start_thread_task, (void*)descr);
    return descr->t ? 1 : 0;
}

unsigned int sample_stop_task ( sample_task_t * descr ) {
    descr->stopped = 1;
    return 0;
}

unsigned int sample_join_task ( sample_task_t * descr ) {
    descr->t->join(descr->t);
    return 0;
}

/* eXtremeDB error handler */
void sample_errhandler(MCO_RET n)
{
  printf("\n\neXtremeDB runtime fatal error: %d \n\t(%s)\n", n, mco_ret_string(n, 0));
  getchar();
  dbg_exit(1);
}

void sample_pause(const char * prompt)
{
  if ( *prompt )
    printf("%s", prompt);
  getchar();

}

int sample_is_release()
{
  mco_runtime_info_t info;
  
  mco_get_runtime_info(&info);
  return ( 0 == info.mco_checklevel );
}

void sample_pause_end(const char * prompt)
{
  /* Windows console applications in release mode issue the prompt 
  "Press any key to continue . . ." before closing the command window.
  The following block simulates this behavior when in debug mode.
  */
  #ifdef _WIN32
    if ( sample_is_release() )
      printf("\n\n");
    else
      sample_pause(prompt);
  #endif
}

void sample_header(const char * descr)
{
  mco_runtime_info_t info;
  char copyr[] = "Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.";
  char mode[64];
  char onDisk[] = "Fusion";

  /* get runtime info */
  mco_get_runtime_info(&info);

  if ( 0 == info.mco_disk_supported ) onDisk[0] = 0;

  if ( 0 == info.mco_checklevel )
    sprintf( mode, "%s%s", onDisk, " Release" );
  else
    sprintf( mode, "%s%s", onDisk, " Debug" );

  printf("%s\neXtremeDB runtime version %d.%d, %s build %d\n%s\n\n", descr, info.mco_version_major,
         info.mco_version_minor, mode, info.mco_build_number, copyr);
}

void sample_rc_check(const char * msg, MCO_RET rc)
{
  if ( MCO_S_OK == rc )
    printf("\n%s : Successful", msg );
  else
    printf("\n%s : \n\tReturn Code %d: %s\n", msg, (unsigned int)rc, mco_ret_string(rc, 0));
}

void sample_show_runtime_info(const char * lead_line)
{
  mco_runtime_info_t info;
  
  /* get runtime info */
  mco_get_runtime_info(&info);

  /* Core configuration parameters: */
  if ( *lead_line )
    printf( "%s", lead_line );
  printf("\n");
  printf( "\tEvaluation runtime ______ : %s\n", info.mco_evaluation_version   ? "yes":"no" );
  printf( "\tCheck-level _____________ : %d\n", info.mco_checklevel );
  printf( "\tMultithread support _____ : %s\n", info.mco_multithreaded        ? "yes":"no" );
  printf( "\tFixedrec support ________ : %s\n", info.mco_fixedrec_supported   ? "yes":"no" );
  printf( "\tShared memory support ___ : %s\n", info.mco_shm_supported        ? "yes":"no" );
  printf( "\tXML support _____________ : %s\n", info.mco_xml_supported        ? "yes":"no" );
  printf( "\tStatistics support ______ : %s\n", info.mco_stat_supported       ? "yes":"no" );
  printf( "\tEvents support __________ : %s\n", info.mco_events_supported     ? "yes":"no" );
  printf( "\tVersioning support ______ : %s\n", info.mco_versioning_supported ? "yes":"no" );
  printf( "\tSave/Load support _______ : %s\n", info.mco_save_load_supported  ? "yes":"no" );
  printf( "\tRecovery support ________ : %s\n", info.mco_recovery_supported   ? "yes":"no" );
  printf( "\tRTree index support _____ : %s\n", info.mco_rtree_supported      ? "yes":"no" );
  printf( "\tUnicode support _________ : %s\n", info.mco_unicode_supported    ? "yes":"no" );
  printf( "\tWChar support ___________ : %s\n", info.mco_wchar_supported      ? "yes":"no" );
  printf( "\tC runtime _______________ : %s\n", info.mco_rtl_supported        ? "yes":"no" );
  printf( "\tSQL support _____________ : %s\n", info.mco_sql_supported        ? "yes":"no" );
  printf( "\tPersistent storage support: %s\n", info.mco_disk_supported       ? "yes":"no" );
  printf( "\tDirect pointers mode ____ : %s\n", info.mco_direct_pointers      ? "yes":"no" );  
}

void sample_show_device_info(const char * lead_line, mco_device_t dev[], int nDev)
{
  const char * memtype[]    = { "MCO_MEMORY_NULL", "MCO_MEMORY_CONV", "MCO_MEMORY_NAMED", "MCO_MEMORY_FILE", 
                          "MCO_MEMORY_MULTIFILE", "MCO_MEMORY_RAID", "MCO_MEMORY_INT_DESC" };
  const char * assigntype[] = { "MCO_MEMORY_ASSIGN_DATABASE", "MCO_MEMORY_ASSIGN_CACHE", 
                          "MCO_MEMORY_ASSIGN_PERSISTENT", "MCO_MEMORY_ASSIGN_LOG" };
  const char * flagtype[]   = { "MCO_FILE_OPEN_DEFAULT", "MCO_FILE_OPEN_READ_ONLY", "MCO_FILE_OPEN_TRUNCATE",
                          "MCO_FILE_OPEN_NO_BUFFERING", "MCO_FILE_OPEN_EXISTING" };
  int i, nflags;

  if ( *lead_line )
    printf("%s", lead_line );
  printf("\n");
  for ( i=0; i < nDev; i++ ) {
    printf("\n\tdev[%d]:\tMemory type: %s\n", i, memtype[dev[i].type] );
    printf("\t\tAssignment : %s\n", assigntype[dev[i].assignment] );
    if ( MCO_MEMORY_FILE == dev[i].type || MCO_MEMORY_MULTIFILE == dev[i].type || MCO_MEMORY_RAID == dev[i].type ) 
       printf("\t\tFilename   : %s\n", dev[i].dev.file.name );
    else
       printf("\t\tSize       : %llu\n", (uint8)dev[i].size );
    printf("\t\tFlags      : " );
    if ( MCO_FILE_OPEN_DEFAULT == ( dev[i].dev.file.flags & MCO_FILE_OPEN_DEFAULT ) )
    {
      printf("%s", flagtype[0] );
    } else {
      nflags = 0;
      if ( MCO_FILE_OPEN_READ_ONLY == ( dev[i].dev.file.flags & MCO_FILE_OPEN_READ_ONLY ) )
      {
        printf("%s", flagtype[1] );
        nflags++;
      }
      if ( MCO_FILE_OPEN_TRUNCATE == ( dev[i].dev.file.flags & MCO_FILE_OPEN_TRUNCATE ) )
      {
        if ( nflags > 0 ) printf(" | ");
        printf("%s", flagtype[2] );
        nflags++;
      }
      if ( MCO_FILE_OPEN_NO_BUFFERING == ( dev[i].dev.file.flags & MCO_FILE_OPEN_NO_BUFFERING ) )
      {
        if ( nflags > 0 ) printf(" | ");
        printf("%s", flagtype[3] );
        nflags++;
      }
      if ( MCO_FILE_OPEN_EXISTING == ( dev[i].dev.file.flags & MCO_FILE_OPEN_EXISTING ) )
      {
        if ( nflags > 0 ) printf(" | ");
        printf("%s", flagtype[4] );
        nflags++;
      }
    }
    printf("\n" );
  }
}

int sample_msec() {
    return (int)MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
}

int sample_disk_supported()
{
  mco_runtime_info_t info;
  
  mco_get_runtime_info(&info);
  return info.mco_disk_supported;
}

void sample_sleep( unsigned int duration_msec )
{
    sleep_msec(duration_msec);
}

SAMPLE_OS_TASK_ID sample_os_task_id_get()
{
    return mco_get_pid();
}

int sample_os_task_id_check( SAMPLE_OS_TASK_ID tid )
{
    return mco_is_process_alive(tid) == MCO_YES ? 0 : 1;
}

#ifdef _NETOS
int strcasecmp(const char *s1, const char *s2)
{
    char c1, c2;
    if ( s1 == 0 && s2 == 0) return 0;
    if ( s1 == 0 ) return 1;
    if ( s2 == 0 ) return -1;

    do {
        c1 = *s1; c2 = *s2;

        if ( c1 >= 'A' && c1 <= 'Z' )
                c1 += 'a' - 'A';

        if ( c2 >= 'A' && c2 <= 'Z' )
                c2 += 'a' - 'A';

    s1++; s2++;

    } while ( c1 == c2  && c1 != 0);

    return c1 < c2 ? -1 : ( c1 == c2 ? 0 : 1);
}
#endif

void dbg_exit (int ret)
{
    exit (ret);
}
