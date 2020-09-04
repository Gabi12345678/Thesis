/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/

#include <stdio.h>
#include <string.h>
#include "characterdb.h"

void prnObject(AreaCode* areaCode)
{
  char pstr[10];
  char val[20];
  uint2 sz;

  memset(val, 0, 20);
  AreaCode_areaCode_get(areaCode, val, 20, &sz);
  AreaCode_strAreaCode_get(areaCode, pstr, 10);
  printf("\t<%s,%s>\n", val, pstr);
}

MCO_RET doListing(mco_db_h db)
{
  MCO_RET rc = MCO_S_OK;
  mco_cursor_t csr;
  mco_trans_h trn;
  AreaCode areaCode;

  printf("\tPress any key to view the list of AreaCodes:\n");
  getchar();

  mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {

    /* initialize cursor */
    rc = AreaCode_IareaCode_index_cursor(trn, &csr);
    if ( MCO_S_OK == rc ) {

      for (rc = mco_cursor_first(trn, &csr); MCO_S_OK == rc; rc = mco_cursor_next(trn, &csr))
      {
        rc = AreaCode_from_cursor(trn, &csr, &areaCode);
        prnObject(&areaCode);
      }
    }
    mco_trans_commit(trn);
  }

  return rc;
}

MCO_RET AddRecord(mco_trans_h t, const char* val, const char* pstr)
{
  MCO_RET rc;
  AreaCode areaCode;

  rc = AreaCode_new(t, &areaCode);
  if ( MCO_S_OK == rc ) {
    rc = AreaCode_strAreaCode_put(&areaCode, pstr, (uint2)(strlen(pstr) + 1));
    rc = AreaCode_areaCode_put(&areaCode, val, (uint2)(strlen(val)));
  }

  return rc;
}

MCO_RET createDatabase(mco_db_h db)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h t;

  mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    AddRecord(t, "01", "ATT");
    AddRecord(t, "020", "BCC");
    AddRecord(t, "025", "TNT");
    AddRecord(t, "03", "ANC");
    AddRecord(t, "0355", "NCC");
    AddRecord(t, "0355", "UDC");
    AddRecord(t, "045", "WTC");
    AddRecord(t, "0454", "FCC");
    AddRecord(t, "05", "ABB");
    AddRecord(t, "051", "DDA");
    AddRecord(t, "051", "FIY");
    AddRecord(t, "0513", "ASD");
    AddRecord(t, "0515", "ASH");
    AddRecord(t, "05183", "ASW");
    AddRecord(t, "0525", "ASP");
    AddRecord(t, "06", "YUI");
    AddRecord(t, "064", "KEW");
    AddRecord(t, "066", "WER");
    AddRecord(t, "0668", "VBR");
    AddRecord(t, "0668", "MPP");
    AddRecord(t, "0668", "FCA");
    AddRecord(t, "06689", "FCU");
    AddRecord(t, "0699", "FCH");
    AddRecord(t, "06683", "FCL");
    AddRecord(t, "06685", "FCD");
    AddRecord(t, "06687", "FCA");
    AddRecord(t, "07", "MNM");
    AddRecord(t, "07", "MGM");
    AddRecord(t, "079", "MSM");
    AddRecord(t, "0795", "EAE");
    AddRecord(t, "081", "YUI");
    AddRecord(t, "084", "WWW");
    AddRecord(t, "0849", "OLA");
    AddRecord(t, "09", "GEA");
    AddRecord(t, "099", "YTR");
    AddRecord(t, "099", "IUI");
    AddRecord(t, "0988", "JHJ");
    AddRecord(t, "0988", "DFD");

    rc = mco_trans_commit(t);
  }

  return rc;
}

