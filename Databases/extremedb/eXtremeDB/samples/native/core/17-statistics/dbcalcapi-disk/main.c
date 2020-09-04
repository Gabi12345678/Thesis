#include <common.h>
#include <mcodbcalc.h>
#include "dbcalcapidb.h"

char sample_descr[] = "Sample 'dbcalcAPI' demonstrates the use of eXtremeDB calculator.\n";
const char * db_name = "dbcalcapidb";

#define DEF_BLMIN 10
#define DEF_BLMAX 20
#define DEF_VLMIN 2
#define DEF_VLMAX 30
#define DEF_SLMIN 10
#define DEF_SLMAX 128

int main(int argc, char *argv[])
{
  MCO_RET rc;
  mco_calc_t calc;
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  mco_calc_init(&calc, dbcalcapidb_get_dictionary());
  calc.dsl.b_lmin = DEF_BLMIN;
  calc.dsl.b_lmax = DEF_BLMAX;
  calc.dsl.v_lmin = DEF_VLMIN;
  calc.dsl.v_lmax = DEF_VLMAX;
  calc.dsl.s_lmin = DEF_SLMIN;
  calc.dsl.s_lmax = DEF_SLMAX;
    
  rc = sample_open_database( db_name, dbcalcapidb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &calc.db);
    if ( MCO_S_OK == rc ) {

      rc = mco_calc_reg_schema_classes(&calc, dbcalcapidb_get_calculator());
      if ( MCO_S_OK == rc ) {

        rc = mco_calc_fill_db(&calc);
        if ( MCO_S_OK == rc ) {
  
          rc = mco_calc_stat_collect(&calc);
          if ( MCO_S_OK == rc ) {
            /* Print statistics */
          }
        }
        /* NOTE: calculator must be deinitialized before you disconnect from the database */
        mco_calc_deinit(&calc);
      }
        

      /* Disconnect and close database, stop runtime */
      mco_db_disconnect(calc.db);
    }
    
    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
