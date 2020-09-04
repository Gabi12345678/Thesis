/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <mcoapic.h>
#include "c_apidb.h"
const char * db_name  = "ctestdb";

#define DISK_CACHE_SIZE (4*1024*1024)
#define DISK_PAGE_SIZE  4096
#ifndef _VXWORKS
#define DATABASE_FILE_PATH "c_apidb.dbs"
#define DATABASE_LOG_PATH  "c_apidb.log"
#else
#define DATABASE_FILE_PATH "/c_apidb.dbs"
#define DATABASE_LOG_PATH  "/c_apidb.log"
#endif

#ifndef DB_LOG_TYPE
    #define DB_LOG_TYPE REDO_LOG
#endif 

#define ALLOC_QUANTUM   1024*1024
#define ALLOC_RETAIN    8*1024*1024 
#define MAX_NAME_LENGTH 64
#define MAX_LINE_LENGTH 72

static void extremedb_error_handler(MCO_RET error_code)
{
    fprintf(stderr, "eXtremeDB error %d\n", error_code);
    sample_os_shutdown();
    dbg_exit(1);
}

static void mcosql_error_handler(status_t status)
{
    fprintf(stderr, "McoSQL error %d\n", status);
    sample_os_shutdown();
    dbg_exit(1);
}

/* Forward declarations */
void perform_transactions(database_t db);
MCO_RET show_persons( database_t db, char* name_pattern );
MCO_RET add_person(database_t db, char const* name, char const* address, char const* position, 
               uint4 salary, float rating, char const* manager);
MCO_RET update_person(database_t db, char const* name, uint4 salary, float rating);
MCO_RET delete_all_persons(database_t db); 
MCO_RET get_person_id( database_t db, char const* name, int64_t* id );
MCO_RET get_persons( database_t db, char const* name_like );
MCO_RET get_person_name(database_t db, char* buf, size_t buf_size, int64_t* id);
MCO_RET get_subordinates( database_t db, char const* manager );
char * s_truncate( char * name );
void print_value(database_t db, int type, void* value);

