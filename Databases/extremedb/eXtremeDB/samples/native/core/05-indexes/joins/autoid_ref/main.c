/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.    *
 *                                                              *
 ****************************************************************/

#include <common.h>
#include "autoid_ref_db.h"

char sample_descr[] = "Sample 'autoid_ref' demonstrates using an autoid to join two classes.\n";
const char * db_name = "autoid_join_db";

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
    char name[10];
    autoid_t dept_id;
};

// Define the relationship of Employees to Departments
struct employee_department {
    char name[20];
    char code[10];
};
struct employee_department employee_departments[] = {
  { "John", "Acct" }, { "Samuel", "Acct" }, { "Thomas", "Acct" }, { "David", "Eng" }, { "James", "Eng" }, { "Robert", "Eng" },
  { "William", "CS" }, { "Kevin", "CS" }, { "Alex", "CS" }, { "Daniel", "CS" }, { "Diego", "CS" }, { "Brandon", "TS" }
};    

MCO_RET fill_database(mco_db_h db)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h t;
  unsigned int i;

  printf("\nCreate departments:\n");
  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  for (i = 0; i < sizeof(departments) / sizeof(departments[0]) && MCO_S_OK == rc; ++i) {
    if ( MCO_S_OK == rc )  {
        Department dept;
      autoid_t autoid = 0;
      Department_new(t, &dept);
      Department_name_put(&dept, departments[i].name, (uint2)strlen(departments[i].name));
        Department_code_put(&dept, departments[i].code, (uint2)strlen(departments[i].code));
      Department_autoid_get(&dept, &autoid);
      printf("\n\t%d) %s, %s, autoid=%ld", i, departments[i].code, departments[i].name, (long)autoid);
    }
  }
  if ( MCO_S_OK != rc ) {
    sample_rc_check("\tUnable to insert Department record", rc );
    sample_pause_end("\n\nPress any key to continue . . . ");
  }
  rc = mco_trans_commit(t);

  printf("\nCreate employees and join each to a department:\n");
  for (i = 0; i < sizeof(employee_departments) / sizeof(employee_departments[0]) && MCO_S_OK == rc; ++i) {
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if (MCO_S_OK == rc) {
      // Find Department by code; extract autoid; create Employee and assign name, dept (autoid)
      Department dept;
      rc = Department_Icode_find(t, employee_departments[i].code, strlen(employee_departments[i].code), &dept);
      if (MCO_S_OK == rc)
      {
        autoid_t dept_id = 0;
        rc = Department_autoid_get(&dept, &dept_id);
        if (MCO_S_OK == rc)
        {
          Employee emp;
          Employee_new(t, &emp);
          Employee_name_put(&emp, employee_departments[i].name, (uint2)strlen(employee_departments[i].name));
          Employee_dept_put(&emp, dept_id);
          printf("\n\t%d) %s, Department.Autoid=%ld", i, employee_departments[i].name, (long)dept_id);
          rc = mco_trans_commit(t);
        }
        else
        {
          printf("\nUnable to get autoid for Department '%s'", employee_departments[i].code);
          rc = mco_trans_rollback(t);
        }
      }
      else
      {
        printf("\nUnable to find Department '%s'", employee_departments[i].code);
        rc = mco_trans_rollback(t);
      }
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

  /* Find co-workers of search_emp */
  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    char name[20];
    uint2 len;
    int cmp;
    autoid_t dept_id1, dept_id2;
    Department dept1;
    Employee emp, emp2;
    mco_cursor_t csr;
      
    // 1. find employee record by name
    rc = Employee_Iname_find(t, search_name, (uint2)strlen(search_name), &emp);
    if (MCO_S_OK == rc) {
      printf("\n\n%s's co-workers in ", search_name);
      Employee_dept_get(&emp, &dept_id1);

      // 2. Find the Department object by its autoid and display the Department name
      rc = Department_autoid_find(t, dept_id1, &dept1);
      if (MCO_S_OK == rc) {
        Department_name_get(&dept1, name, sizeof(name), &len);
        printf("%s are:\n", name);

        // 3. Position the cursor in the Idept_name index to the first object with this autoid
        Employee_Idept_name_index_cursor(t, &csr);
        rc = Employee_Idept_name_search(t, &csr, MCO_GE, dept_id1, "", 0);
        if (MCO_S_OK == rc) {
          while (MCO_S_OK == rc) {
            // Check if the current Employee is same as the found Employee
            Employee_from_cursor(t, &csr, &emp2);
            Employee_name_get(&emp2, name, sizeof(name), &len);
            // If the two names are not equal, display the name (if dept_id is still the same)
            if (0 != strcmp(name, search_name)) {
              // Verify that the dept_id is still the same, otherwise break out of loop
              Employee_dept_get(&emp2, &dept_id2);
              if (dept_id1 != dept_id2) break;
              printf("\n\t%s", name);
            }
            rc = mco_cursor_next(t, &csr);
          }
          printf("\n");
        }
        else {
          sample_rc_check("\nEmployee_Idept_name not found", rc);
        }
      }
    }
    else {
      sprintf(str, "Can't find '%s'", search_name);
      sample_rc_check(str, rc);
    }
  }
  mco_trans_rollback(t);

  return rc;
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
    
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, autoid_ref_db_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Fill db, then perform search for employees in same department as Alex */
      rc = fill_database(db);
      if ( MCO_S_OK == rc ) {
        rc = find_coworkers(db, "William");
      }
      /* Disconnect and close database, stop runtime */
      mco_db_disconnect(db);
    }
  
    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
