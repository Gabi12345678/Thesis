/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "searchdb.h"

char sample_descr[] = 
"Sample 'rtree' demonstrates eXtremeSQL spatial searches with an rtree index.\n";
const char * db_name = "searchdb";

using namespace McoSql;

#define N_OBJECTS 10

// Define the structure correponding to database table Object
struct _Object
{
    Array* box;
    int4 value;
};

// Forward declarations
int addObject( McoSqlEngine& engine, _Object* o );
int spatialSearch( McoSqlEngine& engine, char const* sql, Array* box );

int main( int argc, char ** argv )
{
  McoSqlEngine engine;

  sample_os_initialize(DEFAULT);
  sample_header(sample_descr);

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, searchdb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);
  engine.trace(true); // print query execution plan

  Allocator* allocator = engine.getAllocator(); // All of the objects created with this allocator will be destroyed at the end of this session
                                                // or use DELETE_OBJ to destroy objects manually

  Array* box = Array::create(allocator, tpInt4, 0, 4);
  Array* point = Array::create(allocator, tpInt4, 0, 2);

  // Fill database
  for ( int i=0; i < N_OBJECTS; i++ ) {
    _Object o;
    box->setAt(0, IntValue::create(allocator, i));
    box->setAt(1, IntValue::create(allocator, i));
    box->setAt(2, IntValue::create(allocator, i+1));
    box->setAt(3, IntValue::create(allocator, i+1));
    o.box = box;
    o.value = i;
    addObject( engine, &o );
  }

  box->setAt(0, IntValue::create(allocator, 4));
  box->setAt(1, IntValue::create(allocator, 4));
  box->setAt(2, IntValue::create(allocator, 5));
  box->setAt(3, IntValue::create(allocator, 5));

  point->setAt(0, IntValue::create(allocator, 3));
  point->setAt(1, IntValue::create(allocator, 3));

  // Show objects in specified rectangle
  spatialSearch(engine, "select * from AnObject where %v contains box", box);

  // Show objects containing specified rectangle
  spatialSearch(engine, "select * from AnObject where box contains %v", box);

  // Show objects overlapping specified rectangle
  spatialSearch(engine, "select * from AnObject where box overlaps %v", box);

  // Objects with specified rectangle
  spatialSearch(engine, "select * from AnObject where box = %v", box );
  
 // Objects closest to the specified point (in order of increasing distance)
  spatialSearch(engine, "select * from AnObject where box near %v", point );

  engine.close();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

int addObject( McoSqlEngine& engine, _Object* o )
{
  int ret = engine.executeStatement("insert into AnObject (box,value) values (%v,%i)", o->box, o->value);
  if ( 0 == ret )
  {
    printf("\t\tError inserting AnObject %d\n", o->value );
  }
  return ret;
}

int spatialSearch( McoSqlEngine& engine, char const* sql, Array* box )
{
  printf( "\n\t%s\n", sql);
  QueryResult result(engine.executeQuery(sql, box));
  int count = 0;
  Cursor* cursor = result->records();
  while ( cursor->hasNext() )
  {
    Record* rec = cursor->next();
    ValueRef ref(rec->get(0));
    Array* r = ref.as<Array>();
    printf("\t\tObject: box={(%d,%d)-(%d,%d)}, value=%u\n", 
           (int)r->getAt(0)->intValue(), 
           (int)r->getAt(1)->intValue(), 
           (int)r->getAt(2)->intValue(), 
           (int)r->getAt(3)->intValue(), 
           (int)rec->get(1)->intValue());
    count++;
  }
  return count;
}
