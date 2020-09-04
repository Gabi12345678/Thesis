#include "create_objects.h"

void createClassAObject( mco_trans_h t, uint4 id );
void createClassBObject( mco_trans_h t, uint4 id );



/* Insert objects into the specified class*/
void createObjects( mco_db_h db, int iClass, uint4 nObjects, uint4 nInsertsPerTransaction )
{
  mco_trans_h t;
  uint4 objectCount;

  CHECK(mco_trans_start ( db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
  for ( objectCount = 1L; objectCount <= nObjects; objectCount++)
  {
    switch ( iClass ) 
    {
    case 0:
        createClassAObject( t, objectCount );
        break;

    case 1:
        createClassBObject( t, objectCount );
        break;

    default:
        CHECK(MCO_E_ILLEGAL_PARAM);
        break;
    }

    if (objectCount % nInsertsPerTransaction == 0)
    {
        CHECK(mco_trans_commit( t ));
        CHECK(mco_trans_start( db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
    }
  }

  CHECK(mco_trans_commit( t ));
}

void createClassAObject( mco_trans_h t, uint4 id )
{
  char tag[16];
  A a;
  ClassA obj;

  CHECK( ClassA_new( t, &obj ) );
  CHECK( ClassA_a_write_handle( &obj, &a ) );
  CHECK( A_id_put( &a, id ) );
  sprintf ( tag, "%012ldtag", id );
  CHECK( A_tag_put( &a, tag, strlen(tag) ) );
  CHECK( A_ui1_put( &a, id ) );
  CHECK( A_ui2_put( &a, id ) );
  CHECK( A_ui3_put( &a, id ) );
  CHECK( A_ui4_put( &a, id ) );
  CHECK( A_ui5_put( &a, id ) );
  CHECK( A_ui6_put( &a, id ) );
  CHECK( A_ui7_put( &a, id ) );
  CHECK( A_ui8_put( &a, id ) );
  CHECK( A_ui9_put( &a, id ) );
  CHECK( A_ui10_put( &a, id ) );
  CHECK( A_ui11_put( &a, id ) );
  CHECK( A_ui12_put( &a, id ) );
  CHECK( A_ui13_put( &a, id ) );
  CHECK( A_ui14_put( &a, id ) );
  CHECK( A_ui15_put( &a, id ) );
  CHECK( A_ui16_put( &a, id ) );
  CHECK( A_ui17_put( &a, id ) );
  CHECK( A_ui18_put( &a, id ) );
  CHECK( A_ui19_put( &a, id ) );
  CHECK( A_ui20_put( &a, id ) );
  CHECK( A_ui21_put( &a, id ) );
  CHECK( A_ui22_put( &a, id ) );
  CHECK( A_ui23_put( &a, id ) );
  CHECK( A_ui24_put( &a, id ) );
  CHECK( A_ui25_put( &a, id ) );
  CHECK( A_ui26_put( &a, id ) );
  CHECK( A_ui27_put( &a, id ) );
  CHECK( A_ui28_put( &a, id ) );
  CHECK( A_ui29_put( &a, id ) );
  CHECK( A_ui30_put( &a, id ) );
}

void createClassBObject( mco_trans_h t, uint4 id )
{
  char tag[16];
  B b;
  ClassB obj;
  B_d bd;

  CHECK( ClassB_new( t, &obj ) );
  CHECK( ClassB_b_write_handle( &obj, &b ) );

  sprintf ( bd.tag, "%012ldtag", id );
  bd.id = bd.ui1 = bd.ui2 = bd.ui3 = bd.ui4 = bd.ui5 = bd.ui6 = bd.ui7 = bd.ui8 = bd.ui9 = bd.ui10 = bd.ui11 = bd.ui12 = \
    bd.ui13 = bd.ui14 = bd.ui15 = bd.ui16 = bd.ui17 = bd.ui18 = bd.ui19 = bd.ui20 = bd.ui21 = bd.ui22 = bd.ui23 = bd.ui24 = \
    bd.ui25 = bd.ui26 = bd.ui27 = bd.ui28 = bd.ui29 = bd.ui30 = id;
  CHECK( B_put_all(&b, &bd) );
}
