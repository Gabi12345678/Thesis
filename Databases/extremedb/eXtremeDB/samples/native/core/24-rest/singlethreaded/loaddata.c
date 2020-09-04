/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.    *
 *                                                              *
 ****************************************************************/
#include "common.h"
#include "mcouda.h"
#include "reststdb.h"

double rrand(double range)
{
  return (((double)rand()) / ((double)RAND_MAX))*(range - 1.0);
}

char* rand_string(char* buf, uint4 sz, unsigned int* len)
{

  int l = (int)rrand(sz - 1) + 1;
  int i;
  char* pc = buf;
  for (i = 0; i < l; i++)
  {
      pc[i] = 'a' + (char)rrand(26);
  }
  pc[l] = 0;

  if (len)
  {
      * len = l;
  }
  return pc;
}

#define ZERO_STRUCT(stru) memset(&stru,0,sizeof(stru))
MCO_RET LoadData(mco_db_h db)
{
  mco_trans_h t;
  int i;
  MCO_RET rc;

  uint2 j, k;
  Measurement obj;
  StructType stru;
  NestedType nest;

  uint8 ui8;
  mco_int8 i8;
  uint4 ui4 = 0;
  uint2 ui2 = 0;
  uint1 ui1 = 0;
  float f = 0;
  double d = 0;
  mco_time tm = 0;
  mco_date dt = 0;

  uint4 l1 = 0;
  char buf1[200];
  uint4 l2 = 0;
  char buf2[200];

  reststdb_oid oid, oid_p;

  ZERO_STRUCT(oid);
  ZERO_STRUCT(oid_p);
  ZERO_STRUCT(ui8);
  ZERO_STRUCT(i8);

  memset(&ui8, 0, sizeof(ui8));
  sprintf(oid_p.id, "parent");
  srand((uint4)time(0));

  /* let's make 10 measurements */
  for (i = 0; i < 10;)
  {
    sprintf(oid.id, "item%d", i);

    mco_uquad_increment(&ui8);
    mco_uquad_increment((uint8*) &i8);
    ui4 = (uint4)rrand(0xFFFFFFFF);
    ui2 = (uint2)rrand(0xFFFF);
    ui1 = (uint1)rrand(0xFF);
    f = (float)rrand(1000);
    d = rrand(2000);
    tm = (mco_time)time(0);
    dt = (mco_date)time(0);

    l1 = sprintf(buf1, "1 character array");
    /* 		rand_string( buf1, sizeof(buf1), &l1 ); */

    /* begin the transaction  */
    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));

    /* allocate a new object */
    CHECK(Measurement_new(t, &oid, &obj));

    /* ref */
    CHECK(Measurement_f_ref_put(&obj, &oid_p));
    memcpy(&oid_p, &oid, sizeof(oid));

    /* atomic fields */
    Measurement_f_uint8_put(&obj, ui8);
    Measurement_f_int8_put(&obj, i8);
    Measurement_f_uint4_put(&obj, ui4);
    Measurement_f_int4_put(&obj, ui4);
    Measurement_f_uint2_put(&obj, ui2);
    Measurement_f_int2_put(&obj, ui2);
    Measurement_f_uint1_put(&obj, ui1);
    Measurement_f_int1_put(&obj, ui1);
    Measurement_f_float_put(&obj, f);
    Measurement_f_double_put(&obj, d);
    Measurement_f_time_put(&obj, tm);
    Measurement_f_date_put(&obj, dt);
    Measurement_f_char10_put(&obj, buf1, (uint2)l1);
    Measurement_f_string_put(&obj, buf1, (uint2)l1);

    /* arrays */
    for (j = 0; j < 5; j++)
    {

      mco_uquad_increment(&ui8);
      mco_uquad_increment((uint8*) &i8);
      ui4++;
      ui2++;
      ui1++;
      f++;
      d++;
      tm++;
      dt++;
      buf1[0]++;

      Measurement_a_uint8_put(&obj, j, ui8);
      Measurement_a_int8_put(&obj, j, i8);
      Measurement_a_uint4_put(&obj, j, ui4);
      Measurement_a_int4_put(&obj, j, ui4);
      Measurement_a_uint2_put(&obj, j, ui2);
      Measurement_a_int2_put(&obj, j, ui2);
      Measurement_a_uint1_put(&obj, j, ui1);
      Measurement_a_int1_put(&obj, j, ui1);
      Measurement_a_float_put(&obj, j, f);
      Measurement_a_double_put(&obj, j, d);
      Measurement_a_time_put(&obj, j, tm);
      Measurement_a_date_put(&obj, j, dt);
      Measurement_a_char10_put(&obj, j, buf1, (uint2)l1);
      Measurement_a_string_put(&obj, j, buf1, (uint2)l1);
    }

    /* vectors */
    k = (uint2)(rrand(7) + 3);
    Measurement_v_uint8_alloc(&obj, k);
    Measurement_v_int8_alloc(&obj, k);
    Measurement_v_uint4_alloc(&obj, k);
    Measurement_v_int4_alloc(&obj, k);
    Measurement_v_uint2_alloc(&obj, k);
    Measurement_v_int2_alloc(&obj, k);
    Measurement_v_uint1_alloc(&obj, k);
    Measurement_v_int1_alloc(&obj, k);
    Measurement_v_float_alloc(&obj, k);
    Measurement_v_double_alloc(&obj, k);
    Measurement_v_time_alloc(&obj, k);
    Measurement_v_date_alloc(&obj, k);
    Measurement_v_char10_alloc(&obj, k);
    Measurement_v_string_alloc(&obj, k);

    for (j = 0; j < k; j++)
    {

      mco_uquad_increment(&ui8);
      mco_uquad_increment((uint8*) &i8);
      ui4++;
      ui2++;
      ui1++;
      f++;
      d++;
      tm++;
      dt++;
      buf1[0]++;

      Measurement_v_uint8_put(&obj, j, ui8);
      Measurement_v_int8_put(&obj, j, i8);
      Measurement_v_uint4_put(&obj, j, ui4);
      Measurement_v_int4_put(&obj, j, ui4);
      Measurement_v_uint2_put(&obj, j, ui2);
      Measurement_v_int2_put(&obj, j, ui2);
      Measurement_v_uint1_put(&obj, j, ui1);
      Measurement_v_int1_put(&obj, j, ui1);
      Measurement_v_float_put(&obj, j, f);
      Measurement_v_double_put(&obj, j, d);
      Measurement_v_time_put(&obj, j, tm);
      Measurement_v_date_put(&obj, j, dt);
      Measurement_v_char10_put(&obj, j, buf1, (uint2)l1);
      Measurement_v_string_put(&obj, j, buf1, (uint2)l1);
    }

    /* plain struct */
    ui4++;
    ui2++;
    ui1++;
    buf1[0]++;
    Measurement_f_struct_write_handle(&obj, &stru);
    StructType_struct_f_uint4_put(&stru, ui4);
    StructType_struct_f_uint2_put(&stru, ui2);
    StructType_struct_f_uint1_put(&stru, ui1);
    StructType_struct_f_string_put(&stru, buf1, (uint2)l1);
    l2 = sprintf(buf2, "This is a blob in the f_struct field. The blob's #%d\n", i);
    StructType_f_blob_put(&stru, buf2, l2);

    /* optional plain struct */
    if (rrand(100) >= 50)
    {
      ui4++;
      ui2++;
      ui1++;
      buf1[0]++;
      Measurement_f_struct_opt_write_handle(&obj, &stru);
      StructType_struct_f_uint4_put(&stru, ui4);
      StructType_struct_f_uint2_put(&stru, ui2);
      StructType_struct_f_uint1_put(&stru, ui1);
      StructType_struct_f_string_put(&stru, buf1, (uint2)l1);

      l2 = sprintf(buf2, "This is a blob in the f_struct_opt field. The blob's #%d\n", i);
      StructType_f_blob_put(&stru, buf2, l2);
    }

    /*  nested struct */
    Measurement_f_nested_write_handle(&obj, &nest);
    buf1[0]++;
    NestedType_nested_f_char10_put(&nest, buf1, (uint2)l1);
    for (j = 0; j < NestedType_nested_f_int1_length; j++)
    {
      ui1++;
      NestedType_nested_f_int1_put(&nest, j, ui1);
    }
    NestedType_nested_f_struct_write_handle(&nest, &stru);
    ui4++;
    ui2++;
    ui1++;
    buf1[0]++;
    StructType_struct_f_uint4_put(&stru, ui4);
    StructType_struct_f_uint2_put(&stru, ui2);
    StructType_struct_f_uint1_put(&stru, ui1);
    StructType_struct_f_string_put(&stru, buf1, (uint2)l1);
    l2 = sprintf(buf2, "This is a blob in the nested f_struct field. The blob's #%d\n", i);
    StructType_f_blob_put(&stru, buf2, l2);

    /* vector of plain structs */
    k = (uint2)(rrand(7) + 2);
    Measurement_v_struct_alloc(&obj, k);
    for (j = 0; j < k; j++)
    {
      ui4++;
      ui2++;
      ui1++;
      buf1[0]++;
      Measurement_v_struct_put(&obj, j, &stru);
      StructType_struct_f_uint4_put(&stru, ui4);
      StructType_struct_f_uint2_put(&stru, ui2);
      StructType_struct_f_uint1_put(&stru, ui1);
      StructType_struct_f_string_put(&stru, buf1, (uint2)l1);
      l2 = sprintf(buf2, "This is a blob in the v_struct field. The blob's #%dx%d\n", i, j);
      StructType_f_blob_put(&stru, buf2, l2);
    }

    /* array of plain structs */
    for (j = 0; j < Measurement_a_struct_length; j++)
    {

      ui4++;
      ui2++;
      ui1++;
      buf1[0]++;
      Measurement_a_struct_put(&obj, j, &stru);
      StructType_struct_f_uint4_put(&stru, ui4);
      StructType_struct_f_uint2_put(&stru, ui2);
      StructType_struct_f_uint1_put(&stru, ui1);
      StructType_struct_f_string_put(&stru, buf1, (uint2)l1);
      l2 = sprintf(buf2, "This is a blob in the a_struct field. The blob's #%dx%d\n", i, j);
      StructType_f_blob_put(&stru, buf2, l2);
    }

    /* blob */
    /* 	rand_string( buf1, sizeof(buf1), &l1 ); */
    l2 = sprintf(buf2, "This is a blob. The blob's #%d\n", i);
    Measurement_f_blob_put(&obj, buf2, l2);

    rc = mco_trans_commit(t);
    /* keep total count of measurement objects of 10 */
    if (rc != MCO_S_DUPLICATE)
    {
        CHECK(rc);
        i++;
    }
  }

  return MCO_S_OK;
}
