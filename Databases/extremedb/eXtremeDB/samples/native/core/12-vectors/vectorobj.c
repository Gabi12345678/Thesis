/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mco.h>
#include "vectors.h"

static uint2 rand2(uint2 lowlimit, uint2 uplimit)
{
  int n = rand();
  return (n % (uplimit - lowlimit + 1)) + lowlimit;
}

static void get_random_owner(char* to, int maxlen)
{
  static const char* owners[5] = 
  {
    "Mike Root", "John Finger", "Peter User", "Samuel Melon", "Brian Smart"
  };

  strncpy(to, owners[rand2(0, 4)], maxlen);
}

static void get_random_zip(char* zip)
{
  static const char* zipcodes[10] = 
  {
    "22333", "12545", "90002", "20166", "33310", "80765", "13131", "11505", "14088", "48821"
  };

  memcpy(zip, zipcodes[rand2(0, 9)], 5);
}

static uint2 next_item_id = 1000;

MCO_RET create_box(mco_trans_h t, uint4 volume, const char* name)
{
  /* attributes and items are generated randomly */
  MCO_RET rc;
  Box box;

  rc = Box_new(t, &box);
  if ( MCO_S_OK == rc ) {

    Box_name_put(&box, name, (uint2)strlen(name));
    Box_volume_put(&box, volume);
    {
       /* set owners: */
      uint2 nowners = rand2(1, 10);
      uint2 n;
      char owner_name[40] = 
      {
        0
      };

      Box_owners_alloc(&box, nowners);

      for (n = 0; n < nowners; n++)
      {
        get_random_owner(owner_name, sizeof(owner_name) - 1);
        Box_owners_put(&box, n, owner_name, (uint2)strlen(owner_name));
      }
    }

    /*  set repair years */
    {
      uint2 nyears = rand2(0, 20);
      uint2 n;

      Box_repairYears_alloc(&box, nyears);

      for (n = 0; n < nyears; n++)
      {
        /* many repairs a year might occur */
        Box_repairYears_put(&box, n, rand2(1980, 2001));
      }
    }

    /* let our boxes be 1 or 2 colored, colors numbers are 1-4 */
    {
      uint2 ncolors = rand2(1, 2);
      uint2 n, colors[10];

      Box_colors_alloc(&box, ncolors);

      for (n = 0; n < ncolors; n++)
      {
        colors[n] = rand2(1, 4);

        /*Box_colors_put(&box, n, ); */
      }

      Box_colors_put_range(&box, 0, ncolors, colors);
    }

    {
      uint2 nzipcodes = rand2(1, 10);
      uint2 n;

      rc = Box_zipcodes_alloc(&box, nzipcodes);
      if ( MCO_S_OK == rc ) {
        for (n = 0; n < nzipcodes; n++)
        {
          char zip[6];
          get_random_zip(zip);
          Box_zipcodes_put(&box, n, zip, 5);
        }
      }
    }

    rc = mco_get_last_error(t);
    if ( MCO_S_OK == rc ) {
      uint2 nitems = rand2(0, 30);
      uint2 n;
      Item item;
      char itemname[30];

      sprintf(itemname, "item_%d", next_item_id);

      Box_items_alloc(&box, nitems);

      for (n = 0; n < nitems && MCO_S_OK == rc; n++)
      {
        rc = Box_items_put(&box, n, &item);
        Item_id_put(&item, next_item_id++);
        Item_name_put(&item, itemname, (uint2)strlen(itemname));
      }
    }
  }

  return mco_get_last_error(t);
}

