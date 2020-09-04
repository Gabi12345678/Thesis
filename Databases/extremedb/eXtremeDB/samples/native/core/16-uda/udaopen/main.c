/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
/*
 * This sample demonstrates the use of mco_uda_db_open()/mco_uda_db_close()
 * to create a database and change class persistence at run-time.
 */
#include <common.h>
#include <mcouda.h>
#include "udaopen.h"

char sample_descr[] = {
  "Sample 'udaopen' demonstrates use of mco_uda_db_open() & mco_uda_db_close().\n" 
};
const char* db_name = "udaopen";
mco_metadict_header_t *metadict; /* Meta-dictionary header */

/* Get class code by class name */
unsigned short get_class_code(const char *class_name)
{
  MCO_RET rc;
  mco_dict_struct_info_t struct_info;
  char msg[128];

  /* Find struct info in meta-dictionary */
  rc = mco_dict_struct_name(metadict, 0, class_name, &struct_info);
  if (rc != MCO_S_OK) {
    sprintf(msg, "Error getting info for class '%s'", class_name );
    sample_rc_check(msg, rc );
    return 0;
  }    
  /* Check if the struct is actually a class */
  if ( ! (struct_info.flags & MCO_DICT_SI_CLASS)) return 0;
  return struct_info.class_code;
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_runtime_info_t info;
  mco_device_t dev[4];
  unsigned int n_dev, metadict_size;
  mco_db_params_t db_params;
    
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  mco_get_runtime_info(&info);

  /* Initialize memory device(s) */
  dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* Assign the device as a main database memory */
  dev[0].size       = DATABASE_SIZE;          /* Set the device size */
  if (info.mco_shm_supported) {
    dev[0].type       = MCO_MEMORY_NAMED;           /* Set the device as a shared named memory device */
    sprintf( dev[0].dev.named.name, "%s-db", db_name ); /* Set memory name */
    dev[0].dev.named.flags = 0;                     /* Zero flags */
    dev[0].dev.named.hint  = 0;                     /* Set mapping address or null it */
  } else {
    dev[0].type       = MCO_MEMORY_CONV;            /* Set the device as a conventional memory device */
    dev[0].dev.conv.ptr = (void*)malloc( DATABASE_SIZE ); /* Allocate memory and set device pointer */
  }
  n_dev = 1;

  if (info.mco_disk_supported) {
    /* Setup memory device as a conventional memory region for cache */
    dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;        /* Assign the device as a cache */
    dev[1].size       = CACHE_SIZE;
    if (info.mco_shm_supported) {
      dev[1].type       = MCO_MEMORY_NAMED;           /* Set the device as a shared named memory device */
      sprintf( dev[1].dev.named.name, "%s-cache", db_name ); /* Set memory name */
      dev[1].dev.named.flags = 0;                     /* Zero flags */
      dev[1].dev.named.hint  = 0;                     /* Set mapping address or null it */
    } else {
      dev[1].type       = MCO_MEMORY_CONV;            /* Set the device as a conventional memory device */
      dev[1].dev.conv.ptr = (void*)malloc( CACHE_SIZE ); /* Allocate memory and set device pointer */
    }
    
    /* Setup memory device for main database storage */
    dev[2].type       = MCO_MEMORY_FILE;                /* Set the device as a file device */
    dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;   /* Assign the device as a main database persistent storage */
    sprintf( dev[2].dev.file.name, FILE_PREFIX "%s.dbs", db_name ); /* Name the device */
    dev[2].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* Set the device flags */

    /* Set up memory device for database log */
    dev[3].type       = MCO_MEMORY_FILE;                /* Set the device as a file device */
    dev[3].assignment = MCO_MEMORY_ASSIGN_LOG;          /* Assign the device as a log */
    sprintf( dev[3].dev.file.name, FILE_PREFIX "%s.log", db_name ); /* Name the device */
    dev[3].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* Set the device flags */

    n_dev            += 3;
  }
    
  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for in-memory part */
  db_params.disk_page_size     = (info.mco_disk_supported) ? PSTORAGE_PAGE_SIZE : 0;   /* Set page size for persistent storage */
  db_params.db_max_connections = 1;                /* Set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

  /* Initialize meta-dictionary for 1 entry */
  mco_metadict_size(1, &metadict_size);
  metadict = (mco_metadict_header_t *) malloc(metadict_size);
  mco_metadict_init (metadict, metadict_size, 0);

  /* Register dictionary */
  rc = mco_metadict_register(metadict, db_name, udaopen_get_dictionary(), 0);
  sample_rc_check("\tmco_metadict_register", rc );

  printf("\n\n\tRuntime disk support : %s\n", (info.mco_disk_supported) ? "yes" : "no");    

  /* Open database */
  rc = mco_uda_db_open(metadict,        /* Meta-dictionary header - must be initialized */
                       0,               /* Dictionary number */
                       dev,             /* Memory devices */
                       n_dev,           /* Num of memory devices */
                       &db_params),      /* db parameters */
  sample_rc_check("\tmco_uda_db_open()", rc );
  
  if ( MCO_S_OK == rc ) {
    /* Close the database */
    mco_uda_db_close(metadict, 0);
  }

  /* Stop runtime and free allocated memory */
  mco_runtime_stop();

  if (!info.mco_shm_supported) {
    free( dev[0].dev.conv.ptr );
    if (info.mco_disk_supported) {
      free( dev[1].dev.conv.ptr );
    }
  }

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