MCO_RET searchExact(mco_db_h db, const char* strAreaCode)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h trn;
  AreaCode areaCode;
  mco_cursor_t csr;

  mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {

    rc = AreaCode_IareaCode_index_cursor(trn, &csr);
    rc = AreaCode_IareaCode_exact_match(trn, &csr, strAreaCode, (uint2)(strlen(strAreaCode)));
    if (rc == MCO_S_OK)
    {
      printf("\tFound ExactMatch for key %s:\n", strAreaCode);
      while ( MCO_S_OK == rc )
      {
        rc = AreaCode_from_cursor(trn, &csr, &areaCode);
        prnObject(&areaCode);

        rc = AreaCode_IareaCode_next_match(trn, &csr, strAreaCode, (uint2)(strlen(strAreaCode)));
      }
    }
    else
    {
      printf("\tExactMatch not found for key %s:\n", strAreaCode);
   }

   rc = mco_trans_commit(trn);
  }

  return rc;
}

MCO_RET searchPrefix(mco_db_h db, const char* strAreaCodePref)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h trn;
  AreaCode areaCode;
  mco_cursor_t csr;
  uint2 sz = strlen(strAreaCodePref);

  mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {
    rc = AreaCode_IareaCode_index_cursor(trn, &csr);
    rc = AreaCode_IareaCode_prefix_match(trn, &csr, strAreaCodePref, sz);
    if ( MCO_S_OK == rc )
    {
      int found = 0;

      while ( MCO_S_OK == rc )
      {
        if (!found)
        {
          printf("\tFound PrefixMatch for key %s:\n", strAreaCodePref);
        }
        found = 1;

        rc = AreaCode_from_cursor(trn, &csr, &areaCode);
        prnObject(&areaCode);
        rc = AreaCode_IareaCode_next_match(trn, &csr, strAreaCodePref, sz);
      }
      if (!found)
      {
        printf("\tPrefixMatch not found for key %s:\n", strAreaCodePref);
      }
    }
    else
    {
      printf("\tPrefixMatch not found for key %s:\n", strAreaCodePref);
    }

    rc = mco_trans_commit(trn);
  }

  return rc;
}

MCO_RET searchLongest(mco_db_h db, const char* strAreaCodePref)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h trn;
  AreaCode areaCode;
  mco_cursor_t csr;
  uint2 sz = strlen(strAreaCodePref);

  mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {

    rc = AreaCode_IareaCode_index_cursor(trn, &csr);
    rc = AreaCode_IareaCode_longest_match(trn, &csr, strAreaCodePref, sz);
    if (rc == MCO_S_OK)
    {
      int found = 0;

      while ( MCO_S_OK == rc )
      {
        if (!found)
        {
          printf("\tFound LongestMatch for key %s:\n", strAreaCodePref);
        }
        found = 1;

        rc = AreaCode_from_cursor(trn, &csr, &areaCode);
        prnObject(&areaCode);

        rc = AreaCode_IareaCode_next_match(trn, &csr, strAreaCodePref, sz);
      }
      if (!found)
      {
        printf("\tLongestMatch not found for key %s:\n", strAreaCodePref);
      }
    }
    else
    {
      printf("\tLongestMatch not found for key %s:\n", strAreaCodePref);
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

  searchExact(db, "01");
  searchExact(db, "055");
  searchExact(db, "5");
  searchExact(db, "05");
  searchExact(db, "06");
  searchExact(db, "07");

  /*////////////////////////////////////////////////////////////////////////*/
  /* prefixSearch*/

  printf("\n\t//////////////////////////////////////\n");
  printf("\tPress any key to view the Prefix Match function demonstrations:");
  getchar();
  printf("\n\n");

  searchPrefix(db, "0190");
  searchPrefix(db, "055");
  searchPrefix(db, "589");
  searchPrefix(db, "051");
  searchPrefix(db, "0511");
  searchPrefix(db, "05189");
  searchPrefix(db, "0668889");
  searchPrefix(db, "0698889");
  searchPrefix(db, "071111111");

  /*////////////////////////////////////////////////////////////////////////*/
  /* longestSearch*/

  printf("\n\t//////////////////////////////////////\n");
  printf("\tPress any key to view the Longest Match function demonstrations:");
  getchar();
  printf("\n\n");

  searchLongest(db, "0190");
  searchLongest(db, "055");
  searchLongest(db, "589");
  searchLongest(db, "051");
  searchLongest(db, "0511");
  searchLongest(db, "05189");
  searchLongest(db, "0668889");
  searchLongest(db, "0698889");
  searchLongest(db, "071111111");

  printf("\n");

  return rc;

}
