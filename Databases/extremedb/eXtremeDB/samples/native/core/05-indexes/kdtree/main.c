/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "kdtreedb.h"

char sample_descr[] = { "Sample 'kdtree' demonstrates kdtree index operations.\n" };
const char * db_name = "kdtree";

#define MAX_STR_LEN  32

typedef struct {
  char vendor[MAX_STR_LEN];
  char model[MAX_STR_LEN];
  char color[MAX_STR_LEN];
  uint4 year;
  uint4   mileage;
  uint1   automatic;
  uint1   ac;
  uint4   price;
  char state[3];
} car_info_t;

car_info_t cars_info[] = {
  {"Ford", "Mustang", "grey", 2005, 60000, MCO_YES, MCO_YES, 20000, "MT"},
  {"Ford", "Explorer", "black", 2000, 80000, MCO_YES, MCO_YES, 15000, "MA"},
  {"Toyota", "Corolla", "green", 2007, 100000, MCO_YES, MCO_YES, 12000, "CA"},
};

void print_car(char * head, Car* car)
{
  char  vendor[MAX_STR_LEN], model[MAX_STR_LEN], color[MAX_STR_LEN];
  uint4 year, mileage, price;
  uint2 len;

  Car_vendor_get(car, vendor, MAX_STR_LEN, &len);
  Car_model_get(car, model, MAX_STR_LEN, &len);
  Car_color_get(car, color, MAX_STR_LEN, &len);
  Car_year_get(car, &year);
  Car_mileage_get(car, &mileage);
  Car_price_get(car, &price);
  printf("\n%s) %s %s, color: %s, year: %d, mileage: %d, price: %d",
         head, vendor, model, color, year, mileage, price);
}
           
void fill_database(mco_db_h db)
{
  MCO_RET rc;
  mco_trans_h t;
  Car car;
  unsigned int i;
  char str[10];

  /* Store cars info and display list */
  printf("\n\tFilling database with cars...\n" );
  for (i = 0; i < sizeof(cars_info) / sizeof(cars_info[0]); ++i) {
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if ( MCO_S_OK == rc ) {
      Car_new(t, &car);
      Car_vendor_put(&car, cars_info[i].vendor, (uint2)strlen(cars_info[i].vendor));
      Car_model_put(&car,  cars_info[i].model, (uint2)strlen(cars_info[i].model));
      Car_color_put(&car, cars_info[i].color, (uint2)strlen(cars_info[i].color));
      Car_year_put(&car, cars_info[i].year);
      Car_mileage_put(&car, cars_info[i].mileage);
      Car_automatic_put(&car, cars_info[i].automatic);
      Car_ac_put(&car, cars_info[i].ac);
      Car_price_put(&car, cars_info[i].price);
      Car_state_put(&car, cars_info[i].state, (uint2)strlen(cars_info[i].state));
      sprintf( str, "%d", i );
      print_car( str, &car );

      rc = mco_trans_commit(t);
      if ( MCO_S_OK != rc ) {
        sample_rc_check("\tUnable to commit transaction", rc );
        sample_pause_end("\n\nPress any key to continue . . . ");
        sample_os_shutdown();
        dbg_exit(1);
      }
    } else {
      sample_rc_check("\tUnable to start transaction", rc );
      sample_pause_end("\n\nPress any key to continue . . . ");
      sample_os_shutdown();
      dbg_exit(1);
    }
  }
}

void simple_query_by_example(mco_db_h db)
{
  mco_trans_h t;
  mco_cursor_t cursor;
  Car car;
  MCO_RET rc;

  /* Use read-write transaction to store a pattern in the database, then find all matches */
  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    Car_new(t, &car); /* Create pattern object */
    Car_vendor_put(&car, "Ford", 4);
    Car_model_put(&car, "Mustang", 7);
    Car_index_index_cursor(t, &cursor);

    printf("\n\n\tStrict match results for vendor='Ford',model='Mustang':\n");
    rc = Car_index_search(t, &cursor, &car, &car);
    while ( MCO_S_OK == rc ) 
    {
      Car choice;
      Car_from_cursor(t, &cursor, &choice);
      print_car("(found", &choice);        
      rc = mco_cursor_next(t, &cursor);
    }
    Car_delete(&car); /* Delete pattern */
    mco_trans_commit(t);
  } else {
    sample_rc_check("\tUnable to start transaction", rc );
    sample_pause_end("\n\nPress any key to continue . . . ");
    sample_os_shutdown();
    dbg_exit(1);
  }
}

void range_query_by_example(mco_db_h db)
{
  mco_trans_h t;
  mco_cursor_t cursor;
  MCO_RET rc;
  Car from, to;
  int i;
  char str[10];
    
  /* Use read-write transaction to store boundary patterns in the database */
  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if (rc == MCO_S_OK) {
    Car_new(t, &from); /* Create low boundary pattern object */
    Car_new(t, &to);   /* Create high boundary pattern object */

    Car_vendor_put(&from, "Ford", 4); Car_vendor_put(&to, "Ford", 4);
    Car_price_put(&to, 30000);
    Car_year_put(&from, 2000); Car_year_put(&to, 2006);
    Car_mileage_put(&to, 100000);

    printf("\n\n\tRange results for:");
    print_car("\t(from", &from );
    print_car("\t(to", &to );
    printf("\n");

    Car_index_index_cursor(t, &cursor);
    rc = Car_index_search(t, &cursor, &from, &to);
    for ( i=1; MCO_S_OK == rc; i++ ) {
      Car choice;
      Car_from_cursor(t, &cursor, &choice);
      sprintf( str, "%d", i );
      print_car( str, &choice);        
      rc = mco_cursor_next(t, &cursor);
    }
    Car_delete(&from); /* Delete pattern */
    Car_delete(&to); /* Delete pattern */
    mco_trans_commit(t);
  } else {
    sample_rc_check("\tUnable to start transaction", rc );
    sample_pause_end("\n\nPress any key to continue . . . ");
    sample_os_shutdown();
    dbg_exit(1);
  }
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
  rc = sample_open_database( db_name, kdtreedb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {
   
      /* Fill db, then perform simple and range 'query by example' */
      fill_database(db);
      simple_query_by_example(db);
      range_query_by_example(db);

      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
