/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include <backupdb.h>

char sample_descr[]  = "Sample '18-backup-migrate' demonstrates migration of an existing disk\n"
                       "database to a more recent eXtremeDB build or/and to a different schema.\n"
                       "To migrate to a new build of eXtremeDB this sample must be compiled\n"
                       "twice. Once the old build of eXtremeDB and once with the new build.\n"
                       "The executable of the sample from the old build should be used for\n"
                       "the backup procedure, the executable of the sample from the new\n"
                       "build for the restore. And the schema for this sample should be\n"
                       "replaced with desired schema.\n"
                       "To migrate to a new schema, the executable compiled with old schema\n"
                       "should be used for the backup and executable compiled with the new\n"
                       "schema for the restore.\n";
const char * db_name = "migratedb";
const char * usage   = "-backup|-restore [-dbfile filename] [-logfile filename] [-logtype no|redo|undo] "
                       "[-backupfile filename] [-memsize Megabytes] [-cachesize Megabytes] [-mempages bytes] "
                       "[-diskpages bytes] [-crc]";

#define MODE_BACKUP     1
#define MODE_RESTORE    2

int mode                    = 0;

char db_file_default[]      = "backupdb.dbs";
char db_log_default[]       = "backupdb.log";
char backup_file_default[]  = "backupdb.bak";

char *db_file               = db_file_default;
char *db_log                = db_log_default;

char *backup_file           = backup_file_default;

uint2 mem_page_size         = 256;
uint4 disk_page_size        = 4096;
MCO_LOG_TYPE db_log_type    = REDO_LOG;

mco_size_t memory_size      = 64; /* Megabytes */
mco_size_t cache_size       = 64; /* Megabytes */

/* Stream writer with prototype mco_stream_write */
mco_size_sig_t file_writer( void *stream_handle /* FILE *  */, const void *from, mco_size_t nbytes )
{
  FILE *f = (FILE *)stream_handle;
  mco_size_sig_t nbs;

  nbs = fwrite( from, 1, nbytes, f );
  return nbs;
}

/* Stream reader with prototype mco_stream_read */
mco_size_sig_t file_reader( void *stream_handle /* FILE *  */,  /* OUT */void *to, mco_size_t max_nbytes )
{
  FILE *f = (FILE *)stream_handle;
  mco_size_sig_t nbs;

  nbs = fread( to, 1, max_nbytes, f );
  return nbs;
}

int parse_cmd_line(int argc, char** argv)
{
    int i = 1;

    while (i < argc && *argv[i] == '-')
    {
        if (strcmp(argv[i], "-backup") == 0) {
            mode = MODE_BACKUP;
            i++;
        } else if (strcmp(argv[i], "-restore") == 0) { 
            mode = MODE_RESTORE;
            i++;
        } else if (strcmp(argv[i], "-dbfile") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Argument for option -dbfile was not specified\n");
                return 1;
            }
            db_file = argv[i];
            i++;
        } else if (strcmp(argv[i], "-logfile") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Argument for option -logfile was not specified\n");
                return 1;
            }
            db_log = argv[i];
            i++;
        } else if (strcmp(argv[i], "-logtype") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Argument for option -logtype was not specified\n");
                return 1;
            }
            if (strcmp(argv[i], "no") == 0) {
                db_log_type = NO_LOG;
            } else if (strcmp(argv[i], "redo") == 0) {
                db_log_type = REDO_LOG;
            } else if (strcmp(argv[i], "undo") == 0) {
                db_log_type = UNDO_LOG;
            } else { 
                fprintf(stderr, "Unknown log type %s\n", argv[i]);
                return 1;
            }
            i++;
        } else if (strcmp(argv[i], "-backupfile") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Argument for option -backupfile was not specified\n");
                return 1;
            }
            backup_file = argv[i];
            i++;
        } else if (strcmp(argv[i], "-mempagesz") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Argument for option -mempagesz was not specified\n");
                return 1;
            }
            mem_page_size = (uint2)atoi(argv[i]);
            i++;
        } else if (strcmp(argv[i], "-diskpagesz") == 0) { 
            i++;
            if (i >= argc) {
                fprintf(stderr, "Argument for option -diskpagesz was not specified\n");
                return 1;
            }
            disk_page_size = (uint4)atoi(argv[i]);
            i++;
        } else if (strcmp(argv[i], "-memsize") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Argument for option -memsize was not specified\n");
                return 1;
            }
            memory_size = (mco_size_t)atoi(argv[i]);
            i++;
        } else if (strcmp(argv[i], "-cachesize") == 0) { 
            i++;
            if (i >= argc) {
                fprintf(stderr, "Argument for option -cachesize was not specified\n");
                return 1;
            }
            cache_size = (mco_size_t)atoi(argv[i]);
            i++;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) { 
            return 1;
        } else { 
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            return 1;
        }
    }
    return i < argc ? 1 : 0;
}

