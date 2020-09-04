/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "rtreedb.h"

char sample_descr[] = "Sample 'rtree' demonstrates eXtremeDB RTree index operations.\n";
const char * db_name = "rtree";

#define NRECTS        6
#define SHOW_FIRST    10

int iterate_rects(mco_trans_h t, mco_cursor_t *c, mco_bool reverse_order)
{
  MCO_RET rc;
  int i;
  rtree_class obj;
  int2 square[4];
    
  rc = (reverse_order == MCO_NO) ? mco_cursor_first(t, c) : mco_cursor_last(t, c);
  for (i = 0; MCO_S_OK == rc; i++) {
    if (i < SHOW_FIRST) {
      rc = rtree_class_from_cursor(t, c, &obj);
      if ( MCO_S_OK != rc )  { 
        sample_rc_check("\trtree_class_from_cursor()", rc );
      }
      rc = rtree_class_square_get_range(&obj, 0, 4, (int2*)square);
      if ( MCO_S_OK != rc )  { 
        sample_rc_check("\trtree_class_square_get()", rc );
      }
      printf("\t%d. (%d,%d) - (%d,%d)\n", i + 1, square[0], square[1], square[2], square[3]);
    }  
    rc = (reverse_order == MCO_NO) ? mco_cursor_next(t, c) : mco_cursor_prev(t, c);
    if ( MCO_S_OK != rc && MCO_S_CURSOR_END != rc )  { 
      sample_rc_check("\tFailed on cursor_next or cursor_prev", rc );
      sample_pause_end("\n\nPress any key to continue . . . ");
    }
  }
  if (i > SHOW_FIRST) {
    printf("\t...\n");
  }      
  
  return i;
}

int main(int argc, char* argv[])
{
  mco_db_h db = 0;
  mco_trans_h t;
  mco_cursor_t c;
  mco_runtime_info_t rt_info;
  int i;
  MCO_RET rc;
  sample_memory_t dbmem;
  rtree_class obj;
  int2 rect1[4] = { 25, 25, 50, 35 };
  int2 rect2[4] = {  5, 45, 60, 65 };
  int2 rect3[4] = { 20, 30, 85, 50 };
  int2 rect4[4] = { 10, 55, 45, 60 };
  int2 point[4] = { 10, 10, 10, 10 };
  
  sample_os_initialize(DEFAULT);

  /* Check for RTree support */
  mco_get_runtime_info(&rt_info);
  if (!rt_info.mco_rtree_supported) {
    printf("RTree is not supported by this version of runtime\n");
    sample_pause_end("\n\nPress any key to continue . . . ");
    sample_os_shutdown();
    dbg_exit( - 1);
  }

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, rtreedb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {
   
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {

        // Insert rects 1 and 2
        rtree_class_new(t, &obj);
        rtree_class_square_put_range(&obj, 0, 4, (int2*)rect1);
        rtree_class_new(t, &obj);
        rtree_class_square_put_range(&obj, 0, 4, (int2*)rect2);

        // Insert rect 3 temporarily, then delete it after exact match found
        rtree_class_new(t, &obj);
        rtree_class_square_put_range(&obj, 0, 4, (int2*)rect3);

        rc = mco_trans_commit(t);
      }

      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {

        /* Create cursor */
        rc = rtree_class_ridx_index_cursor(t, &c);
        if ( MCO_S_OK == rc ) {
          printf("\tIterate through cursor with no search condition : \n");
          i = iterate_rects(t, &c, MCO_NO);
          printf("\tFound %d total rects", i);
        }

        /* Search for the sample rect, exact match (MCO_EQ) and delete it */
        printf("\n\n\tApplying search condition for cursor:\n\t MCO_EQ to (%d,%d) - (%d,%d)\n", 
               rect3[0], rect3[1], rect3[2], rect3[3]);
        rc = rtree_class_ridx_search(t, MCO_EQ, &c, (int2*)rect3);
        if ( MCO_S_OK == rc ) {
          i = iterate_rects(t, &c, MCO_NO);
          printf("\tFound %d sample rect(s)\n", i);
          if ( i > 0 ) {
            rtree_class obj;
            printf("\n\tDelete the found rect...");
            rc = mco_cursor_first(t, &c);
            if ( MCO_S_OK == rc ) {
              rtree_class_from_cursor(t, &c, &obj);
              rtree_class_delete(&obj);
              printf("\n\tSample rect deleted\n");
            } else {
              sample_rc_check("\tUnable to find first rect", rc );
            }
          }
        } else {
          sample_rc_check("\tUnable to find any matching rect", rc );
        }

        /* Look up overlapped rects and iterate them, condition MCO_OVERLAP */
        printf("\n\tApplying search condition for cursor:\n\t MCO_OVERLAP with (%d,%d) - (%d,%d)\n", 
               rect3[0], rect3[1], rect3[2], rect3[3]);
        rc = rtree_class_ridx_search(t, MCO_OVERLAP, &c, (int2*)rect3);
        if ( MCO_S_OK == rc ) {
          i = iterate_rects(t, &c, MCO_NO);
          printf("\tFound %d rects", i);
        } else {
          printf("\n\tCouldn't find any suitable rect, code = %d, line = %d\n", rc, __LINE__);
        }

        /* Look up rects containing a specified rect, condition MCO_CONTAIN */
        printf("\n\n\tApplying search condition for cursor:\n\t MCO_CONTAIN (%d,%d) - (%d,%d)\n", 
               rect4[0], rect4[1], rect4[2], rect4[3]);
        rc = rtree_class_ridx_search(t, MCO_CONTAIN, &c, (int2*)rect4);
        if ( MCO_S_OK == rc ) {
          i = iterate_rects(t, &c, MCO_NO);
          printf("\tFound %d rects\n", i);
        } else {
          sample_rc_check("\tUnable to find any matching rect", rc );
        }

        /* Order all rects by distance from a specified point (rect with 0 height and width), condition MCO_NEIGHBOURHOOD */
        printf("\n\n\tApplying search condition for cursor:\n\t MCO_NEIGHBOURHOOD (%d,%d) - (%d,%d)\n", 
               point[0], point[1], point[2], point[3]);
        rc = rtree_class_ridx_search(t, MCO_NEIGHBOURHOOD, &c, (int2*)point);
        if ( MCO_S_OK == rc ) {
          i = iterate_rects(t, &c, MCO_NO);
          printf("\tFound %d rects\n", i);
          printf("\tIn reverse order :\n");
          i = iterate_rects(t, &c, MCO_YES);
        } else {
          sample_rc_check("\tUnable to find any matching rect", rc );
        }

        printf("\n\tDelete all rects\n");
        rc = rtree_class_delete_all(t);
        if ( MCO_S_OK != rc ) 
          sample_rc_check("\trtree_class_delete_all", rc );
        rc = mco_trans_commit(t);
        if ( MCO_S_OK != rc ) 
          sample_rc_check("\tUnable to commit delete_all transaction", rc );
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
