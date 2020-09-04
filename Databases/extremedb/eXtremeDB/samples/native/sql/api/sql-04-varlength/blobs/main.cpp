/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "blobsdb.h"

char sample_descr[] = 
"Sample 'blobs' demonstrates the use of the eXtremeDb 'blob' (Binary Large Object) data type.\n";
const char * db_name = "blobsdb";

using namespace McoSql;

#ifdef _VXWORKS
  char * filename = "/main.cpp";
#else
#ifdef WIN32
  char * filename = "main.cpp";
#else
  char * filename = "./main.cpp";
#endif
#endif

// Define the structure correponding to database table File
struct _AFile
{
  int id;
  char* name;
  McoSql::Value* blob;
};

// Forward declarations
int insertFile( McoSqlEngine& engine, int id, char* name );
void viewBlob( McoSqlEngine& engine, char* name );
void fetchFile( McoSqlEngine& engine, char* name );

int main( int argc, char ** argv )
{
  int ret;
  McoSqlEngine engine;

  sample_os_initialize(DEFAULT|DISK);
  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, blobsdb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
    
  printf( "\n\tPress any key to insert %s as a blob and view its contents...\n", filename );
  sample_pause("");
 
  ret = insertFile(engine, 1, filename);
  if ( ret == 0 ) {
    fetchFile( engine, filename );
    viewBlob( engine, filename );
  }

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ret;
}

int insertFile(McoSqlEngine& engine, int id, char* name)
{
  Allocator *allocator = engine.getAllocator();
  _AFile file;
  file.name = name;
  file.id = id;
  FILE* f = fopen(name, "rb");
  if ( 0 == f ) {
    printf( "\n\tFile %s not found\n", name );
    return -1;
  } 
  // Read file contents into a ScalarArray
  fseek(f, 0, SEEK_END);
  long content_length = ftell(f);
  char* content = new char[content_length];
  fseek(f, 0, SEEK_SET);
  fread(content, 1, content_length, f);
  fclose(f);
  Array* blob = Array::create(allocator, tpInt1, 0, content_length);
  blob->setBody(content, 0, content_length);
  file.blob = blob;

  // Insert record using traditional SQL syntax
  engine.executeStatement("insert into AFile values (%i,%s,%v)", id, name, blob );
  delete [] content;
  // Destroy the blob object or else it will be released only at the end of this session
  // as session allocator was used
  DELETE_OBJ(allocator, blob);

  return 0;
}

void viewBlob( McoSqlEngine& engine, char* name )
{
  _AFile file;
  QueryResult result( engine.executeQuery( "select * from AFile where name=%s", name ) );
  Cursor* cursor = result->records();
  Record* rec = cursor->next();
  result->extract(rec, &file, sizeof(_AFile));

  Blob* blob = (Blob*)file.blob;
  char buf[4096];
  int len = blob->get( buf, sizeof( buf ) );
  if ( 0 < len )
  {
    if ( len < (int)sizeof( buf ) )
      memset( &buf[len], 0, sizeof( buf ) - len );
    printf( "\n%s", buf );
  }
}

void fetchFile( McoSqlEngine& engine, char* name )
{
  _AFile file;
  QueryResult result(engine.executeQuery("select * from AFile where name=%s", name ) );
  Cursor* cursor = result->records();
  Record* rec = cursor->next();
  result->extract(rec, &file, sizeof(_AFile));

  FILE* f = fopen(name, "rb");
  if ( 0 == f ) {
    printf( "\n\tFile %s not found\n", name );
  } else {
    printf( "\n\tFile %s has id=%d\n", name, file.id );
  }
  fclose(f);
}

