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
#include "mursimuldb.h"

#define TRAP_ON_ERROR(RC) { if ( MCO_S_OK != (RC) ) { fprintf(stderr, "\nLine %d failed with return code %d: %s\n", \
                                    __LINE__, (unsigned int)(RC), mco_ret_string(RC, 0)); do_exit(-1); }}

#define TRAP_ON_ERROR_SQL(RC) { if ( SQL_OK != (RC) ) { fprintf(stderr, "\nLine %d failed with return code %d\n", \
                                    __LINE__, (unsigned int)(RC)); do_exit(-1); }}

const char * db_name1 = "mursimuldb1";
const char * db_name2 = "mursimuldb2";

/* Define the structure correponding to database record Person*/
typedef struct Person_r
{
  char const* name;
  int age;
  float weight;
} Person_r;

/* Forward declarations */
int addPerson(database_t engine, Person_r* p);
int updatePersonData(database_t engine, char const* name, int age, float weight);
int deletePerson(database_t engine, char const* name);
int showPersons(data_source_t data_source);
int listPersons(database_t engine);
int searchPersonByName(database_t engine, char const* name);

static void do_exit(int n)
{
  dbg_exit(n);
}

static void extremedb_error_handler(MCO_RET error_code)
{
  fprintf(stderr, "\n\neXtremeDB error %d\n", error_code);
  sample_os_shutdown();
  do_exit(1);
}

static void mcosql_error_handler(status_t status)
{
  fprintf(stderr, "\n\nMcoSQL error %d\n", status);
  sample_os_shutdown();
  do_exit(1);
}

