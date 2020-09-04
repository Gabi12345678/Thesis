/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include "mcosql.h"
#include <commonSQL.h>

char sample_descr[] = 
"Sample 'transactions_mursiw' demonstrates insert, select, update, and delete\n" 
"operations using C++ with the SQL engine and MURSIW Transaction Mgr.\n";

const char * db_name = "transactionsdb";

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
int addPerson(McoSqlEngine& engine, Person* p);
int updatePersonData(McoSqlEngine& engine, char const* name, int age, float weight);
int deletePerson(McoSqlEngine& engine, char const* name);
int showPersons(QueryResult * result);
int listPersons(McoSqlEngine& engine);
int searchPersonByName(McoSqlEngine& engine, char const* name);

int main( int argc, char ** argv )
{
  McoSqlEngine engine;
  Person p;
  int count = 0;
  McoSqlOpenParameters params;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  // Open eXtremeDB database and SQL engine
  engine.open(db_name,  // database name
    transactionsdb_get_dictionary(),  // database dictionary
    DATABASE_SIZE,  // database size
    MEMORY_PAGE_SIZE,  // page size
    MAP_ADDRESS); // mapping address for shared memory mode

  // Add two Person records
  printf ("\n\tInsert Person records...\n");
  p.name = "John Smith";
  p.age = 35;
  p.weight = 72.1f;
  count = addPerson(engine, &p);
  p.name = "Peter Brown";
  p.age = 40;
  p.weight = 62.1f;
  count += addPerson(engine, &p);
  printf("\t%d records added\n", count);

  // List the contents of the Person table
  printf ("\n\tCurrent list of Person records:\n");
  count = listPersons(engine);

  // Search for Person records by name
  printf ("\n\tSearch persons by name='John'...\n");
  count = searchPersonByName(engine, "John%");
  printf ("\n\tSearch persons by name='Brown'...\n");
  count = searchPersonByName(engine, "%Brown%");

  // Update two Person records by name
  printf ("\n\tUpdate two Person records...\n");
  count = updatePersonData(engine, "John Smith", 36, 75.2f);
  count += updatePersonData(engine, "Peter Brown", 41, 65.0f);
  printf ("\t%d records updated\n", count);

  // List the current contents of the Person table
  printf ("\n\tCurrent list of Person records:\n");
  count = listPersons(engine);

  // Delete two Person records by name
  printf ("\n\tDelete two records by name...\n");
  count = deletePerson(engine, "John Smith");
  count += deletePerson(engine, "Peter Brown");
  printf ("\t%d records deleted\n", count);

  // List the current contents of the Person table
  printf ("\n\tCurrent list of Person records:\n");
  count = listPersons(engine);
  printf ("\t%d records remaining\n", count);

  // Close SQL engine and database
  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

// Add, Update and Delete functions for Person objects
int addPerson(McoSqlEngine& engine, Person* p)
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
int updatePersonData(McoSqlEngine& engine, char const* name, int age, float weight)
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
int deletePerson(McoSqlEngine& engine, char const* name)
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
int listPersons(McoSqlEngine& engine)
{
  // Execute query and store returned data sourse in QueryResult object 
  // which destructor will automatically release this data source after return from 
  // the function
  QueryResult result(engine.executeQuery("select * from Person order by age"));
  return ( showPersons( &result ) );
}

// Search for a Person by name 
int searchPersonByName(McoSqlEngine& engine, char const* name)
{
  // Excecute query with given parameter "name"
  QueryResult result(engine.executeQuery("select * from Person where name like %s", name));
  return ( showPersons( &result ) );
}

