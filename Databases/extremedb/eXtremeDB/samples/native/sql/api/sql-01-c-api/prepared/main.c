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
#include <sqlcln_c.h>
#include "prepareddb.h"


#define TRAP_ON_ERROR(RC) { if ( MCO_S_OK != (RC) ) { fprintf(stderr, "\nLine %d failed with return code %d: %s\n", \
                                    __LINE__, (unsigned int)(RC), mco_ret_string(RC, 0)); do_exit(-1); }}

#define TRAP_ON_ERROR_SQL(RC) { if ( SQL_OK != (RC) ) { fprintf(stderr, "\nLine %d failed with return code %d\n", \
                                    __LINE__, (unsigned int)(RC)); do_exit(-1); }}

const char * db_name = "preparemdb";

/* Define the structure correponding to database record Person*/
typedef struct Person_r
{
  char const* name;
  int age;
  double weight;
} Person_r;

/* Forward declarations */
int addPerson(database_t engine, Person_r* p);
int addPersons(database_t engine, Person_r p[], int npersons, mco_bool local);
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

static void usage(void)
{
    printf ("To use local SQL engine start program with no arguments:\n\tprepared\n");
    printf ("To use remote SQL server start program with host:port as argument:\n\tprepared localhost:5000\n");
}

int main( int argc, char ** argv )
{
  MCO_RET           rc;
  mco_db_h          db;
  mco_device_t      dev;
  mco_db_params_t   db_params;
  database_t engine;
  status_t sql_stat;

  char *host = NULL;
  int port = 5000;

  Person_r p;
  Person_r prs[2];

  int count = 0;

  if (argc == 2) {
      char *s, *cp = (char *)malloc (strlen(argv[1] + 1));
      strcpy(cp, argv[1]);
      s = strchr(cp, ':');
      if (s == NULL) {
          usage();
          exit(1);
      }
      *s = 0;
      host = (char *)malloc(strlen(cp));
      strcpy(host, cp);
      port = atoi(s+1);
  } else if (argc != 1) {
      usage();
      exit(1);
  }

  sample_os_initialize(DEFAULT);

  sample_header("Sample 'prepared' demonstrates insert, select, update, and delete\n" 
                "operations using prepared statements using the C language API with\n"
                "the SQL engine and MURSIW Transaction Mgr.\n" );

  /* set fatal error handler */
  mco_error_set_handler(extremedb_error_handler);

  /* start eXtremeDB runtime */
  mco_runtime_start();


  if (host == NULL)
  {
      /* setup memory devices as a plain conventional memory region */

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
      db_params.db_max_connections = 10;                /* set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
      db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

      /* open a database on the device with given params */
      rc = mco_db_open_dev(db_name, prepareddb_get_dictionary(), &dev, 1, &db_params );
      TRAP_ON_ERROR(rc);

      /* connect to the database by name */
      rc = mco_db_connect( db_name, &db );
      TRAP_ON_ERROR(rc);

      sql_stat = mcoapi_create_engine(db, &engine);
      TRAP_ON_ERROR_SQL(sql_stat);
  } else {
      sql_stat = sqlcln_create(&engine, 128*1024);
      TRAP_ON_ERROR_SQL(sql_stat);

      sql_stat = sqlcln_open(engine, host, port, 10);
      TRAP_ON_ERROR_SQL(sql_stat);
  }

    /* Add two Person records*/
  printf ("\n\tInsert Person records into database...\n");
  p.name = "John Smith";
  p.age = 35;
  p.weight = 72.1f;
  count = addPerson(engine, &p);
  p.name = "Peter Brown";
  p.age = 40;
  p.weight = 62.1f;
  count += addPerson(engine, &p);

  prs[0].name = "Adam Smith";
  prs[0].age = 42;
  prs[0].weight = 54.5f;
  prs[1].name = "Hello World";
  prs[1].age = 25;
  prs[1].weight = 89.9f;

  count += addPersons(engine, prs, sizeof(prs) / sizeof(prs[0]), host == NULL ? MCO_YES : MCO_NO);

  /* List the contents of the Person table*/
  printf ("\n\tCurrent list of Person records in database:\n");
  count = listPersons(engine);

  /* Search for Person records by name*/
  printf ("\n\tSearch persons by name='John' in database ...\n");
  count = searchPersonByName(engine, "John%");
  printf ("\n\tSearch persons by name='Brown' in database ...\n");
  count = searchPersonByName(engine, "%Brown%");

  /* Update two Person records by name*/
  printf ("\n\tUpdate two Person records per database...\n");
  count = updatePersonData(engine, "John Smith", 36, 75.2f);
  count += updatePersonData(engine, "Peter Brown", 41, 65.0f);
  printf ("\t%d records updated\n", count);

  /* List the current contents of the Person table*/
  printf ("\n\tCurrent list of Person records in database:\n");
  count = listPersons(engine);

  /* Delete two Person records by name*/
  printf ("\n\tDelete two records by name...\n");
  count = deletePerson(engine, "John Smith");
  count += deletePerson(engine, "Peter Brown");
  printf ("\t%d records deleted\n", count);

  /* List the current contents of the Person table*/
  printf ("\n\tCurrent list of Person records in database:\n");
  count = listPersons(engine);
  printf ("\t%d records remaining\n", count);

  if (host == NULL) {
      sql_stat = mcoapi_destroy_engine(engine);
      TRAP_ON_ERROR_SQL(sql_stat);

      /* disconnect database */
      rc = mco_db_disconnect( db );
      TRAP_ON_ERROR(rc);

      /* close the database */
      rc = mco_db_close(db_name);
      TRAP_ON_ERROR(rc);
  } else {
      sql_stat = sqlcln_close(engine);
      TRAP_ON_ERROR_SQL(sql_stat);

      sql_stat = sqlcln_destroy(engine);
      TRAP_ON_ERROR_SQL(sql_stat);
  }

  /* stop eXtremeDB runtime */
  mco_runtime_stop();

  if (host == NULL) {
    /* free allocated memory */
    free( dev.dev.conv.ptr );
  }

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
  prepared_statement_t stmt;

  sql_stat = mcosql_prepare_statement(engine, &stmt, "insert into Person(name, age, weight) values(%*s, %*i, %*f)",
                                      &p->name, &p->age, &p->weight);

  if (SQL_OK != sql_stat)
  {
    printf("\t\tError preparing statement for %s\n", p->name);
    TRAP_ON_ERROR_SQL(sql_stat);
  }

  sql_stat = mcosql_execute_prepared_statement(engine, 0, stmt, &ret );
  if (SQL_OK != sql_stat || ret == 0)
  {
    printf("\t\tError inserting Person %s\n", p->name);
    TRAP_ON_ERROR_SQL(sql_stat);
  }

  sql_stat = mcosql_release_prepared_statement(engine, stmt);
  if (SQL_OK != sql_stat)
  {
    printf("\t\tError releasing statement for %s\n", p->name);
    TRAP_ON_ERROR_SQL(sql_stat);
  }

  return ret;
}

