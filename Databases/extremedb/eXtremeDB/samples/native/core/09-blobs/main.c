/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include "blobdb.h"

char sample_descr[] = "Sample 'blob' demonstrates basic BLOB operations.\n";
const char * db_name = "blob";
const int max_blob_size = 4*1024;
const char * end_tag = "endofblob";
const int tag_size = 9; /* NB: this must be equal to 'strlen(end_tag)' */

const int n_blobs = 100;

MCO_RET update_blob(mco_db_h db, int objid)
{
  MCO_RET rc;
  int k = 0;
  blobdb_oid id;
  BlobClass blob;
  mco_trans_h t = 0;
  char* buf = 0;
  uint4 blob_size;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    id.seq = objid;
    rc = BlobClass_oid_find(t, &id, &blob);
    if ( MCO_S_OK == rc ) {
      rc = BlobClass_blo_size(&blob, &blob_size);
      if ( MCO_S_OK == rc ) {
        buf = (char*)malloc(blob_size);
        if ( 0 == buf ) {
          printf("\n\tUnable to allocate %d bytes", blob_size);
        } else {
          /* Get blob field, modify update buffer and put to blob field, free buffer */
          BlobClass_blo_get(&blob, 0, buf, blob_size, 0);
          for (k = 0; k < (int)blob_size; k++)
          {
            buf[k]++;
          }
          BlobClass_blo_put(&blob, buf, blob_size);
          free(buf);
        }
      }
    }

    rc = mco_trans_commit(t);
  }

  return rc;
}

MCO_RET read_blob(mco_db_h db, int objid, uint4* lgb, int blob_size)
{
  MCO_RET rc;
  int k;
  blobdb_oid id;
  BlobClass bo;
  mco_trans_h t = 0;
  char* buf = 0;
  int block_size = 200;
  uint4 start = 0, retlen;

  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    id.seq = objid;

    rc = BlobClass_oid_find(t, &id, &bo);
    if ( MCO_S_NOTFOUND == rc ) {
      printf("\nError locating object (%d) - object not found\n", rc);
    }

    if ( MCO_S_OK == rc ) {
      rc = BlobClass_blo_size(&bo, lgb);
      if ( MCO_S_OK == rc ) {
        buf = (char*)malloc(blob_size);
        if ( 0 == buf ) {
          printf("\n\tUnable to allocate %d bytes", blob_size);
        } else {
          /* Read blob field into buffer in block_size bites */
          for (k = 0, retlen = 1; 0 != retlen && MCO_S_OK == rc; k++)
          {
            retlen = 0;
            rc = BlobClass_blo_get(&bo, start, buf, block_size, &retlen);
            if ( MCO_S_OK != rc ) {
              printf("\nError reading BLOB (%d)", rc);
            } else {
              start += retlen;
            }
          }
        }
      }
    }
     
    mco_trans_commit(t);
  }

  /* Free the input buffer if allocated */
  if ( 0 != buf )
  {
    free( buf );
  }
  return rc;
}

MCO_RET write_blob(mco_db_h db, int objid, const char* text, int lgtext)
{
  mco_trans_h t = 0;
  MCO_RET rc;
  blobdb_oid id;
  BlobClass bo;

  id.seq = objid;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    rc = BlobClass_new(t, &id, &bo);
    rc = BlobClass_blo_put(&bo, text, lgtext - tag_size);
    if ( MCO_S_OK != rc ) {
      if ( MCO_E_NOMEM == rc )
      {
        printf("\nError writing BLOB (%d) - out of memory", rc);
      }
      else
      {
        printf("\nError writing BLOB: %d", rc);
      }
    } else {
      /* Append the blob */
      rc = BlobClass_blo_append(&bo, end_tag, tag_size);
      if ( MCO_E_NOMEM == rc )
      {
        printf("\nCouldn't append to BLOB (%d) - out of memory", rc);
      } 
    }

    if ( MCO_S_OK == rc ) {
      rc = mco_trans_commit(t);
    } else {
      mco_trans_rollback(t);
    }
  }

 return rc;
}

void show_mem(mco_db_h db)
{
  uint4 totalpg, freepg;

  mco_db_free_pages(db, &freepg);
  mco_db_total_pages(db, &totalpg);

  printf("\n\tMemory Report:\n\ttotal pages=%d (%dK)\n\tfree pages=%d (%dK)\n\tused %dK\n\n", totalpg, totalpg *
         MEMORY_PAGE_SIZE / 1024, freepg, freepg * MEMORY_PAGE_SIZE / 1024, (totalpg - freepg)* MEMORY_PAGE_SIZE / 1024);
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
  int i, j;
  time_t t1, t2;
  uint4 tsize = 0, lgtxt;
  char fill_letter;
  char* txt;
    
  sample_os_initialize(DEFAULT);

  txt = (char*)malloc(max_blob_size);
  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open databases */
  rc = sample_open_database( db_name, blobdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      show_mem(db);
      printf("\n\tCreating %d blobs ...\n\t", n_blobs);

      /* Start the timer, write NumBlobs into the database */
      t1 = sample_msec();
      for (i = 0; i < n_blobs && MCO_S_OK == rc; i++) {

        /* Set this blob size to count plus 40 bytes */
        lgtxt = i + 40;
        /* Make second half of the blob list bigger by 2K */
        if (i > n_blobs / 2) {
	    lgtxt += 2*1024;
        }
        /* Make sure that we don't exceed the text buffer size */
        if ( lgtxt > max_blob_size - tag_size )
          lgtxt = max_blob_size - tag_size;
        /* Fill this blob text with the letter 'A', 'B', 'C', 'D' or 'E' */
        fill_letter = (i % 5);
        for (j = 0; j < (int)lgtxt; j++) {
          txt[j] = 'A' + fill_letter;
        }

        rc = write_blob(db, i, txt, lgtxt);
        if ( MCO_S_OK == rc ) {
          /* Increment the total size of BLOBs written including the length of the tag 
           * appended to blob by write_blob, and print output for every twenty written */
          tsize += lgtxt + tag_size; 
          if ( 0 == i % 20 ) {
            printf(".");
          }
        }
      }

      printf("\n\n\t%d blobs created\n", i);
      show_mem(db);
      t2 = sample_msec();

      if (tsize > 1024) {
        printf("\n\n\tTotal %ld K written: %d sec", (unsigned long)(tsize / 1024), (int)(t2 - t1));
      }
      else
      {
        printf("\n\n\tTotal %d bytes written: %d sec", tsize, (int)(t2 - t1));
      }

      lgtxt = tsize = 0;

      /* Read the data back */
      t1 = sample_msec();
      for (i = 0; i < n_blobs && MCO_S_OK == rc; i++) {
        rc = read_blob(db, i, &lgtxt, max_blob_size);
        if ( MCO_S_OK == rc ) {
          tsize += lgtxt;
        }
      }
 
      t2 = sample_msec();
      printf("\n\tTotal %ld K read: %d sec", (unsigned long)(tsize / 1024), (int)(t2 - t1));

      /* Update the blobs  */
      t1 = sample_msec();
      tsize = 0;
      for (i = 0; i < n_blobs && MCO_S_OK == rc; i++)
      {
        rc = update_blob(db, i);
      }

      t2 = sample_msec();
      printf("\n\n\t%d blobs updated: %d sec", i, (int)(t2 - t1));

      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  free(txt);

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}
