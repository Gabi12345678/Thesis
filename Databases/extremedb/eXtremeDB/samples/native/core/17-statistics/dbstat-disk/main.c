/*****************************************************************
 *                                                               *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.     *
 *                                                               *
 *****************************************************************/
#include <common.h>
#include "dbstatdb.h"
char sample_descr[] = {
  "Sample 'dbstat' demonstrates how to obtain runtime statistics.\n"
};
const char * db_name = "dbstatdb";

MCO_RET load_data(mco_db_h db)
{
  mco_trans_h t;
  int i;
  MCO_RET rc = MCO_S_OK;

  printf("\n\tInsert 1000 test records\n");
  for (i = 0; i < 123 && MCO_S_OK == rc; i++)
  {
    HashedObj o;
    
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if ( MCO_S_OK == rc ) {
      HashedObj_new( t, &o );
      HashedObj_key_put( &o, i );
      
      rc = mco_trans_commit(t);
    }
  }
  if ( MCO_S_OK == rc ) {
    for (i = 0; i < 255 && MCO_S_OK == rc; i++)
    {
      TriedObj o;
      
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        TriedObj_new( t, &o );
        TriedObj_key_put( &o, i );          
        rc = mco_trans_commit(t);
      }
    }
  }
  if ( MCO_S_OK == rc ) {
    for (i = 0; i < 76 && MCO_S_OK == rc; i++)
    {
      ListedObj o;
      
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        ListedObj_new( t, &o );
        ListedObj_key_put( &o, i );
        
        rc = mco_trans_commit(t);
      }
    }
  }
  if ( MCO_S_OK == rc ) {
    for (i = 0; i < 333 && MCO_S_OK == rc; i++)
    {
      AutoidedObj o;
      
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        AutoidedObj_new( t, &o );
        AutoidedObj_key_put( &o, i );
        
        rc = mco_trans_commit(t);
      }
    }
  }
  if ( MCO_S_OK == rc ) {
    for (i = 0; i < 26 && MCO_S_OK == rc; i++)
    {
      OIDedObj o;
      dbstatdb_oid oid;
      
      oid.key = i;
      
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        OIDedObj_new( t, &oid, &o );
        OIDedObj_key_put( &o, i );
        
        rc = mco_trans_commit(t);
      }
    }
  }
  if ( MCO_S_OK == rc ) {
    for (i = 0; i < 192 && MCO_S_OK == rc; i++)
    {
      //ComplexObj o;
      dbstatdb_oid oid;
      
      oid.key = i;
      
      //rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      //if ( MCO_S_OK == rc ) {
      //  ComplexObj_new( t, &oid, &o );
      //  ComplexObj_key_put( &o, i );
      //  
      //  rc = mco_trans_commit(t);
      //}
    }
  }
	
  return rc;
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
  mco_trans_h   t;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, dbstatdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      rc = load_data( db );
      if ( MCO_S_OK == rc ) {
        uint2            j    = 1;
        mco_class_stat_t stat;
    
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {

          while ( MCO_S_OK == mco_class_stat_get( t, j, &stat ) ) {

            char * name = "";

            switch ( j ) {
              case AutoidedObj_code: name = "AutoidedObj";break; 
              case ComplexObj_code:  name = "ComplexObj";break; 
              case HashedObj_code:   name = "HashedObj";break; 
              case ListedObj_code:   name = "ListedObj";break; 
              case OIDedObj_code:    name = "OIDedObj";break; 
              case TriedObj_code:    name = "TriedObj";break; 
            }
            
            printf( "%25s %4d object(s), %4d page(s), %4d blob page(s), %4d sequence page(s)\n", 
                            name, (int)stat.objects_num, stat.core_pages, stat.blob_pages, stat.seq_pages );

            j++;
          }

          mco_trans_commit(t);
        }
      
        printf("\n");

        {
          mco_index_stat_t stat;
          uint2            cnt, j;

          rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc ) {
            
            mco_index_stat_num( t, &cnt );
            
            for ( j=0; j < cnt && MCO_S_OK == rc; j++ ) {
              rc = mco_index_stat_get( t, j, &stat );
              if ( MCO_S_OK == rc  ) {

                printf( "%25s %4d key(s) %4d page(s)\n\t\t\t", stat.plabel, (int)stat.keys_num, (int)stat.pages_num );
                
                /* Type */
                if ( (stat.type & MCO_IDXST_TYPE_MASK) == MCO_IDXST_TYPE_MEM )
                    printf("inmem ");
                if ( (stat.type & MCO_IDXST_TYPE_MASK) == MCO_IDXST_TYPE_DISK )
                    printf("disk ");
              
                /* Nature */
                switch ( (stat.type & MCO_IDXST_NATURE_MASK) ) {
                  case MCO_IDXST_NATURE_BTREE: 
                    printf( "btree " );
                    break;
                  case MCO_IDXST_NATURE_PTREE:
                    printf( "ptree " );
                    break;
                  case MCO_IDXST_NATURE_KDTREE: 
                    printf( "kdtree " );
                    break;
                  case MCO_IDXST_NATURE_HASH:
                    printf( "hash  " );
                    break;
                  case MCO_IDXST_NATURE_META:
                    printf( "meta  " );
                    break;
                }

                /* Features */
                if ( (stat.type & MCO_IDXST_FEATURE_UNIQUE ) != 0 )
                    printf("unique ");
                if ( (stat.type & MCO_IDXST_FEATURE_UDF ) != 0 )
                    printf("userdef ");

                /* Function */
                switch ( (stat.type & MCO_IDXST_FUNCTION_MASK ) ) {
                  case MCO_IDXST_FUNCTION_REGULAR:
                    printf( "Regular " );
                    break;
                  case MCO_IDXST_FUNCTION_OID:
                    printf( "OID " );
                    break;
                  case MCO_IDXST_FUNCTION_AUTOOID:
                    printf( "AUTOOID " );
                    break;
                  case MCO_IDXST_FUNCTION_AUTOID:
                    printf( "AutoID " );
                    break;
                  case MCO_IDXST_FUNCTION_LIST:
                    printf( "List " );
                    break;
                }
                
                /* Specific characteristics */
                switch ( (stat.type & MCO_IDXST_NATURE_MASK) ) {
                  case MCO_IDXST_NATURE_BTREE:
                    printf( "levels=%d ", (int)stat.spec.btree.levels_num );
                    break;
                  case MCO_IDXST_NATURE_PTREE:
                    printf( "levels=%d ", (int)stat.spec.ptree.levels_num );
                    break;
                  case MCO_IDXST_NATURE_KDTREE:
                    printf( "levels=%d ", (int)stat.spec.kdtree.levels_num );
                    break;
                  case MCO_IDXST_NATURE_HASH:
                    printf( "max chain length=%d ", (int)stat.spec.hash.max_chain_length );
                    break;
                  case MCO_IDXST_NATURE_META:
                    printf( "%d disk pages ", (int)stat.spec.meta.disk_pages_num );
                    break;
                }

                printf( "\n\n" );
              }
            }

            mco_trans_commit(t);
          }
        }

      }
  
      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}
