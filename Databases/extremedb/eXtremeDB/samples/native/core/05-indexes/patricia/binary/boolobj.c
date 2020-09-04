/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/

#include <stdio.h>
#include <string.h>
#include "binarydb.h"

unsigned char* make_mask(uint4 val, uint2 bitnum)
{
  static unsigned char _mask_[4];
  uint2 i;
  memset(_mask_, 0, sizeof(_mask_));
  for (i = 0; i < bitnum; i++, val = val >> 1)
  {
      _mask_[i >> 3] |= (val &1) << (i &7);
  }
  return _mask_;
}
uint4 make_uint(uint1* _mask_, uint2 bitnum)
{
  uint4 val = 0;
  uint2 i;
  for (i = 0; i < bitnum; i++)
  {
      val |= ((_mask_[i >> 3] >> (i &7)) &1) << i;
  }
  return val;
}

void prnObject(AreaCodeBool* areaCode)
{
  char pstr[10];
  uint4 val;
  uint1 _tmp_[sizeof(val)];
  uint2 sz;

  AreaCodeBool_areaCode_size(areaCode, &sz);
  AreaCodeBool_areaCode_get_range(areaCode, 0, sz, _tmp_);
  val = make_uint(_tmp_, sz);
  AreaCodeBool_strAreaCode_get(areaCode, pstr, 10);
  printf("\t<%x,%s>\n", val, pstr);
}

MCO_RET doListing(mco_db_h db)
{
  MCO_RET rc = MCO_S_OK;
  mco_cursor_t csr;
  mco_trans_h trn;
  AreaCodeBool areaCode;

  printf("\tPress any key to view the list of AreaCodes:\n");
  getchar();

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {

    /* initialize cursor */
    rc = AreaCodeBool_IareaCode_index_cursor(trn, &csr);
    if ( MCO_S_OK == rc ) {

      for (rc = mco_cursor_first(trn, &csr); MCO_S_OK == rc; rc = mco_cursor_next(trn, &csr))
      {
        rc = AreaCodeBool_from_cursor(trn, &csr, &areaCode);
        prnObject(&areaCode);
      }
    }
    rc = mco_trans_commit(trn);
  }

  return rc;
}

int calcBitLen(uint4 val)
{
  int i, blen = 31;
  i = blen;
  while (i)
  {
    if ((val >> i) &1)
    {
      break;
    }
    i--;
  }
  i = ((i + 3+1) / 4)* 4;
  if (!i)
  {
    return 4;
  }
  else
  {
    return i;
  }
  return 32;
}

MCO_RET AddRecord(mco_trans_h t, uint4 val, const char* pstr)
{
  MCO_RET rc;
  AreaCodeBool areaCode;
  uint2 sz;

  rc = AreaCodeBool_new(t, &areaCode);
  if ( MCO_S_OK == rc ) {
  
    rc = AreaCodeBool_strAreaCode_put(&areaCode, pstr, (uint2)(strlen(pstr) + 1));
    sz = calcBitLen(val);
    rc = AreaCodeBool_areaCode_alloc(&areaCode, sz);
    rc = AreaCodeBool_areaCode_put_range(&areaCode, 0, sz, (uint1*)make_mask(val, sz));
  }

  return rc;
}

MCO_RET createDatabase(mco_db_h db)
{
  MCO_RET rc = MCO_S_OK; 
  mco_trans_h t;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {

    AddRecord(t, 0x1, "ATT");
    AddRecord(t, 0x20, "BCC");
    AddRecord(t, 0x25, "TNT");
    AddRecord(t, 0x3, "ANC");
    AddRecord(t, 0x355, "NCC");
    AddRecord(t, 0x355, "UDC");
    AddRecord(t, 0x45, "WTC");
    AddRecord(t, 0x454, "FCC");
    AddRecord(t, 0x5, "ABB");
    AddRecord(t, 0x51, "DDA");
    AddRecord(t, 0x51, "FIY");
    AddRecord(t, 0x513, "ASD");

    AddRecord(t, 0x515, "ASH");
    AddRecord(t, 0x5183, "ASW");
    AddRecord(t, 0x525, "ASY");

    AddRecord(t, 0x6, "YUI");
    AddRecord(t, 0x64, "KEW");
    AddRecord(t, 0x66, "WER");
    AddRecord(t, 0x668, "VBR");
    AddRecord(t, 0x668, "MPP");
    AddRecord(t, 0x668, "FCA");
    AddRecord(t, 0x6689, "FCU");
    AddRecord(t, 0x699, "FCH");
    AddRecord(t, 0x6683, "FCL");
    AddRecord(t, 0x6685, "FCD");
    AddRecord(t, 0x6687, "FCA");
    AddRecord(t, 0x7, "MNM");
    AddRecord(t, 0x7, "MGM");
    AddRecord(t, 0x79, "MSM");
    AddRecord(t, 0x795, "EAE");
    AddRecord(t, 0x81, "YUI");
    AddRecord(t, 0x84, "WWW");
    AddRecord(t, 0x849, "OLA");
    AddRecord(t, 0x9, "GEA");
    AddRecord(t, 0x99, "YTR");
    AddRecord(t, 0x99, "IUI");
    AddRecord(t, 0x988, "JHJ");
    AddRecord(t, 0x988, "DFD");

    rc = mco_trans_commit(t);
  }

  return rc;
}

