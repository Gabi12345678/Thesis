#include "sequential.h"

void scanAObjects( mco_trans_h t );
void scanBObjects( mco_trans_h t );

/* Sequential scan objects from the specified class using the tree index*/
void sequentialScan( mco_db_h db, int iClass )
{
  mco_trans_h t;
  uint4 searchCount;

  CHECK(mco_trans_start ( db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t ));
  switch ( iClass ) 
  {
  case 0:
      scanAObjects( t );
      break;

  case 1:
      scanBObjects( t );
      break;

  default:
      CHECK(MCO_E_ILLEGAL_PARAM);
      break;
  }

  CHECK(mco_trans_commit( t ));
}

void scanAObjects( mco_trans_h t )
{
  char tag[16];
  A a;
  ClassA obj;
  uint4 id;
  mco_cursor_t c;
  MCO_RET rc;

  CHECK( ClassA_tKey_index_cursor(t, &c) );
  CHECK( mco_cursor_first(t, &c) );
  for (rc = MCO_S_OK; rc == MCO_S_OK; rc = mco_cursor_next(t, &c))
  {
    CHECK( ClassA_from_cursor(t, &c, &obj) );
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
  }
}

void scanBObjects( mco_trans_h t )
{
  char tag[16];
  B b;
  ClassB obj;
  B_d bd;
  mco_cursor_t c;
  MCO_RET rc;

  CHECK( ClassB_tKey_index_cursor(t, &c) );
  CHECK( mco_cursor_first(t, &c) );
  for (rc = MCO_S_OK; rc == MCO_S_OK; rc = mco_cursor_next(t, &c))
  {
    CHECK( ClassB_from_cursor(t, &c, &obj) );
    CHECK( ClassB_b_read_handle( &obj, &b ) );

    CHECK( B_get_all(&b, &bd) );
  }
}