int main( int argc, char ** argv )
{
  MCO_RET           rc;
  mco_db_h          db1;
  mco_db_h          db2;
  mco_device_t      dev1;
  mco_device_t      dev2;
  mco_db_params_t   db_params;
  database_t engine1;
  database_t engine2;
  status_t sql_stat;

  Person_r p;
  int count = 0;

  sample_os_initialize(DEFAULT);

  sample_header("Sample 'mursimul' demonstrates insert, select, update, and delete\n" 
                "operations in two database instances using the C language API with\n"
                "the SQL engine and MURSIW Transaction Mgr.\n" );

  /* set fatal error handler */
  mco_error_set_handler(extremedb_error_handler);

  /* start eXtremeDB runtime */
  mco_runtime_start();

  /* setup memory devices as a plain conventional memory region */

  dev1.type       = MCO_MEMORY_CONV;                /* set the device as a conventional memory device */
  dev1.assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* assign the device as main database memory */
  dev1.size       = DATABASE_SIZE;                  /* set the device size */
  dev1.dev.conv.ptr = (void*)malloc( DATABASE_SIZE );
  if ( !dev1.dev.conv.ptr ) {
    extremedb_error_handler(MCO_E_NOMEM);
  }

  dev2.type       = MCO_MEMORY_CONV;                /* set the device as a conventional memory device */
  dev2.assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* assign the device as main database memory */
  dev2.size       = DATABASE_SIZE;                  /* set the device size */
  dev2.dev.conv.ptr = (void*)malloc( DATABASE_SIZE );
  if ( !dev2.dev.conv.ptr ) {
    extremedb_error_handler(MCO_E_NOMEM);
  }

  /* initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* set page size for the in memory part */
  db_params.disk_page_size     = 0;                /* set page size to zero to disable disk operations */
  db_params.db_max_connections = 10;                /* set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

  /* open a database 1 on the device with given params */
  rc = mco_db_open_dev(db_name1, mursimuldb_get_dictionary(), &dev1, 1, &db_params );
  TRAP_ON_ERROR(rc);

    /* open a database 2 on the device with given params */
  rc = mco_db_open_dev(db_name2, mursimuldb_get_dictionary(), &dev2, 1, &db_params );
  TRAP_ON_ERROR(rc);

  /* connect to the database 1 by name */
  rc = mco_db_connect( db_name1, &db1 );
  TRAP_ON_ERROR(rc);

  /* connect to the database 2 by name */
  rc = mco_db_connect( db_name2, &db2 );
  TRAP_ON_ERROR(rc);

  sql_stat = mcoapi_create_engine(db1, &engine1);
  TRAP_ON_ERROR_SQL(sql_stat);

  sql_stat = mcoapi_create_engine(db2, &engine2);
  TRAP_ON_ERROR_SQL(sql_stat);

    /* Add two Person records*/
  printf ("\n\tInsert Person records into both databases...\n");
  p.name = "John Smith 1";
  p.age = 35;
  p.weight = 72.1f;
  count = addPerson(engine1, &p);
  p.name = "Peter Brown 1";
  p.age = 40;
  p.weight = 62.1f;
  count += addPerson(engine1, &p);
  p.name = "John Smith 2";
  p.age = 36;
  p.weight = 73.1f;
  count += addPerson(engine2, &p);
  p.name = "Peter Brown 2";
  p.age = 41;
  p.weight = 63.1f;
  count += addPerson(engine2, &p);
  printf("\t%d records added\n", count);

  /* List the contents of the Person table*/
  printf ("\n\tCurrent list of Person records in database 1:\n");
  count = listPersons(engine1);
  printf ("\n\tCurrent list of Person records in database 2:\n");
  count = listPersons(engine2);

  /* Search for Person records by name*/
  printf ("\n\tSearch persons by name='John' in database 1...\n");
  count = searchPersonByName(engine1, "John%");
  printf ("\n\tSearch persons by name='Brown' in database 1...\n");
  count = searchPersonByName(engine1, "%Brown%");

  /* Search for Person records by name*/
  printf ("\n\tSearch persons by name='John' in database 2...\n");
  count = searchPersonByName(engine2, "John%");
  printf ("\n\tSearch persons by name='Brown' in database 2...\n");
  count = searchPersonByName(engine2, "%Brown%");

  /* Update two Person records by name*/
  printf ("\n\tUpdate two Person records per database...\n");
  count = updatePersonData(engine1, "John Smith 1", 36, 75.2f);
  count += updatePersonData(engine1, "Peter Brown 1", 41, 65.0f);
  count += updatePersonData(engine2, "John Smith 2", 37, 75.2f);
  count += updatePersonData(engine2, "Peter Brown 2", 42, 65.0f);
  printf ("\t%d records updated\n", count);

  /* List the current contents of the Person table*/
  printf ("\n\tCurrent list of Person records in database 1:\n");
  count = listPersons(engine1);
  printf ("\n\tCurrent list of Person records in database 2:\n");
  count = listPersons(engine2);

  /* Delete two Person records by name*/
  printf ("\n\tDelete two records per database by name...\n");
  count = deletePerson(engine1, "John Smith 1");
  count += deletePerson(engine1, "Peter Brown 1");
  count += deletePerson(engine2, "John Smith 2");
  count += deletePerson(engine2, "Peter Brown 2");
  printf ("\t%d records deleted\n", count);

  /* List the current contents of the Person table*/
  printf ("\n\tCurrent list of Person records in database 1:\n");
  count = listPersons(engine1);
  printf ("\n\tCurrent list of Person records in database 2:\n");
  count += listPersons(engine2);
  printf ("\t%d records remaining\n", count);

  sql_stat = mcoapi_destroy_engine(engine1);
  TRAP_ON_ERROR_SQL(sql_stat);

  sql_stat = mcoapi_destroy_engine(engine2);
  TRAP_ON_ERROR_SQL(sql_stat);

  /* disconnect database */
  rc = mco_db_disconnect( db1 );
  TRAP_ON_ERROR(rc);

  rc = mco_db_disconnect( db2 );
  TRAP_ON_ERROR(rc);

  /* close the database */
  rc = mco_db_close(db_name1);
  TRAP_ON_ERROR(rc);

  rc = mco_db_close(db_name2);
  TRAP_ON_ERROR(rc);

  /* stop eXtremeDB runtime */
  mco_runtime_stop();

  /* free allocated memory */
  free( dev1.dev.conv.ptr );
  free( dev2.dev.conv.ptr );

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

/* Add, Update and Delete functions for Person objects*/
int addPerson(database_t engine, Person_r *p)
{
  /* The Person record will be initialized with data passed through the Person_r struct.*/
  /* Note that the result of this statement execution is the number of records inserted (1 or 0).*/
  int64_t ret = 0;
  status_t sql_stat;

  sql_stat = mcosql_execute_statement(engine, 0, &ret, "insert into Person %r", p);
  if (SQL_OK != sql_stat || ret == 0)
  {
    printf("\t\tError inserting Person %s\n", p->name);
    TRAP_ON_ERROR_SQL(sql_stat);
  }
  return ret;
}

int updatePersonData(database_t engine, char const* name, int age, float weight)
{
  /* Update the Person record with specified name (passed as query parameter).*/
  /* Note that the result of this statement execution is the number of records updated.*/
  int64_t ret = 0;
  status_t sql_stat;

  sql_stat = mcosql_execute_statement(engine, 0, &ret, "update Person set age=%i, weight=%f where name=%s",
                                                                    age, weight, name);
  if (SQL_OK != sql_stat || ret == 0)
  {
    printf("\t\tPerson %s not found\n", name);
    TRAP_ON_ERROR_SQL(sql_stat);
  }
  return ret;
}

int deletePerson(database_t engine, char const* name)
{
  /* Delete the Person record with specified name (passed as query parameter).*/
  /* Note that the result of this statement execution is the number of records deleted.*/
  int64_t ret = 0;
  status_t sql_stat;

  sql_stat = mcosql_execute_statement(engine, 0, &ret, "delete from Person where name=%s", name);
  if (SQL_OK != sql_stat || ret == 0)
  {
    printf("\t\tPerson %s not found\n", name);
    TRAP_ON_ERROR_SQL(sql_stat);
  }
  return ret;
}

/* Extract and display all records from a query result */
int showPersons(data_source_t data_source)
{
  int count = 0;
  status_t sql_stat;
  cursor_t cursor;
  record_t record;

  sql_stat = mcosql_get_cursor(data_source, &cursor);
  TRAP_ON_ERROR_SQL(sql_stat);

  while (mcosql_cursor_move_next(cursor, &record) == SQL_OK)
  {
    Person_r p;

    /* Extract the Person record to the corresponding struct*/
    sql_stat = mcosql_extract_struct(data_source, record, &p, sizeof(p), 0);
    TRAP_ON_ERROR_SQL(sql_stat);

    printf("\t\t%s: Age=%d Weight=%.2f\n", p.name, p.age, p.weight);
    count++;
  }
  return count;
}

/* Select and display all records from the Person table*/
int listPersons(database_t engine)
{
  /* Execute query and store the returned data source in the QueryResult object */
  /* whose destructor will automatically release this data source on return from */
  /* this function*/
  int ret;
  status_t sql_stat;
  data_source_t data_source;

  sql_stat = mcosql_execute_query(engine, 0, &data_source, "select * from Person order by age");
  TRAP_ON_ERROR_SQL(sql_stat);

  ret = showPersons(data_source);

  sql_stat = mcosql_release_query_result(data_source);
  TRAP_ON_ERROR_SQL(sql_stat);

  return ret;
}

/* Search for a Person by name */
int searchPersonByName(database_t engine, char const* name)
{
  /* Excecute query with given parameter "name"*/
  int ret;
  status_t sql_stat;
  data_source_t data_source;

  sql_stat = mcosql_execute_query(engine, 0, &data_source, "select * from Person where name like %s", name);
  TRAP_ON_ERROR_SQL(sql_stat);

  ret = showPersons(data_source);

  sql_stat = mcosql_release_query_result(data_source);
  TRAP_ON_ERROR_SQL(sql_stat);

  return ret;
}