int main( int argc, char** argv)
{
  MCO_RET           rc;
  mco_db_h          db;
  mco_device_t      dev;
  mco_db_params_t   db_params;
  database_t        engine;

  sample_os_initialize(DEFAULT);

  sample_header("Sample 'transactions' demonstrates basic database transactions\n"
                "using the eXtremeSQL C language API.\n" );
       
  /* set fatal error handler */
  mco_error_set_handler(extremedb_error_handler);

  /* start eXtremeDB runtime */
  mco_runtime_start();
   
  /* setup memory device as a plain conventional memory region */
  dev.type       = MCO_MEMORY_CONV;                /* set the device as a conventional memory device */
  dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* assign the device as main database memory */
  dev.size       = DATABASE_SIZE;                  /* set the device size */
  dev.dev.conv.ptr = (void*)malloc( DATABASE_SIZE );
  if ( !dev.dev.conv.ptr ) {
    extremedb_error_handler(MCO_E_NOMEM);
  }
  
  /* initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* set page size for the in memory part */
  db_params.disk_page_size     = 0;                /* set page size to zero to disable disk operations */
  db_params.db_max_connections = 1;                /* set total number of connections to the database */
  db_params.db_log_type        = DB_LOG_TYPE;      /* set transaction log type */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
    
  /* open a database on the device with given params */
  rc = mco_db_open_dev(db_name, c_apidb_get_dictionary(), &dev, 1, &db_params );
  if ( MCO_S_OK == rc ) {

    /* connect to the database by name */
    rc = mco_db_connect( db_name, &db );
    if ( MCO_S_OK == rc ) {

      rc = (MCO_RET)mcoapi_create_engine(db, &engine);
      if ( MCO_S_OK == rc ) {

        /* Do database processing ... */
        perform_transactions( engine );

        /* Destroy the SQL engine and disconnect from and close database */
        rc = (MCO_RET)mcoapi_destroy_engine( engine );

        /* Close SQL engine failed; disconnect from database... */
        sample_rc_check("\tClose SQL engine and disconnect", rc );
        rc = mco_db_disconnect(db);
        rc = mco_db_close(db_name);
      } else {
        /* Create Engine failed; disconnect from database... */
        sample_rc_check("\tCreate Engine", rc );
        rc = mco_db_disconnect(db);
        rc = mco_db_close(db_name);
      }
    } else {
      /* Connect failed; disconnect from database... */
      sample_rc_check("\tConnect", rc );
      rc = mco_db_close(db_name);
    }
  }

  /* stop eXtremeDB runtime */
  mco_runtime_stop();

  /* free allocated memory */
  free( dev.dev.conv.ptr );

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

/* Add Person records then perform searches, updates and deletes */
void perform_transactions(database_t db)
{
  printf("\n\tAdd Person records...\n");
  add_person( db, "John Smith", "unknown", "Team leader", 75000, 80.3f, NULL );
  add_person( db, "Joe Cooker", "unknown", "Programmer", 60000, 70.2f, "John Smith" );
  add_person( db, "Peter Jones", "unknown", "Programmer", 50000, 60.5f, "John Smith" );

  show_persons( db, "John Smith");
  show_persons( db, "Jo%");
  show_persons( db, "%Jone%");

  get_persons( db, "Jo%" );

  update_person(db, "Peter Jones", 60000, 65.7f);

  show_persons( db, "%" );

  get_subordinates( db, "John Smith" );

  printf("\n\tDeleting all Person records...\n");
  delete_all_persons(db);
}

/* Return the autoid for the Person record with the specified name */
MCO_RET get_person_id( database_t db, char const* name, int64_t* id )
{
  MCO_RET rc = MCO_S_OK;
  data_source_t data_source;
  cursor_t cursor;
  status_t status = SQL_OK;
  record_t record;

  rc = (MCO_RET)mcosql_execute_query(db, NULL, &data_source, "select autoid from Person where name=%s", name);
  if ( MCO_S_OK == rc ) {

    rc = (MCO_RET)mcosql_get_cursor(data_source, &cursor);
    if ( MCO_S_OK == rc ) {
      status = mcosql_cursor_move_next(cursor, &record);
      if ( SQL_OK == status ) {
        type_t type;
        rc = (MCO_RET)mcosql_get_column_value(record, 0, &type, id, sizeof(int64_t), NULL);
      }
      else if ( NO_MORE_ELEMENTS != status )
      {
          mcosql_error_handler(status);
      }
    }

    mcosql_release_query_result(data_source);
  }
  
  return rc;
}

/* Add a Person record; If a manager name is specified, retreive the manager_id and include it in the Person record */
MCO_RET add_person(database_t db, char const* name, char const* address, char const* position, 
               uint4 salary, float rating, char const* manager)
{
  MCO_RET rc = MCO_S_OK;
  int64_t manager_id = 0;

  if ( NULL != manager ) {
    rc = get_person_id( db, manager, &manager_id );
    if ( MCO_S_OK != rc )
    {
      fprintf(stderr, "Unable to find manager '%s'\n", manager);
      return rc;
    }
  }

  rc = (MCO_RET)mcosql_execute_statement(db, NULL, NULL, 
        "insert into Person (name,address,position,salary,rating,manager) values (%s,%s,%s,%u,%f,%p)",
        name, address, position, salary, rating, &manager_id );
  if ( MCO_S_OK == rc ) 
    printf("\t\t%s: %s with manager: %s\n", name, position, ( NULL != manager ? manager : "no manager" ) );
  else
    printf("\t\tUnable to add %s; error code = %d\n", name, rc );

  return rc;
}

/* Lookup Person record by Id and return the name field value */
MCO_RET get_person_name(database_t db, char* buf, size_t buf_size, int64_t* id)
{
  MCO_RET rc = MCO_S_OK;
  data_source_t data_source;
  cursor_t cursor;
  status_t status;
  record_t record;
  type_t type;

  rc = (MCO_RET)mcosql_execute_query( db, NULL, &data_source, "select name from Person where autoid=%p", id );
  if ( MCO_S_OK == rc ) {
    rc = (MCO_RET)mcosql_get_cursor( data_source, &cursor );
    if ( MCO_S_OK == rc ) {
      status = mcosql_cursor_move_next(cursor, &record );
      if ( SQL_OK == status ) {
        status = mcosql_get_column_value( record, 0, &type, buf, buf_size, NULL );
      }
      else if ( NO_MORE_ELEMENTS != status )
      {
        mcosql_error_handler( status );
      }
    }
    mcosql_release_query_result( data_source );
  }

  return rc;
}


/* Display the specified name_pattern and the records found */
MCO_RET show_persons( database_t db, char* name_pattern )
{
  MCO_RET rc = MCO_S_OK;
  data_source_t data_source = NULL;
  cursor_t cursor;
  status_t status;
  record_t record;
  size_t n_columns;
  int i, j, n;
  type_t type;
  char* column_name;
  char value_buf[64];

  printf("\n\tPerson records found with name like '%s':\n", name_pattern );

  rc = (MCO_RET)mcosql_execute_query(db, NULL, &data_source, 
        "select name,position,salary,rating,manager,address from Person where name like %s",
        name_pattern );
  if ( MCO_S_OK == rc ) {
    /* Display header line: iterate through the columns and display the column name. */
    rc = (MCO_RET)mcosql_get_number_of_columns( data_source, &n_columns );
    if ( MCO_S_OK == rc ) {
      for (i = 0; i < n_columns; i++)
      {
        if ( MCO_S_OK != ( status = mcosql_get_column_info( data_source, i, &type, &column_name ) ) )
          break;

        if ( 0 == i )
          printf( "\t" );
        else
          printf( ", " );

        printf("%s", s_truncate( column_name ) );
      }
      if ( MCO_S_OK != status )
      {
          mcosql_error_handler(status);
      }
      printf("\n\t-------------------------------------------------------\n");

      /* For each row of the result set, iterate through the columns and display values. */
      rc = (MCO_RET)mcosql_get_cursor( data_source, &cursor );
      if ( MCO_S_OK == rc ) {
        for ( n = 0; SQL_OK == ( status = mcosql_cursor_move_next( cursor, &record ) ); n++ )
        {
          for (j = 0; j < n_columns; j++)
          {
            rc = (MCO_RET)mcosql_get_column_value( record, j, &type, value_buf, sizeof(value_buf), NULL );
            if ( MCO_S_OK == rc ) {
              if ( 0 == j )
                printf( "\t" );
              else
                printf( ", " );
              print_value(db, type, value_buf);
            }
          }
          printf("\n");
        }
        if ( NO_MORE_ELEMENTS != status )
        {
            mcosql_error_handler(status);
        }
      }
    }
  }

  printf("\t-------------------------------------------------------\n");
  if (data_source != NULL) {
    mcosql_release_query_result( data_source );
  }
  return rc;
}

/* Update the salary and rating for the record with the specified name */
MCO_RET update_person(database_t db, char const* name, uint4 salary, float rating)
{
  int64_t n_records = 0;
  int rc = mcosql_execute_statement(db, NULL, &n_records, "update Person set salary=%u, rating=%f where name=%s",
                      salary, rating, name );
  if ( MCO_S_OK == (MCO_RET)rc )
  {
    printf("\n\tSalary and Rating changed for '%s' to: %u and %.1f\n", name, salary, rating );
  } 
  else 
  {
    fprintf( stderr, "No person found with name '%s'\n", name );
  }
  return (MCO_RET)rc;
}

/* Demonstrate use of the function mcosql_extract_struct() */
MCO_RET get_persons( database_t db, char const* name_like )
{
  MCO_RET rc = MCO_S_OK;
  data_source_t data_source;
  cursor_t cursor;
  status_t status;
  record_t record;
  int64_t id;
  type_t type;
  void* value;
  int n;
  /* Define a structure with exactly the same components as the Person database class */
  struct { char * name; char * address; char * position; uint4 salary; float rating; autoid_t manager; } rec;
  /* Define a character array to hold possible null indicators if any fields are null */
  char null_indicators[10];

  printf("\n\tGet Person records with name like '%s':\n", name_like );

  rc = (MCO_RET)mcosql_execute_query(db, NULL, &data_source, "select * from Person where name like %s", name_like );
  if ( MCO_S_OK == rc ) {
    rc = (MCO_RET)mcosql_get_cursor( data_source, &cursor );
    if ( MCO_S_OK == rc ) {
      for (n = 0; SQL_OK == ( status = mcosql_cursor_move_next( cursor, &record ) ); n++)
      {
        /* Extract the structure "rec" from the result set record */
        rc = (MCO_RET)mcosql_extract_struct(data_source, record, &rec, sizeof(rec), null_indicators);

        printf("\t\t%d. %s: salary=%d, rating = %f\n", n + 1, rec.name, rec.salary, rec.rating);
      }
      if ( NO_MORE_ELEMENTS != status )
      {
          mcosql_error_handler(status);
      }
    } else {
      fprintf(stderr, "No person found with name like '%s'\n", name_like);
    }

    mcosql_release_query_result( data_source );
  }

  return rc;
}

/* Show all subordinates for the specified manager */
MCO_RET get_subordinates( database_t db, char const* manager )
{
  MCO_RET rc = MCO_S_OK;
  data_source_t data_source;
  cursor_t cursor;
  status_t status;
  record_t record;
  int64_t id;
  type_t type;
  char value[64];
  int n;

  printf("\n\tPersons managed by '%s':\n", manager );

  rc = get_person_id( db, manager, &id );
  if ( MCO_S_OK == rc )
  {
    rc = (MCO_RET)mcosql_execute_query(db, NULL, &data_source, "select name from Person where manager=%p",
           &id );
    if ( MCO_S_OK == rc ) {
      rc = (MCO_RET)mcosql_get_cursor( data_source, &cursor );
      if ( MCO_S_OK == rc ) {
        for (n = 0; SQL_OK == ( status = mcosql_cursor_move_next( cursor, &record ) ); n++)
        {
          rc = (MCO_RET)mcosql_get_column_value( record, 0, &type, value, sizeof(value), NULL );
            printf("\t\t%d. %s\n", n + 1, value);
        }
        if ( NO_MORE_ELEMENTS != status )
        {
            mcosql_error_handler(status);
        }
      } else {
        fprintf(stderr, "No person found with name '%s'\n", manager);
      }
    }

    mcosql_release_query_result( data_source );
  }

  return rc;
}

/* Delete all Person records */
MCO_RET delete_all_persons(database_t db)
{
  MCO_RET rc = (MCO_RET)mcosql_execute_statement(db, NULL, NULL, "delete from Person" );

  return rc;
}
/* Truncate string at first white space character */
char * s_truncate( char * name )
{
  int i = 0;
  int len = strlen( name );

  for ( i=0; i < len; i++ )
    if ( isspace( name[i] ) ) break;

  if ( i < len )
    name[i] = 0;

  return name;
}
/* print the value contained in this database field depending on its type */
void print_value(database_t db, int type, void* value)
{
  char buf[MAX_NAME_LENGTH];

  switch (type)
  {
    case CT_NULL:
      printf("null");
      break;
    case CT_BOOL:
      printf(*(char*)value ? "true" : "false");
      break;
    case CT_UINT1:
      printf("%u", *(uint1*)value);
      break;
    case CT_UINT2:
      printf("%u", *(uint2*)value);
      break;
    case CT_UINT4:
      printf("%u", *(uint4*)value);
      break;
    case CT_UINT8:
      printf("%" INT8_FORMAT "d", *(int64_t*)value);
      break;
    case CT_INT1:
      printf("%d", *(int1*)value);
      break;
    case CT_INT2:
      printf("%d", *(int2*)value);
      break;
    case CT_INT4:
      printf("%d", *(int4*)value);
      break;
    case CT_INT8:
      printf("%" INT8_FORMAT "d", *(int64_t*)value);
      break;
    case CT_REAL4:
      printf("%.1f", *(float*)value);
      break;
    case CT_REAL8:
      printf("%.1f", *(double*)value);
      break;
    case CT_STRING:
      printf("%s", (char*)value);
      break;
    case CT_REFERENCE:
      {
        /* lookup the referenced record and extract its name field */
        int64_t id = *(int64_t*)value;
        get_person_name(db, buf, sizeof(buf), &id);
        printf("%s", buf);
        break;
      }
    default:
        printf("???");
  }
}