MCO_RET searchExact(mco_db_h db, uint4 AreaCode)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h trn;
  AreaCodeBool areaCode;
  mco_cursor_t csr;
  uint2 sz;

  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {

    rc = AreaCodeBool_IareaCode_index_cursor(trn, &csr);
    sz = calcBitLen(AreaCode);
    rc = AreaCodeBool_IareaCode_exact_match(trn, &csr, (char*)make_mask(AreaCode, sz), sz);
    if ( MCO_S_OK == rc )
    {
      printf("\tFound ExactMatch for key %x:\n", AreaCode);
      while ( MCO_S_OK == rc )
      {
        rc = AreaCodeBool_from_cursor(trn, &csr, &areaCode);
        prnObject(&areaCode);

        rc = AreaCodeBool_IareaCode_next_match(trn, &csr, (char*)make_mask(AreaCode, sz), sz);
      }
    }
    else
    {
      printf("\tExactMatch not found for key %x:\n", AreaCode);
    }

    rc = mco_trans_commit(trn);
  }

  return rc;
}

MCO_RET searchPrefix(mco_db_h db, uint4 AreaCodePref)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h trn;
  AreaCodeBool areaCode;
  mco_cursor_t csr;
  uint2 sz;
  uint4 key = AreaCodePref;

  mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {

    rc = AreaCodeBool_IareaCode_index_cursor(trn, &csr);
    sz = calcBitLen(AreaCodePref);
    rc = AreaCodeBool_IareaCode_prefix_match(trn, &csr, (char*)make_mask(key, sz), sz);
    if ( MCO_S_OK == rc )
    {
      int found = 0;

      while ( MCO_S_OK == rc )
      {
        if (!found)
        {
          printf("\tFound PrefixMatch for key %x:\n", AreaCodePref);
        }
        found = 1;

        rc = AreaCodeBool_from_cursor(trn, &csr, &areaCode);
        prnObject(&areaCode);
        rc = AreaCodeBool_IareaCode_next_match(trn, &csr, (char*)make_mask(key, sz), sz);
      }
      if (!found)
      {
        printf("\tPrefixMatch not found for key %x:\n", AreaCodePref);
      }
    }
    else
    {
      printf("\tPrefixMatch not found for key %x:\n", AreaCodePref);
    }

    rc = mco_trans_commit(trn);
  }

  return rc;
}

MCO_RET searchLongest(mco_db_h db, uint4 AreaCodePref)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h trn;
  AreaCodeBool areaCode;
  mco_cursor_t csr;
  uint2 sz;
  uint4 key = AreaCodePref;

  mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {

    rc = AreaCodeBool_IareaCode_index_cursor(trn, &csr);
    sz = calcBitLen(AreaCodePref);
    rc = AreaCodeBool_IareaCode_longest_match(trn, &csr, (char*)make_mask(key, sz), sz);
    if ( MCO_S_OK == rc )
    {
      int found = 0;

      while ( MCO_S_OK == rc )
      {
        if (!found)
        {
          printf("\tFound LongestMatch for key %x:\n", AreaCodePref);
        }
        found = 1;

        rc = AreaCodeBool_from_cursor(trn, &csr, &areaCode);
        prnObject(&areaCode);

        rc = AreaCodeBool_IareaCode_next_match(trn, &csr, (char*)make_mask(key, sz), sz);

      }
      if (!found)
      {
        printf("\tLongestMatch not found for key %x:\n", AreaCodePref);
      }
    }
    else
    {
      printf("\tLongestMatch not found for key %x:\n", AreaCodePref);
    }

    rc = mco_trans_commit(trn);
  }
  return rc;
}

MCO_RET doGoogle(mco_db_h db)
{
  MCO_RET rc = MCO_S_OK;

  /*////////////////////////////////////////////////////////////////////////*/
  /* Exact search*/

  printf("\n\t//////////////////////////////////////\n");
  printf("\tPress any key to view the Exact Match function demonstrations:");
  getchar();
  printf("\n\n");

  searchExact(db, 0x1);
  searchExact(db, 0x55);
  searchExact(db, 0x5);
  searchExact(db, 0x51);
  searchExact(db, 0x6);
  searchExact(db, 0x7);

  /*////////////////////////////////////////////////////////////////////////*/
  /* prefixSearch*/

  printf("\n\t//////////////////////////////////////\n");
  printf("\tPress any key to view the Prefix Match function demonstrations:");
  getchar();
  printf("\n\n");

  searchPrefix(db, 0x190);
  searchPrefix(db, 0x55);
  searchPrefix(db, 0x589);
  searchPrefix(db, 0x51);
  searchPrefix(db, 0x511);
  searchPrefix(db, 0x5189);
  searchPrefix(db, 0x668889);
  searchPrefix(db, 0x698889);
  searchPrefix(db, 0x71111111);


  /*////////////////////////////////////////////////////////////////////////*/
  /* longestSearch*/

  printf("\n\t//////////////////////////////////////\n");
  printf("\tPress any key to view the Longest Match function demonstrations:");
  getchar();
  printf("\n\n");

  searchLongest(db, 0x190);
  searchLongest(db, 0x55);
  searchLongest(db, 0x589);
  searchLongest(db, 0x5189);
  searchLongest(db, 0x51);
  searchLongest(db, 0x511);
  searchLongest(db, 0x668889);
  searchLongest(db, 0x698889);
  searchLongest(db, 0x71111111);

  printf("\n");

  return rc;
}