/* Add persons using indirect parameter value */
/* this may work as binding parameters */

int addPersons(database_t engine, Person_r p[], int npersons, mco_bool local)
{
  /* The Person record will be initialized with data passed through the Person_r struct.*/
  /* Note that the result of this statement execution is the number of records inserted (1 or 0).*/
  int64_t ret = 0;
  int64_t count = 0;
  status_t sql_stat;
  prepared_statement_t stmt;
  Person_r prs;
  int i;

  sql_stat = mcosql_execute_statement(engine, 0, 0, "start transaction");
  if (SQL_OK != sql_stat)
  {
    printf("\t\tError starting transaction\n");
    TRAP_ON_ERROR_SQL(sql_stat);
  }

  if (local)
    sql_stat = mcosql_prepare_statement(engine, &stmt, "insert into Person %*r", &prs);
  else
    sql_stat = mcosql_prepare_statement(engine, &stmt, "insert into Person(name, age, weight) values(%*s, %*i, %*f)",
                                      &prs.name, &prs.age, &prs.weight);
  if (SQL_OK != sql_stat)
  {
    printf("\t\tError preparing statement for persons list\n");
    TRAP_ON_ERROR_SQL(sql_stat);
  }

  for (i = 0; i < npersons; i++) {
      prs = p[i];
      sql_stat = mcosql_execute_prepared_statement(engine, 0, stmt, &count );
      if (SQL_OK != sql_stat || count == 0)
      {
          printf("\t\tError inserting Person %s\n", p[i].name);
          TRAP_ON_ERROR_SQL(sql_stat);
      }
      ret += count;
  }

  sql_stat = mcosql_release_prepared_statement(engine, stmt);
  if (SQL_OK != sql_stat)
  {
    printf("\t\tError releasing statement\n");
    TRAP_ON_ERROR_SQL(sql_stat);
  }

  sql_stat = mcosql_execute_statement(engine, 0, 0, "commit transaction");
  if (SQL_OK != sql_stat)
  {
    printf("\t\tError committing transaction\n");
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
  prepared_statement_t stmt;

  sql_stat = mcosql_prepare_statement(engine, &stmt, "update Person set age=%i, weight=%f where name=%s",
                                      age, weight, name);

  if (SQL_OK != sql_stat)
  {
    printf("\t\tError preparing statement for %s\n", name);
    TRAP_ON_ERROR_SQL(sql_stat);
  }

  sql_stat = mcosql_execute_prepared_statement(engine, 0, stmt, &ret);
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
  prepared_statement_t stmt;

  sql_stat = mcosql_prepare_statement(engine, &stmt, "delete from Person where name=%s", name);
  if (SQL_OK != sql_stat)
  {
    printf("\t\tError preparing statement for %s\n", name);
    TRAP_ON_ERROR_SQL(sql_stat);
  }

  sql_stat = mcosql_execute_prepared_statement(engine, 0, stmt,  &ret);
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
  /* Execute query with given parameter "name"*/
  int ret;
  status_t sql_stat;
  data_source_t data_source;
  prepared_statement_t stmt;

  sql_stat = mcosql_prepare_statement(engine, &stmt, "select * from Person where name like %s", name);
  if (sql_stat != SQL_OK)
  {
    printf("\t\tError preparing statement for %s\n", name);
    TRAP_ON_ERROR_SQL(sql_stat);
  }

  sql_stat = mcosql_execute_prepared_query(engine, 0, stmt, &data_source );
  TRAP_ON_ERROR_SQL(sql_stat);

  ret = showPersons(data_source);

  sql_stat = mcosql_release_query_result(data_source);
  TRAP_ON_ERROR_SQL(sql_stat);

  return ret;
}
