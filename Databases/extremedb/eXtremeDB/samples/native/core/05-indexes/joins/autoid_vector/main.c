/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.    *
 *                                                              *
 ****************************************************************/

#include <common.h>
#include "autoid_vector_db.h"

char sample_descr[] = "Sample 'autoid_vector' demonstrates using a vector of autoids to join two classes.\n";
const char * db_name = "autoid_vect_db";
const int VECTOR_SIZE = 10;

// The struct corresponding to class Department
struct department_data {
    char name[20];
    char code[10];
};
struct department_data departments[] = {
    { "Accounting", "Acct" },{ "Engineering", "Eng" },{ "Customer Service", "CS" },{ "Technical support", "TS" }
};

// The struct corresponding to class Employee
struct employee_data {
    char name[20];
    autoid_t dept;
};

// Define the relationship of Employees to Departments
struct employee_department {
    char name[20];
    char dept_code[10];
};
struct employee_department employee_departments[] = {
  { "John", "Acct" }, { "Samuel", "Acct"}, { "Thomas", "Acct" }, { "David", "Eng" }, { "James", "Eng"}, { "Robert", "Eng"},
  { "William", "CS"}, { "Kevin", "CS"}, { "Alex", "CS"}, { "Daniel", "CS"}, { "Diego", "CS"},  { "Brandon", "TS" }
};    

MCO_RET fill_database(mco_db_h db)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h t;
  unsigned int i;
  char name[30] = "";
  uint2 len;
  autoid_t emp_id1 = 0;

  printf("\nCreate departments:\n");
  for (i = 0; i < sizeof(departments) / sizeof(departments[0]) && MCO_S_OK == rc; ++i) {
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if ( MCO_S_OK == rc )  
    {
        Department dept;
        autoid_t autoid = 0;
        char name[20];
        char code[10];
        uint2 len, j;
        Department_new(t, &dept);
        Department_name_put(&dept, departments[i].name, strlen(departments[i].name));
        Department_code_put(&dept, departments[i].code, strlen(departments[i].code));

        // Allocate space for Employee autoids and initialize to 0
        Department_employees_alloc(&dept, VECTOR_SIZE);
        for (j = 0; j < VECTOR_SIZE; j++) {
            Department_employees_put(&dept, j, 0);
        }

        Department_name_get(&dept, name, sizeof(name), &len);
        Department_code_get(&dept, code, sizeof(code), &len);
        Department_autoid_get(&dept, &autoid);
        printf("\n\t%d) Department: %s, %s, Autoid: %ld", i, code, name, (long int)autoid);
        rc = mco_trans_commit(t);
    }
  }
  if ( MCO_S_OK != rc ) {
    sample_rc_check("\tUnable to insert Department record", rc );
    sample_pause_end("\n\nPress any key to continue . . . ");
  }

  // Create and insert Employee objects
  printf("\n\nCreate employees and join each to a department:\n");

  for (i = 0; i < sizeof(employee_departments) / sizeof(employee_departments[0]) && MCO_S_OK == rc; ++i)
  {
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if (MCO_S_OK == rc) 
    {
        // Find the Department object for this Employee by the BTree index on Department.code
        Department dept;
        rc = Department_Icode_find(t, employee_departments[i].dept_code, strlen(employee_departments[i].dept_code), &dept);
        if (MCO_S_OK == rc)
        {
          short j;
          autoid_t dept_id;
          Department_autoid_get(&dept, &dept_id);

          // Find the first vacant Employee vector element
          for (j = 0; j < VECTOR_SIZE; j++)
          {
              rc = Department_employees_at(&dept, j, &emp_id1);
              if (0 == emp_id1)
              {
                  Employee emp;
                  autoid_t emp_id2;
                  Employee_new(t, &emp);
                  Employee_name_put(&emp, employee_departments[i].name, strlen(employee_departments[i].name));
                  Employee_dept_put(&emp, dept_id);
                  Employee_name_get(&emp, name, sizeof(name), &len);
                  Employee_autoid_get(&emp, &emp_id2);
                  Department_employees_put(&dept, j, emp_id2);
                  printf("\t %d) name: %s, Autoid=%ld, Department.Autoid = %ld \n", i, name, (long int)emp_id2, (long int)dept_id);
                  break;
              }
          }
        }
        rc = mco_trans_commit(t);
      }
  }
  if (MCO_S_OK != rc) {
    sample_rc_check("\tUnable to insert Department record", rc);
    sample_pause_end("\n\nPress any key to continue . . . ");
  }

  return rc;
}

MCO_RET find_coworkers(mco_db_h db, char * search_name)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h t;
  char str[100];

  // Find co-workers of search_emp
  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) 
  {
    char name[20];
    uint2 len;
    Department dept1;
    autoid_t emp_id1, emp_id2, dept_id1;
    Employee emp1, emp2;
      
    // 1. find employee record by name
    rc = Employee_Iname_find(t, search_name, strlen(search_name), &emp1);
    if ( MCO_S_OK == rc ) 
    {
      printf("\n\n\t%s's co-workers in ", search_name);
      Employee_dept_get(&emp1, &dept_id1);
      rc = Employee_autoid_get(&emp1, &emp_id1 );

      // 2. Find the Department object by its autoid and display the Department name
      rc = Department_autoid_find(t, dept_id1, &dept1);
      if (MCO_S_OK == rc)
      {
        uint2 j;
        Department_name_get(&dept1, name, sizeof(name), &len);
        printf("%s are:\n", name);

        // 3. Scroll through the vector of Employee autoids, find the Employee object and display its name
        for (j = 0; j < VECTOR_SIZE; j++)
        {
          rc = Department_employees_at(&dept1, j, &emp_id2);

          // End loop when the value of this vector element is 0
          if (0 == emp_id2) break;

          // Skip if this is the autoid of the "search_name" object
          if (emp_id1 != emp_id2)
          {
            // Find the Employee object by autoid and display the name
            rc = Employee_autoid_find(t, emp_id2, &emp2);
            if (MCO_S_OK == rc)
            {
              Employee_name_get(&emp2, name, sizeof(name), &len);
              printf("\t%s\n", name);
            }
          }
        }
      } else {
        sample_rc_check("\nDepartment not found", rc );
      }
    } else {
      sprintf( str, "Can't find '%s'", search_name);
      sample_rc_check(str, rc );
    }
    mco_trans_rollback(t);
  }

  return rc;
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
    
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  // Set fatal error handler and start eXtremeDB runtime
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  // Open and connect to database
  rc = sample_open_database( db_name, autoid_vector_db_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      // Fill db, then perform search for employees in same department as William
      rc = fill_database(db);
      if ( MCO_S_OK == rc ) {
        rc = find_coworkers(db, "William");
      }
      // Disconnect and close database, stop runtime
      mco_db_disconnect(db);
    }
  
    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
