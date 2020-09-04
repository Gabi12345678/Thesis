/*******************************************************************
 *                                                                 *
 *  simple.cpp                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifdef _WIN32
    #include <windows.h>
#endif 
#include "mcosql.h"
#include "sql.h"
#include "sqlext.h"

using namespace McoSql;

size_t const DB_PAGE_SIZE = 128; // define eXtremeDB page size
size_t const DATABASE_SIZE = 4 * 1024 * 1024; // define size of eXtrmeDB database
void* const MAP_ADDRESS = MCO_DATABASE_DEFAULT_MAP_ADDRESS; // define mapping address for eXtremeDB database

const int MAX_NAME_LENGTH = 100;

// Define structure corresponding to database record 
// (will be done automatically by mcocomp in future
struct Person
{
    char const* name;
    int age;
    float weight;
};

// Forward declaration of function describing database schema
GET_DICTIONARY(simpledb);

McoMultithreadedSqlEngine engine; // McoSQL engine 
SQLHENV hEnv; // ODBC database environment handle
SQLHDBC hDbc; // ODBC database connection handle

#define ODBC_CHECK(op) do { SQLRETURN rc = op; if (rc != SQL_SUCCESS) \
{ fprintf(stderr, "%s:%d: " #op " failed with code %d\n",\
__FILE__, __LINE__, rc); abort(); } } while (0)

// This function will add new record in database
void addPerson(Person* p)
{
    // Add new record to the table. 
    // Record will be initialized with data passed through Person struct.
    engine.executeStatement("insert into Person %r", p);
}

// This function select all records from the table
void listPersons()
{
    SQLHSTMT hStmt;
    SQLLEN nameLen;
    float weight;
    int age;
    char name[MAX_NAME_LENGTH];

    ODBC_CHECK(SQLAllocStmt(hDbc, &hStmt)); // allocate statement

    ODBC_CHECK(SQLExecDirect(hStmt, (SQLCHAR*)"select * from Person order by age", SQL_NTS)); // execute query

    // Bind columns
    ODBC_CHECK(SQLBindCol(hStmt, 1, SQL_C_CHAR, &name, sizeof name, &nameLen));
    ODBC_CHECK(SQLBindCol(hStmt, 2, SQL_C_LONG, &age, sizeof age, NULL));
    ODBC_CHECK(SQLBindCol(hStmt, 3, SQL_C_FLOAT, &weight, sizeof weight, NULL));

    // Loop for all records
    while (SQLFetch(hStmt) == SQL_SUCCESS)
    {
        printf("%.*s %d %f\n", (int)nameLen, name, age, weight);
    }
    ODBC_CHECK(SQLCloseCursor(hStmt)); // release result set
    ODBC_CHECK(SQLFreeStmt(hStmt, SQL_DROP)); // drop statement
}

// Search person by name pattern 
void searchPersonsByName(char const **namePatterns)
{
    SQLHSTMT hStmt;
    SQLLEN nameLen = SQL_NTS;
    float weight;
    int age;
    char name[MAX_NAME_LENGTH];
    char namePatternBuff[32];
    char const *namePattern;

    ODBC_CHECK(SQLAllocStmt(hDbc, &hStmt)); // allocate statement

    ODBC_CHECK(SQLPrepare(hStmt, (SQLCHAR*)"select * from Person where name like ?", SQL_NTS)); // prepare query

    // Bind parameters
    ODBC_CHECK(SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)namePatternBuff, 0,
               NULL));

    // Bind columns
    ODBC_CHECK(SQLBindCol(hStmt, 1, SQL_C_CHAR, &name, sizeof name, &nameLen));
    ODBC_CHECK(SQLBindCol(hStmt, 2, SQL_C_LONG, &age, sizeof age, NULL));
    ODBC_CHECK(SQLBindCol(hStmt, 3, SQL_C_FLOAT, &weight, sizeof weight, NULL));

    for (; *(namePatterns) != NULL; namePatterns++)
    {
        namePattern = *(namePatterns);
        strcpy(namePatternBuff, namePattern);

        ODBC_CHECK(SQLExecute(hStmt)); // execute prepared statement

        // Loop for all records
        while (SQLFetch(hStmt) == SQL_SUCCESS)
        {
            printf("%.*s %d %f\n", (int)nameLen, name, age, weight);
        }
        ODBC_CHECK(SQLCloseCursor(hStmt)); // release result set
    }
    ODBC_CHECK(SQLFreeStmt(hStmt, SQL_DROP)); // drop statement
}

// Calculate average age of persons using aggregate functions 
void calculateAverageAge()
{
    // Execute query calculating average age of all persons
    QueryResult result(engine.executeQuery("select avg(age) from Person"));

    // Get cursor
    Cursor* cursor = result->records();

    // Result data source consists of one record with single column.
    // Indices of columns starts from 0
    Value* avgAge = cursor->next()->getCopy(0);

    printf("Average age: %d\n", (int)avgAge->intValue());
}

// Update record
void updatePersonData(char const* name, int age, float weight)
{
    SQLHSTMT hStmt;

    ODBC_CHECK(SQLAllocStmt(hDbc, &hStmt)); // allocate statement
    ODBC_CHECK(SQLPrepare(hStmt, (SQLCHAR*)"update Person set age=?, weight=? where name=?", SQL_NTS)); // perpare query

    // Bind parameters
    ODBC_CHECK(SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &age, 0, NULL));
    ODBC_CHECK(SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &weight, 0, NULL));
    ODBC_CHECK(SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)name, 0, NULL));

    ODBC_CHECK(SQLExecute(hStmt)); // execute prepared statement
    ODBC_CHECK(SQLFreeStmt(hStmt, SQL_DROP)); // drop statement
}

// Delete record from the table
void deletePerson(char const* name)
{
    SQLHSTMT hStmt;
    SQLLEN nameLen = SQL_NTS;

    ODBC_CHECK(SQLAllocStmt(hDbc, &hStmt)); // allocate statement

    // Bind parameter
    ODBC_CHECK(SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)name, 0, &nameLen));

    ODBC_CHECK(SQLExecDirect(hStmt, (SQLCHAR*)"delete from Person where name=?", SQL_NTS)); // execute statement
    ODBC_CHECK(SQLFreeStmt(hStmt, SQL_DROP)); // drop statement
}

void listTables()
{
    SQLHSTMT hStmt;
    SQLLEN tableCatLen = SQL_NTS, tableSchemaLen = SQL_NTS, tableNameLen = SQL_NTS,
        tableTypeLen = SQL_NTS, remarksLen = SQL_NTS;
    char tableCat[MAX_NAME_LENGTH];
    char tableSchema[MAX_NAME_LENGTH];
    char tableName[MAX_NAME_LENGTH];
    char tableType[MAX_NAME_LENGTH];
    char remarks[MAX_NAME_LENGTH];

    ODBC_CHECK(SQLAllocStmt(hDbc, &hStmt)); // allocate statement

    ODBC_CHECK(SQLBindCol(hStmt, 1, SQL_C_CHAR, &tableCat, sizeof tableCat, &tableCatLen));
    ODBC_CHECK(SQLBindCol(hStmt, 2, SQL_C_CHAR, &tableSchema, sizeof tableSchema, &tableSchemaLen));
    ODBC_CHECK(SQLBindCol(hStmt, 3, SQL_C_CHAR, &tableName, sizeof tableName, &tableNameLen));
    ODBC_CHECK(SQLBindCol(hStmt, 4, SQL_C_CHAR, &tableType, sizeof tableType, &tableTypeLen));
    ODBC_CHECK(SQLBindCol(hStmt, 5, SQL_C_CHAR, &remarks, sizeof remarks, &remarksLen));

    ODBC_CHECK(SQLTables(hStmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS));

    // Loop for all records
    while (SQLFetch(hStmt) == SQL_SUCCESS)
    {
        printf("Catalog: '%s'\nShema  : '%s'\nDB Name: '%s'\nDB Type: '%s'\nRemark : '%s'\n\n",
            tableCat, tableSchema, tableName, tableType, remarks);
    }
    ODBC_CHECK(SQLCloseCursor(hStmt)); // release result set

    ODBC_CHECK(SQLFreeStmt(hStmt, SQL_DROP)); // drop statement
}

int main()
{
    Person p;

    // Open eXtremeDB database and SQL engine
    engine.open("simpledb",  // database name
    simpledb_get_dictionary(),  // database dictionary
    DATABASE_SIZE,  // database size
    DB_PAGE_SIZE,  // page size
    MAP_ADDRESS); // mapping address for shared memory mode

    ODBC_CHECK(SQLAllocEnv(&hEnv)); // allocate environment
    ODBC_CHECK(SQLAllocConnect(hEnv, &hDbc)); // create database handle
    ODBC_CHECK(SQLConnect(hDbc, (SQLCHAR*)&engine, -1 /* specify a pointer nature of the second argument */, NULL, 0, NULL, 0)); // connect to the database

    p.name = "John Smith";
    p.age = 35;
    p.weight = 72.1f;
    addPerson(&p);

    p.name = "Peter Brown";
    p.age = 40;
    p.weight = 62.1f;
    addPerson(&p);

    listTables();

    listPersons();

    const char *namesToSearch[3] = { "John%", "%Brown%", NULL };
    searchPersonsByName(namesToSearch);

    calculateAverageAge();

    updatePersonData("John Smith", 36, 75.2f);
    updatePersonData("Peter Brown", 41, 65.0f);

    listPersons();

    calculateAverageAge();

    deletePerson("John Smith");
    deletePerson("Peter Brown");

    ODBC_CHECK(SQLDisconnect(hDbc)); // close ODBC connection
    ODBC_CHECK(SQLFreeConnect(hDbc)); // free connection handle
    ODBC_CHECK(SQLFreeEnv(hEnv)); // free environment handle

    // Close database and SQL engine
    engine.close();

    return 0;
}
