/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include "mcosql.h"
#include <commonSQL.h>
#include "FileTable.h"

char sample_descr[] = 
"Sample 'vtable' demonstrates the use of virtual tables\n" 
"in combination with an eXtremeDB database.\n";

char const* db_name = "transactionsdb";

using namespace McoSql;

// Define the structure corresponding to database record Person
struct Person
{
  char const* name;
  int age;
  float weight;
};

struct VTabRow
{
  char const* id;
  char const* value;
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
int addVTable(McoSqlEngine& engine, VTabRow* r);
int updateVTableData(McoSqlEngine& engine, char const* id, const char *value);
int deleteVTable(McoSqlEngine& engine, char const* id);
int showVTable(QueryResult * result);
int listVTable(McoSqlEngine& engine);

int main( int argc, char ** argv )
{
  McoSqlEngine engine;
  Person p;
  VTabRow r;
  int count = 0, count2 = 0;
  McoSqlOpenParameters params;
  FileTable::FieldDescriptor virtualTableFields[] = 
  {
   {"id", 16, tpString},
   {"value", 18, tpString}
  };
  Allocator *allocator = engine.getAllocator();

  remove("vtable.data");

  // Declare Virtual Table 'VTab' and load it from file 'vtable.data'
  FileTable* virtualTable = new (allocator) FileTable(allocator, "VTab", "vtable.data", virtualTableFields, 2);
  Table* virtualTables[] = { virtualTable };

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  // Set database parameters
  params.databaseName = db_name;
  params.dictionary = transactionsdb_get_dictionary();
  params.mainMemoryDatabaseSize = DATABASE_SIZE;
  params.mainMemoryPageSize = MEMORY_PAGE_SIZE;
  params.mainMemoryDatabaseAddress = MAP_ADDRESS;
  // Set Virtual Tables parameters
  params.n_tables = 1;
  params.tables = virtualTables;

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  // Open eXtremeDB database and SQL engine
  engine.open(params);

  // List the contents of the Person table
  printf ("\n\tCurrent list of Person records:\n");
  count = listPersons(engine);
  CHECK_VALUE(count, 0);

  // List the contents of the virtual table
  printf ("\n\tCurrent list of VTable records:\n");
  count2 = listVTable(engine);
  CHECK_VALUE(count2, 0);

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
  CHECK_VALUE(count, 2);

  // Add two VTab records
  printf ("\n\tInsert VTable records...\n");
  r.id = "John Smith";
  r.value = "35";
  count2 = addVTable(engine, &r);
  r.id = "Peter Brown";
  r.value = "40";
  count2 += addVTable(engine, &r);
  printf("\t%d records added\n", count2);
  CHECK_VALUE(count2, 2);

  // List the contents of the Person table
  printf ("\n\tCurrent list of Person records:\n");
  count = listPersons(engine);
  CHECK_VALUE(count, 2);

  // List the contents of the virtual table
  printf ("\n\tCurrent list of VTable records:\n");
  count2 = listVTable(engine);
  CHECK_VALUE(count2, 2);

  // Search for Person records by name
  printf ("\n\tSearch persons by name='John'...\n");
  count = searchPersonByName(engine, "John%");
  CHECK_VALUE(count, 1);
  printf ("\n\tSearch persons by name='Brown'...\n");
  count = searchPersonByName(engine, "%Brown%");
  CHECK_VALUE(count, 1);

  // Update two Person records by name
  printf ("\n\tUpdate two Person records...\n");
  count = updatePersonData(engine, "John Smith", 36, 75.2f);
  count += updatePersonData(engine, "Peter Brown", 41, 65.0f);
  printf ("\t%d records updated\n", count);
  CHECK_VALUE(count, 2);

  // List the current contents of the Person table
  printf ("\n\tCurrent list of Person records:\n");
  count = listPersons(engine);
  CHECK_VALUE(count, 2);

  // Update two VTab records by name
  printf ("\n\tUpdate two VTable records...\n");
  count2 = updateVTableData(engine, "John Smith", "36");
  count2 += updateVTableData(engine, "Peter Brown", "41");
  printf ("\t%d records updated\n", count2);
  CHECK_VALUE(count2, 2);

  // List the current contents of the Person table
  printf ("\n\tCurrent list of VTable records:\n");
  count2 = listVTable(engine);
  CHECK_VALUE(count2, 2);

  // Delete two Person records by name
  printf ("\n\tDelete two records by name...\n");
  count = deletePerson(engine, "John Smith");
  count += deletePerson(engine, "Peter Brown");
  printf ("\t%d records deleted\n", count);
  CHECK_VALUE(count, 2);

  // List the current contents of the Person table
  printf ("\n\tCurrent list of Person records:\n");
  count = listPersons(engine);
  printf ("\t%d records remaining\n", count);
  CHECK_VALUE(count, 0);

  // Delete two VTab records by name
  printf ("\n\tDelete two VTable records by name...\n");
  count2 = deleteVTable(engine, "John Smith");
  count2 += deleteVTable(engine, "Peter Brown");
  printf ("\t%d records deleted\n", count2);
  CHECK_VALUE(count2, 2);

  // List the current contents of the VTab table
  printf ("\n\tCurrent list of VTable records:\n");
  count2 = listVTable(engine);
  printf ("\t%d records remaining\n", count2);
  CHECK_VALUE(count2, 0);

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

// Add, Update and Delete functions for VTable objects
int addVTable(McoSqlEngine& engine, VTabRow* r)
{
  // The VTab record will be initialized with data passed through the VTabRow struct.
  // Note that the result of this statement execution is the number of records inserted (1 or 0).
  int ret = engine.executeStatement("insert into VTab %r", r);
  if ( 0 == ret )
  {
    printf("\t\tError inserting VTab %s\n", r->id);
  }
  return ret;
}

int updateVTableData(McoSqlEngine& engine, char const* id, char const* value)
{
  // Update the VTab record with specified name (passed as query parameter).
  // Note that the result of this statement execution is the number of records updated.
  int ret = engine.executeStatement("update VTab set value=%s where id=%s", value, id);
  if ( 0 == ret )
  {
    printf("\t\nVTab %s not found\n", id);
  }
  return ret;
}

int deleteVTable(McoSqlEngine& engine, char const* id)
{
  // Delete the VTab record with specified name (passed as query parameter).
  // Note that the result of this statement execution is the number of records deleted.
  int ret = engine.executeStatement("delete from VTab where id=%s", id);
  if ( 0 == ret )
  {
    printf("\t\tVTab %s not found\n", id);
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

int showVTable(QueryResult * result)
{
  int count = 0;
  Cursor* cursor = (*result)->records();
  while (cursor->hasNext())
  {
    Record* rec = cursor->next();
    VTabRow r;

    // Extract the VTabRow record to the corresponding struct
    (*result)->extract(rec, &r, sizeof(r));
    printf("\t\tId=(%s) Value=(%s)\n", r.id, r.value );
    count++;
  }
  return count;
}

// Select and display all records from the Person and VTable tables
int listPersons(McoSqlEngine& engine)
{
  // Execute query and store returned data source in QueryResult object 
  // which destructor will automatically release this data source after return from 
  // the function
  QueryResult result(engine.executeQuery("select * from Person order by age"));
  return ( showPersons( &result ) );
}

int listVTable(McoSqlEngine& engine)
{
  QueryResult result(engine.executeQuery("select * from VTab"));
  return ( showVTable( &result ) );
}

// Search for a Person by name
int searchPersonByName(McoSqlEngine& engine, char const* name)
{
  // Execute query with given parameter "name"
  QueryResult result(engine.executeQuery("select * from Person where name like %s", name));
  return ( showPersons( &result ) );
}
