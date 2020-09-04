/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "cppdemo.hpp"
char sample_descr[] = {
  "Sample 'cppdemo' demonstrates the -hpp -smartptr eXtremeDB schema compiler \noption.\n" 
};
const char* db_name = "cppdemo";

using namespace cppdemo;

class MyPart: public Part
{
  public:

    void print(mco_trans_h t);
};

void MyPart::print(mco_trans_h t)
{
  if (!is_valid())
  {
    return ;
  }

  printf("\n[%d] %s : %s\n", (uint4)code, ((std::string) name).c_str(), ((std::string) type).c_str());

  Dimensions dim;
  dim_read(dim);

  float w, h, l;
  w = dim.width;
  l = dim.length;
  h = dim.height;

  printf("%g x %g x %g \n", l, w, h);

  std::vector<uint4> options_arr;
  options_arr = options;
  printf("Options: ");
  for (int i = 0; i < Part_options_length; i++)
    printf("%d; ", options_arr[i]);
  printf("\n");
}

static int rand2(int lowlimit, int uplimit)
{
  int n = rand();
  return ((n % (uplimit - lowlimit + 1)) + lowlimit);
}

MCO_RET populate_db(mco_db_h db)
{
  mco_trans_h t = 0;
  MCO_RET rc = MCO_S_OK;
  int i, j;

  for (j = 0; j < 1000 && MCO_S_OK == rc; j++)
  {
    MyPart part;
    Dimensions dim;

    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if ( MCO_S_OK == rc ) {
      char temp[200];

      part.create(t);

      sprintf(temp, "%d", j % 100);
      part.type = (const char *)temp;

      sprintf(temp, "part # %d", j);
      part.name = (const char *)temp;

      part.price = (float)(2.0 + j % 100 / 200.0);
      part.code = 1000000+j;

      part.dim_write(dim);
      dim.height = (float)rand2(20, 40);
      dim.width = (float)rand2(20, 50);
      dim.length = (float)rand2(35, 90);

      std::vector<uint4> options_arr(Part_options_length);
      for (i = 0; i < Part_options_length; i++)
          options_arr[i] = rand2(0, 30);
      part.options = options_arr;

      rc = mco_trans_commit(t);
    }
  }

  printf("Inserted %d parts to the database\n", j);

  return rc;
}

// Print all parts with given type
MCO_RET printPartsOfType(mco_db_h db, const char* type)
{
  mco_cursor_t csr;
  mco_trans_h t;
  MCO_RET rc = MCO_S_OK;

  printf("Parts with type = %s :\n", type);

  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {

    rc = Part::ByType::cursor(t, &csr);
    if ( MCO_S_OK == rc ) {

      rc = Part::ByType::search(t, &csr, MCO_GE, type, (uint2)strlen(type), "", 0); 
                                     // Empty name here for positioning

      for (; MCO_S_OK == rc; rc = mco_cursor_next(t, &csr))
      {
        MyPart part;

        part.from_cursor(t, &csr);

        if (strcmp(((std::string) part.type).c_str(), type) != 0)
        {
          break;
        }

        part.print(t);
      }
    }

    mco_trans_rollback(t); // Read-only trn
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

  rc = sample_open_database( db_name, cppdemo_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect_ctx(db_name, 0, &db); /* No recovery connection data */
    if ( MCO_S_OK == rc ) {
#ifdef DISK_DATABASE
      mco_disk_transaction_policy(db, MCO_COMMIT_NO_SYNC);
#endif
      rc = populate_db(db);
      if ( MCO_S_OK == rc ) {
        printPartsOfType(db, "23");
      }

      rc = mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
