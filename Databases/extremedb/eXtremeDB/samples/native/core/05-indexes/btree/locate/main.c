/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.    *
 *                                                              *
 ****************************************************************/

#include <common.h>
#include "locatedb.h"

char sample_descr[] = "Sample 'locate' demonstrates _locate and _compare cursor operations.\n";
const char * db_name = "locate";

struct employee_data {
  char name[10];
  uint2 dept_no;
};

struct employee_data employees[] = {
  { "John", 1 }, { "Samuel", 1}, { "Thomas", 1 }, { "David", 2 }, { "James", 2}, { "Robert", 2},
  { "William", 3}, { "Kevin", 3}, { "Alex", 3}, { "Daniel", 3}, { "Diego", 3},  { "Brandon", 4 }
};    

MCO_RET fill_database(mco_db_h db)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h t;
  unsigned int i;

  printf("\n\tFilling database:\n");
  for (i = 0; i < sizeof(employees) / sizeof(employees[0]) && MCO_S_OK == rc; ++i) {
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if ( MCO_S_OK == rc )  {
      Employee emp;
      Employee_new(t, &emp);
      Employee_name_put(&emp, employees[i].name, (uint2)strlen(employees[i].name));
      Employee_dept_no_put(&emp, employees[i].dept_no);
      printf("\n\t%d) %s, Department: %d", i, employees[i].name, employees[i].dept_no);
      rc = mco_trans_commit(t);
    }
  }
  if ( MCO_S_OK != rc ) {
    sample_rc_check("\tUnable to insert record", rc );
    sample_pause_end("\n\nPress any key to continue . . . ");
  }
  return rc;
}

MCO_RET find_coworkers(mco_db_h db, char * search_emp)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h t;
  char str[100];

  /* Find co-workers of search_emp */
  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    char name[10];
    uint2 len;
    int cmp;
    uint2 dept_no;
    Employee emp, emp2;
    mco_cursor_t csr;
      
  /* 1. find employee record by name */
    rc = Employee_Iname_find(t, search_emp, (uint2)strlen(search_emp), &emp);
    if ( MCO_S_OK == rc ) {
      printf("\n\n\tFind %s's co-workers :\n", search_emp);
      Employee_dept_no_get(&emp, &dept_no);
      Employee_Idept_index_cursor(t, &csr);
      
      /* 2. locate cursor in the Idept index */
      rc = Employee_Idept_locate(t, &csr, &emp);
      if ( MCO_S_OK == rc ) {
        while ( MCO_S_OK == rc ) {
          /* 3. scroll forward through cursor and display names found... */
          rc = mco_cursor_next(t, &csr);
          if ( MCO_S_OK == rc ) {
            rc = Employee_Idept_compare(t, &csr, dept_no, &cmp);
            if ( MCO_S_OK != rc || cmp != 0 ) break;
            Employee_from_cursor(t, &csr, &emp2);
            Employee_name_get(&emp2, name, sizeof(name), &len);
            printf("\n\t%s", name);
          } else {
            sample_rc_check("\nmco_cursor_next forward loop", rc );
          }
        }
        rc = Employee_Idept_locate(t, &csr, &emp);
        if ( MCO_S_OK == rc ) {
          while ( MCO_S_OK == rc ) {
           /* 4. scroll backward through cursor and display names found... */
           rc = mco_cursor_prev(t, &csr);
            if ( MCO_S_OK == rc ) {
              rc = Employee_Idept_compare(t, &csr, dept_no, &cmp);
              if ( MCO_S_OK != rc || cmp != 0) break;
              Employee_from_cursor(t, &csr, &emp2);
              Employee_name_get(&emp2, name, sizeof(name), &len);
              printf("\n\t%s", name);
            } else {
              sample_rc_check("\nmco_cursor_prev back loop", rc );
            }
          }
        } else {
          sample_rc_check("\nEmployee_Idept_locate back loop", rc );
        }
      } else {
        sample_rc_check("\nEmployee_Idept_locate forward loop", rc );
      }
    } else {
      sprintf( str, "Can't find '%s'", search_emp);
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

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, locatedb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Fill db, then perform search for employees in same department as Alex */
      rc = fill_database(db);
      if ( MCO_S_OK == rc ) {
        rc = find_coworkers(db, "Alex");
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