MCO_RET open_load_database(sample_memory_t *pdev, FILE *fbak)
{
    MCO_RET            rc;
    mco_runtime_info_t info;
    mco_db_params_t    db_params;

    pdev->n_dev = 1;

    /* Get runtime info */
    mco_get_runtime_info(&info);

    /* Set-up memory device as a plain conventional memory region */
    pdev->dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* Assign the device as a main database memory */
    pdev->dev[0].size       = memory_size;                    /* Set the device size */
    if (info.mco_shm_supported) {
#ifdef _INTEGRITY
        pdev->dev[0].type       = MCO_MEMORY_INT_DESC;        /* set the device as a shared named memory device */
        pdev->dev[0].dev.idesc.handle= 13;                    /* see app_integrate.h */
#else 
        pdev->dev[0].type       = MCO_MEMORY_NAMED;           /* Set the device as a shared named memory device */
        sprintf( pdev->dev[0].dev.named.name, "%s-db", db_name ); /* Set memory name */
        pdev->dev[0].dev.named.flags = 0;                     /* Zero flags */
        pdev->dev[0].dev.named.hint  = (info.direct_pointers_supported) ? DATABASE_MAP_ADDRESS : 0; /* Set mapping address or null it */
#endif
    } else {
        pdev->dev[0].type       = MCO_MEMORY_CONV;            /* Set the device as a conventional memory device */
        pdev->dev[0].dev.conv.ptr = (void*)malloc( memory_size );   /* Allocate memory and set device pointer */
    }

    /* Set up memory device as a conventional memory region for cache */
    pdev->dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;        /* Assign the device as a cache */
    pdev->dev[1].size       = cache_size;
    if (info.mco_shm_supported) {
#ifdef _INTEGRITY
        pdev->dev[1].type       = MCO_MEMORY_INT_DESC;        /* set the device as a shared named memory device */
        pdev->dev[1].dev.idesc.handle= 19;                    /* see app_integrate.h */
#else 
        pdev->dev[1].type       = MCO_MEMORY_NAMED;           /* Set the device as a shared named memory device */
        sprintf( pdev->dev[1].dev.named.name, "%s-cache", db_name ); /* Set memory name */
        pdev->dev[1].dev.named.flags = 0;                     /* Zero flags */
        pdev->dev[1].dev.named.hint  = 0;                     /* Set mapping address or null it */
#endif
    } else {
        pdev->dev[1].type       = MCO_MEMORY_CONV;            /* Set the device as a conventional memory device */
        pdev->dev[1].dev.conv.ptr = (void*)malloc( cache_size );/* Allocate memory and set device pointer */
    }
        
    /* Set-up memory device for main database storage */
    pdev->dev[2].type       = MCO_MEMORY_FILE;                /* Set the device as a file device */
    pdev->dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;   /* Assign the device as a main database persistent storage */
    sprintf( pdev->dev[2].dev.file.name, "%s", db_file );     /* Name the device */
    pdev->dev[2].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* Set the device flags */
    
    /* Set-up memory device for database log */
    pdev->dev[3].type       = MCO_MEMORY_FILE;                /* Set the device as a file device */
    pdev->dev[3].assignment = MCO_MEMORY_ASSIGN_LOG;          /* Assign the device as a log */
    sprintf( pdev->dev[3].dev.file.name, "%s", db_log );      /* Name the device */
    pdev->dev[3].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* Set the device flags */

    pdev->n_dev            += 3;

    /* Initialize and customize the database parameters */
    mco_db_params_init ( &db_params );              /* Initialize the params with default values */

    db_params.mem_page_size      = mem_page_size;   /* Set page size for in-memory part */
    db_params.disk_page_size     = disk_page_size;  /* Set page size for persistent storage */
    db_params.db_max_connections = 10;    /* Set total number of connections to the database */
    db_params.db_log_type        = db_log_type;
#ifdef EXTREMEDB_LICENSE_KEY
    db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

    if (fbak) {
        /* Open a database on the device with given params */
        rc = mco_db_load((void *)fbak, file_reader, db_name, backupdb_get_dictionary(), pdev->dev, pdev->n_dev, &db_params );
    } else {
        /* Open a database on the device with given params */
        rc = mco_db_open_dev(db_name, backupdb_get_dictionary(), pdev->dev, pdev->n_dev, &db_params );
    }

    if ( rc != MCO_S_OK && !info.mco_shm_supported ) {
        free( pdev->dev[0].dev.conv.ptr );
        free( pdev->dev[1].dev.conv.ptr );
        pdev->dev[0].assignment = MCO_MEMORY_NULL;
        pdev->dev[1].assignment = MCO_MEMORY_NULL;
        pdev->dev[2].assignment = MCO_MEMORY_NULL;
        pdev->dev[3].assignment = MCO_MEMORY_NULL;
    }

    return rc;
}

