/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include "mcosql.h"
#include <commonSQL.h>

char sample_descr[] = 
"Sample 'transactions_mursiw2' demonstrates insert, select, update, and delete\n" 
"operations using C++ in two database instances with the SQL engine and MURSIW Transaction Mgr.\n";

const char * db_name1 = "transactionsdb1";
const char * db_name2 = "transactionsdb2";

// Define global McoSQL engine 
using namespace McoSql;

// Define the structure correponding to database record Person
struct Person
{
  char const* name;
  int age;
  float weight;
};

// Forward declaration of function describing database schema
GET_DICTIONARY(transactionsdb);

// Forward declarations */
int addPerson(McoSqlEngine &engine, Person* p);
int updatePersonData(McoSqlEngine &engine, char const* name, int age, float weight);
int deletePerson(McoSqlEngine &engine, char const* name);
int showPersons(QueryResult * result);
int listPersons(McoSqlEngine &engine);
int searchPersonByName(McoSqlEngine &engine, char const* name);

int main( int argc, char ** argv )
{
  Person p;
  int count = 0;
  McoSqlOpenParameters params;
  McoSqlEngine engine1;
  McoSqlEngine engine2;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);
  #ifdef EXTREMEDB_LICENSE_KEY
  engine1.license(EXTREMEDB_LICENSE_KEY);
  engine2.license(EXTREMEDB_LICENSE_KEY);
  #endif

  // Open eXtremeDB first database and SQL engine
  engine1.open(db_name1,  // database name
    transactionsdb_get_dictionary(),  // database dictionary
    DATABASE_SIZE,  // database size
    MEMORY_PAGE_SIZE,  // page size
    MAP_ADDRESS); // mapping address for shared memory mode

  // Open eXtremeDB second database and SQL engine
  engine2.open(db_name2,  // database name
    transactionsdb_get_dictionary(),  // database dictionary
    DATABASE_SIZE,  // database size
    MEMORY_PAGE_SIZE,  // page size
    MAP_ADDRESS,  // mapping address for shared memory mode
    0,
    McoSqlOpenParameters::DEFAULT_OPEN_FLAGS & (~McoSqlOpenParameters::START_MCO_RUNTIME));

  // Add two Person records
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

  // List the contents of the Person table
  printf ("\n\tCurrent list of Person records in database 1:\n");
  count = listPersons(engine1);
  printf ("\n\tCurrent list of Person records in database 2:\n");
  count = listPersons(engine2);

  // Search for Person records by name
  printf ("\n\tSearch persons by name='John' in database 1...\n");
  count = searchPersonByName(engine1, "John%");
  printf ("\n\tSearch persons by name='Brown' in database 1...\n");
  count = searchPersonByName(engine1, "%Brown%");

  // Search for Person records by name
  printf ("\n\tSearch persons by name='John' in database 2...\n");
  count = searchPersonByName(engine2, "John%");
  printf ("\n\tSearch persons by name='Brown' in database 2...\n");
  count = searchPersonByName(engine2, "%Brown%");

  // Update two Person records by name
  printf ("\n\tUpdate two Person records in each database...\n");
  count = updatePersonData(engine1, "John Smith 1", 36, 75.2f);
  count += updatePersonData(engine1, "Peter Brown 1", 41, 65.0f);
  count += updatePersonData(engine2, "John Smith 2", 37, 75.2f);
  count += updatePersonData(engine2, "Peter Brown 2", 42, 65.0f);
  printf ("\t%d records updated\n", count);

  // List the current contents of the Person table
  printf ("\n\tCurrent list of Person records in database 1:\n");
  count = listPersons(engine1);
  printf ("\n\tCurrent list of Person records in database 2:\n");
  count = listPersons(engine2);

  // Delete two Person records by name
  printf ("\n\tDelete two records per database by name...\n");
  count = deletePerson(engine1, "John Smith 1");
  count += deletePerson(engine1, "Peter Brown 1");
  count += deletePerson(engine2, "John Smith 2");
  count += deletePerson(engine2, "Peter Brown 2");
  printf ("\t%d records deleted\n", count);

  // List the current contents of the Person table
  printf ("\n\tCurrent list of Person records in database 1:\n");
  count = listPersons(engine1);
  printf ("\n\tCurrent list of Person records in database 2:\n");
  count += listPersons(engine2);
  printf ("\t%d records remaining\n", count);

  // Close SQL engine and database
  engine2.close();
  engine1.close(); // Close this last because ths engine closes mco runtime too

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

// Add, Update and Delete functions for Person objects
int addPerson(McoSqlEngine &engine, Person* p)
{
  // The Person record will be initialized with data passed through the Person struct.
  // Note that the result of this statement execution is the number of records inserted (1 or 0).
  int ret = engine.executeStatement("insert into Person %r", p);
  if ( 0 == ret )
  {
    printf("\t\tError inserting Person %s\n", p->name);
  }
  return ret;
}
int updatePersonData(McoSqlEngine &engine, char const* name, int age, float weight)
{
  // Update the Person record with specified name (passed as query parameter).
  // Note that the result of this statement execution is the number of records updated.
  int ret = engine.executeStatement("update Person set age=%i, weight=%f where name=%s", age, weight, name);
  if ( 0 == ret )
  {
    printf("\t\tPerson %s not found\n", name);
  }
  return ret;
}
int deletePerson(McoSqlEngine &engine, char const* name)
{
  // Delete the Person record with specified name (passed as query parameter).
  // Note that the result of this statement execution is the number of records deleted.
  int ret = engine.executeStatement("delete from Person where name=%s", name);
  if ( 0 == ret )
  {
    printf("\t\tPerson %s not found\n", name);
  }
  return ret;
}

// Extract and display all records from a query result 
int showPersons(QueryResult * result)
{
  int count = 0;
  Cursor* cursor = (*result)->records();
  while (cursor->hasNext())
  {
    Record* rec = cursor->next();
    Person p;

    // Extract the Person record to the corresponding struct
    (*result)->extract(rec, &p, sizeof(p));
    printf("\t\t%s: Age=%d Weight=%.2f\n", p.name, p.age, p.weight);
    count++;
  }
  return count;
}

// Select and display all records from the Person table
int listPersons(McoSqlEngine &engine)
{
  // Execute the query and store the returned data source in the QueryResult object 
  // whose destructor will automatically release this data source on return from 
  // this function
  QueryResult result(engine.executeQuery("select * from Person order by age"));
  return ( showPersons( &result ) );
}

// Search for a Person by name 
int searchPersonByName(McoSqlEngine &engine, char const* name)
{
  // Excecute query with given parameter "name"
  QueryResult result(engine.executeQuery("select * from Person where name like %s", name));
  return ( showPersons( &result ) );
}

