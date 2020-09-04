/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "vectorsdb.h"

char sample_descr[] = 
"Sample 'vectors' demonstrates the use of the eXtremeDb Vector data type.\n";
const char * db_name = "vectorsdb";

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
  Array* lines;
};

// Forward declarations
int insertFile( McoSqlEngine& engine, int id, char* name );
void viewLines( McoSqlEngine& engine, char* name );

int main( int argc, char ** argv )
{
  int ret;
  McoSqlEngine engine;

  sample_os_initialize(DEFAULT|DISK);
  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, vectorsdb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
    
  printf( "\n\tA vector of integers values representing the incremental sum \n"
          "\tof the lengths of the lines of file '%s' will be \n"
          "\tinserted into the database, then extracted from a\n"
          "\tquery result and displayed...", filename );

  sample_pause("\n\nPress any key to continue . . . ");
 
  ret = insertFile(engine, 1, filename);
  if ( ret == 0 ) {
    viewLines( engine, filename );
  }

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  return ret;
}

int insertFile(McoSqlEngine& engine, int id, char* name)
{
  FILE* f = fopen(name, "rb");
  if ( 0 == f ) {
    printf( "\n\tFile %s not found.\n" \
            "\tPlease set directory of sample's sources as current directory",
            name );
    return -1;
  } 
  // Read file contents into a ScalarArray
  fseek(f, 0, SEEK_END);
  long content_length = ftell(f);
  char* content = new char[content_length];
  fseek(f, 0, SEEK_SET);
  fread(content, 1, content_length, f);
  fclose(f);

  // Count newlines
  size_t i, lines_num = 1;
  for (i = 0; i < content_length; i++)
  {
    if (content[i] == '\n')
    {
      lines_num++;
    }
  }

  // Build vector of file line offsets
  Allocator allocator;      // Objects created with this allocator will be automatically destroyed
                            // in allocator's destructor in the end of this function

  Array* lines = Array::create(&allocator, tpInt4, 0, lines_num);
  size_t j = 1;
  lines->setAt(0, IntValue::create(&allocator, 0));
  for (i = 0; i < content_length; i++)
  {
    if (content[i] == '\n')
    {
      lines->setAt(j, IntValue::create(&allocator, i));
      j++;
    }
  }

  // insert record using traditional SQL syntax
  engine.executeStatement("insert into AFile values (%i,%s,%v)", id, name, lines );
  delete [] content;

  return 0;
}

void viewLines( McoSqlEngine& engine, char* name )
{
  _AFile file;
  QueryResult result( engine.executeQuery( "select * from AFile where name=%s", name ) );
  Cursor* cursor = result->records();
  Record* rec = cursor->next();
  result->extract(rec, &file, sizeof(_AFile));

  int *buf = new int[file.lines->size()];
  file.lines->getBody(buf, 0, file.lines->size());

  for ( int i=0; i < file.lines->size(); i++ ) {    
    printf( "\n%d", buf[i] );
  }

  delete [] buf;
}