int play_backup(void)
{
  mco_db_h          con;
  FILE             *fbak;
  sample_memory_t   mem;

  fbak = fopen(backup_file, "wb");
  if (!fbak)
  {
      printf("Can't open backup file %s for writing!", backup_file);
      return 1;
  }

  printf("Open and connect to disk database..\n");
  CHECK(open_load_database(&mem, 0));
  CHECK(mco_db_connect(db_name, &con));

  printf("Save disk database backup to backup file using BSE..\n");
  CHECK(mco_db_save((void *)fbak, file_writer, con));

  fclose(fbak);

  printf("Disconnect and close disk database..\n");
  CHECK(mco_db_disconnect(con));
  CHECK(sample_close_database(db_name, &mem));

  return 0;
}

int play_restore(void)
{
  mco_db_h          con;
  FILE             *fbak;
  sample_memory_t   mem;

  fbak = fopen(backup_file, "rb");
  if (!fbak)
  {
      printf("Can't open backup file %s for reading!", backup_file);
      return 1;
  }

  printf("Load database from backup file into disk database..\n");
  CHECK(open_load_database(&mem, fbak));

  fclose(fbak);

  printf("Close freshly restored disk database..\n");
  CHECK(sample_close_database(db_name, &mem));

  return 0;
}

int  main(int argc, char** argv) 
{
  int ret;

  if (parse_cmd_line(argc, argv) || mode == 0)
  {
    printf("Usage: 18-backup-migrate %s", usage);
    return 1;
  }

  sample_os_initialize(DEFAULT|DISK);

  /* Start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  CHECK(mco_runtime_start());

  sample_header(sample_descr);

  sample_show_runtime_info("The database runtime has the following characteristics:\n" );

  printf("\nProgram options:\n\n\tMode:           %s\n\tDB file:        %s\n\tLog file:       %s\n"
         "\tLog type:       %s\n\tBackup file:    %s\n\tMemory size:    %d Megabytes\n"
         "\tCache size:     %d Megabytes\n\tMem. page size: %d bytes\n\tDisk page size: %d bytes\n\n",
         mode == MODE_BACKUP ? "backup" : "restore", db_file, db_log_type == NO_LOG ? "none" : db_log,
         db_log_type == NO_LOG ? "no" : (db_log_type == REDO_LOG ? "redo" : "undo"),
         backup_file, (int)memory_size, (int)cache_size, (int)mem_page_size, (int)disk_page_size);

  memory_size *= 1024*1024;
  cache_size *= 1024*1024;

  if (mode == MODE_BACKUP)
      ret = play_backup();
  else
      ret = play_restore();

  CHECK(mco_runtime_stop());

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ret;
}
