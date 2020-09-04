#include "search_delete.h"

void searchDeleteAObject( mco_trans_h t, uint4 key );
void searchDeleteBObject( mco_trans_h t, uint4 key );

/* Search and delete all objects from the specified class using the hash index*/
void searchDelete( mco_db_h db, int iClass, uint4 nObjects, uint4 nDeletesPerTransaction )
{
  mco_trans_h t;
  uint4 searchCount;

  CHECK(mco_trans_start ( db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
  for ( searchCount = 1L; searchCount <= nObjects; searchCount++)
  {
    switch ( iClass ) 
    {
    case 0:
        searchDeleteAObject( t, searchCount );
        break;

    case 1:
        searchDeleteBObject( t, searchCount );
        break;

    default:
        CHECK(MCO_E_ILLEGAL_PARAM);
        break;
    }

    if (searchCount % nDeletesPerTransaction == 0)
    {
        CHECK(mco_trans_commit( t ));
        CHECK(mco_trans_start( db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
    }
  }

  CHECK(mco_trans_commit( t ));
}

void searchDeleteAObject( mco_trans_h t, uint4 key )
{
  ClassA obj;
  /*
  char tag[16];
  A a;
  uint4 id;
  */
  CHECK( ClassA_hKey_find(t, key, &obj) );
  /*
  CHECK( ClassA_a_read_handle( &obj, &a ) );

  CHECK( A_id_get( &a, &id ) );
  CHECK( A_tag_get( &a, tag, sizeof(tag) ) );
  CHECK( A_ui1_get( &a, &id ) );
  CHECK( A_ui2_get( &a, &id ) );
  CHECK( A_ui3_get( &a, &id ) );
  CHECK( A_ui4_get( &a, &id ) );
  CHECK( A_ui5_get( &a, &id ) );
  CHECK( A_ui6_get( &a, &id ) );
  CHECK( A_ui7_get( &a, &id ) );
  CHECK( A_ui8_get( &a, &id ) );
  CHECK( A_ui9_get( &a, &id ) );
  CHECK( A_ui10_get( &a, &id ) );
  CHECK( A_ui11_get( &a, &id ) );
  CHECK( A_ui12_get( &a, &id ) );
  CHECK( A_ui13_get( &a, &id ) );
  CHECK( A_ui14_get( &a, &id ) );
  CHECK( A_ui15_get( &a, &id ) );
  CHECK( A_ui16_get( &a, &id ) );
  CHECK( A_ui17_get( &a, &id ) );
  CHECK( A_ui18_get( &a, &id ) );
  CHECK( A_ui19_get( &a, &id ) );
  CHECK( A_ui20_get( &a, &id ) );
  CHECK( A_ui21_get( &a, &id ) );
  CHECK( A_ui22_get( &a, &id ) );
  CHECK( A_ui23_get( &a, &id ) );
  CHECK( A_ui24_get( &a, &id ) );
  CHECK( A_ui25_get( &a, &id ) );
  CHECK( A_ui26_get( &a, &id ) );
  CHECK( A_ui27_get( &a, &id ) );
  CHECK( A_ui28_get( &a, &id ) );
  CHECK( A_ui29_get( &a, &id ) );
  CHECK( A_ui30_get( &a, &id ) );
  */
  CHECK( ClassA_delete(&obj) );
}

void searchDeleteBObject( mco_trans_h t, uint4 key )
{
  ClassB obj;
  /*
  char tag[16];
  B b;
  B_d bd;
  */
  CHECK( ClassB_hKey_find(t, key, &obj) );
  /*
  CHECK( ClassB_b_read_handle( &obj, &b ) );

  CHECK( B_get_all(&b, &bd) );
  */
  CHECK( ClassB_delete(&obj) );
}