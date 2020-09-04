/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "structuresdb.h"

char sample_descr[] = 
"Sample 'structures' opens and initializes a database with four tables\n"
"and displays the initial contents.  Then an array of Points are added\n"
"to each record and the results displayed.  The arrays are then scanned\n"
"and searched by key.\n";
char explanation[] = 
"-----------------------------------------------------------------------\n"
"The number of array elements can be varied with command line arguments\n"
"as follows:\n"
"\n"
"   argument 1: nRecords (1-%d): number of Record objects\n"
"   argument 2: nBytes (1-%d): elements in the Byte Array field of Record\n"
"   argument 3: nPolygons (1-%d): number of Polygons in each aRecord object\n"
"   argument 4: nPoints (1-%d): number of Points per Polygon\n"
"   argument 5: nLines (1-%d): number of Lines in each Record object\n"
"-----------------------------------------------------------------------\n";

const char * db_name = "structuresdb";
int nRecords = 3;
int nBytes = 3;
int nPolygons = 4;
int nPoints = 4;
int nLines = 2;
const double EPSILON = 0.0001;

// Define max array sizes and buffer for byte array
const int MAX_RECORDS = 100;
const int MAX_BYTES = 10;
const int MAX_POLYGONS = 10;
const int MAX_POINTS = 10;
const int MAX_LINES = 10;
char byteArray[ MAX_BYTES ];

using namespace McoSql;

// Define the structure correponding to database tables
struct _Point {
  int x;
  int y;
};
struct _Line {
  _Point begin;
  _Point end;
};
struct _Polygon {
  Array* points;
};
struct _Record {
  char* s;
  short i2;
  _Point p;
  char i1;
  int64_t i8;
  float f;
  double d;
  _Line l;
  Array* ai1;
  Array* vp;
  Array* lines;
  int i4;
};

// timer variables
time_t start_time;
time_t delta;

/* forward declarations */
void insertRecords( McoSqlEngine& engine );
void showRecords( McoSqlEngine& engine );
void addArrays( McoSqlEngine& engine );
void showRecordArrays( McoSqlEngine& engine );
void showStruct( McoSqlEngine& engine, int x );

int main( int argc, char* argv[] )
{
  McoSqlEngine engine;

  sample_os_initialize(DEFAULT);

  sample_header( sample_descr );

  // Show how to vary command line args and max values
  printf( explanation, MAX_RECORDS, MAX_BYTES, MAX_POLYGONS, MAX_POINTS, MAX_LINES );

  // Get command line args if specified
  if ( argc > 1 ) 
    nRecords = atoi( argv[1] );
  if ( argc > 2 )
    nBytes = atoi( argv[2] );
  if ( argc > 3 )
    nPolygons = atoi( argv[3] );
  if ( argc > 4 )
    nPoints = atoi( argv[4] );
  if ( argc > 5 )
    nLines = atoi( argv[5] );

  // Assure that values are within limits
  if ( nRecords > MAX_RECORDS ) nRecords = MAX_RECORDS;
  if ( nBytes > MAX_BYTES ) nBytes = MAX_BYTES;
  if ( nPolygons > MAX_POLYGONS ) nPolygons = MAX_POLYGONS;
  if ( nPoints > MAX_POINTS ) nPoints = MAX_POINTS;
  if ( nLines > MAX_LINES ) nLines = MAX_LINES;

  printf( "\n\tRunning with nRecords=%d, nBytes=%d, nPolygons=%d, nPoints=%d, nLines=%d.\n",
          nRecords, nBytes, nPolygons, nPoints, nLines );

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, structuresdb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE);

  // Insert records
  insertRecords( engine );

  // Show contents (without arrays)
  showRecords( engine );

  // Add arrays of ints, Polygons and Lines to Records
  addArrays( engine );

  // Show contents with arrays
  showRecordArrays( engine );

  showStruct( engine, 3 );

  engine.close();
  
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

void insertRecords( McoSqlEngine& engine )
{
  printf("\n\tInsert %d record%s ...\n\t", nRecords, ( nRecords > 1 ? "s" : "" ) );
  start_time = time(NULL);
  for (int i = 1; i <= nRecords; i++)
  {
    _Record r;
    char buf[256];
    sprintf(buf, "Record %d", i);
    r.s = buf;
    r.i2 = i * 10;
    r.p.x = i + 1;
    r.p.y = i + 2;
    r.i1 = i % 10;
    r.i8 = i * 100;
    r.f = (float)i / 10;
    r.d = (double)i / 100;
    r.l.begin.x = i + 2;
    r.l.begin.y = i + 2;
    r.l.end.x = i + 3;
    r.l.end.y = i + 4;
    r.i4 = i;
    r.ai1 = NULL;
    r.vp = NULL;
    r.lines = NULL;
    engine.executeStatement("insert into aRecord %r", &r);
  }

  delta = time(NULL) - start_time;
  printf("\t%d objects: %d seconds (%d object/second)\n", nRecords, (int)delta,
                                  (int)delta != 0 ? nRecords / (int)delta: nRecords);
}

void addArrays( McoSqlEngine& engine )
{
  Allocator allocator;      // Objects created with this allocator will be automatically destroyed
                            // by the allocator's destructor when this function returns

  int i,j, k;
  printf("\n\n\tUpdate records adding Polygons and Lines ...\n");
  start_time = time(NULL);
  for ( i = 1; i <= nRecords; i++ ) {

    // Get record i for updating
    QueryResult result(engine.executeQuery( "select * from aRecord where i4=%i for update", i ) );
    Cursor* cursor = result->records();
    Record* rec = cursor->next();

    // Get field values into Field variables
    Field* vp = result->findField(String::create(&allocator, "vp"));
    Field* points = vp->element()->findComponent(String::create(&allocator, "points"));
    Field* x = points->element()->findComponent(String::create(&allocator, "x"));
    Field* y = points->element()->findComponent(String::create(&allocator, "y"));
    Field* lines = result->findField(String::create(&allocator, "lines"));
    Field* beg = lines->element()->findComponent(String::create(&allocator, "begin"));
    Field* end = lines->element()->findComponent(String::create(&allocator, "end"));
    
    // Get current contents of record into struct _Record
    _Record r;
    result->extract(rec, &r, sizeof(r));
    
    // Set values of Byte array
    memset( byteArray, 0, nBytes );
    for ( j=0; j < nBytes; j++ )
      byteArray[j] = ( i * nBytes) + j;
    r.ai1->setBody( byteArray, 0, nBytes );
    
    // Set Polygons
    r.vp->setSize( nPolygons );
    for ( j = 0; j < nPolygons; j++ ) {
      Struct* poly = (Struct*)r.vp->updateAt( j );
      Array* pa = (Array*)points->update( poly );
      pa->setSize(nPoints);
      for ( k = 0; k < nPoints; k++) {
        Struct* point = (Struct*)pa->updateAt(k);
        x->set( point, new (&allocator) IntValue( (i * 100) + j + k + 1 ) );
        y->set( point, new (&allocator) IntValue( (i * 100) - j - k - 1 ) );
      }
    }

    // Set lines
    for ( j = 0; j < nLines; j++ ) {
      Struct* line = (Struct*)r.lines->updateAt(j);
      Struct* bp = (Struct*)beg->update(line);
      Struct* ep = (Struct*)end->update(line);
      x->set( bp, new (&allocator) IntValue( (i * 100) + j + 1 ) );
      y->set( bp, new (&allocator) IntValue( (i * 100) - j - 1 ) );
      x->set( ep, new (&allocator) IntValue( (i * 100) + j + 3 ) );
      y->set( ep, new (&allocator) IntValue( (i * 100) - j - 3 ) );
    }
  }

  delta = time(NULL) - start_time;
  printf("\t\t%d objects: %d seconds (%d object/second)\n", nRecords, (int)delta,
                                    (int)delta != 0 ? nRecords / (int)delta: nRecords);
}

void showRecords( McoSqlEngine& engine )
{
  printf("\n\tRecord table contents:" );
  for ( int i=1; i <= nRecords; i++ )
  {
    _Record r;
    QueryResult result( engine.executeQuery( "select * from aRecord where i4=%i", i ) );
    Cursor* cursor = result->records();
    if ( cursor->hasNext() ) {
      Record* rec = cursor->next();
      result->extract( rec, &r, sizeof(r) );
      printf( "\n\t%d) %s: i2=%d, i4=%d, x=%d, y=%d, f=%f, d=%f, \n\t\t"
              "line(x1,y1,x2,y2)=%d,%d,%d,%d",
              i, r.s, r.i2, r.i4, r.p.x, r.p.y, r.f, r.d, 
              r.l.begin.x, r.l.begin.y, r.l.end.x, r.l.end.y );
    }
  }
}
void showRecordArrays( McoSqlEngine& engine )
{
  int i,j,k;
  printf("\n\n\tRecord Array contents:" );
  for ( i=1; i <= nRecords; i++ )
  {
    // Get record i
    QueryResult result(engine.executeQuery( "select * from aRecord where i4=%i", i ) );
    Cursor* cursor = result->records();
    Record* rec = cursor->next();
    _Record r;
    result->extract(rec, &r, sizeof(r));
    printf( "\n\n\t%d) %s: ", i, r.s );

    // Display Byte Array
    printf( "\n\t\tArray of %d 'i1' values: ", nBytes );
    r.ai1->getBody( byteArray, 0, nBytes );
    for ( j=0; j < nBytes; j++) {
      printf( "%d%s", byteArray[j], ( j == nBytes-1 ? "" : ", " ) );
    }

    // Display Polygon points
    printf( "\n\t\tPoints for %d Polygons : ", nPolygons );
    for ( j=0; j < nPolygons; j++ ) {
      ValueRef polyref(r.vp->getAt(j));
      Struct* poly = polyref.as<Struct>();
      ValueRef pointsref(poly->get(0));
      Array* points = pointsref.as<Array>();
      printf( "\n\t\t\t Polygon %d:", j+1 );
      for ( k=0; k < nPoints; k++ )
      {
        ValueRef pointref(points->getAt(k));
        Struct* point = pointref.as<Struct>();
        printf( "\n\t\t\t\t%d) x=%d, y=%d ", k+1, (int)point->get(0)->intValue(), (int)point->get(1)->intValue() );
      }
    }

    // Display Line end points
    printf( "\n\t\tEnd points for %d Lines : ", nLines );
    for ( j=0; j < nLines; j++ ) {
      ValueRef lineref(r.lines->getAt(j));
      Struct* line = lineref.as<Struct>();
      ValueRef begref(line->get(0));
      ValueRef endref(line->get(1));
      Struct* beg = begref.as<Struct>();
      Struct* end = endref.as<Struct>();
      printf( "\n\t\t\t\t%d) begin=(%d,%d) end=(%d,%d)", j+1, (int)beg->get(0)->intValue(), (int)beg->get(1)->intValue(),
              (int)end->get(0)->intValue(), (int)end->get(1)->intValue() );
   }
  }
}
// Select a struct element and string field from aRecord
void showStruct( McoSqlEngine& engine, int x )
{
  printf("\n\n\tExtract p.y and s fields from aRecord with p.x=%d:", x );
  QueryResult result(engine.executeQuery( "select p.y, s from aRecord where p.x=%i", x ) );
  Cursor* cursor = result->records();
  Record* rec = cursor->next();

  // Use Struct::get(int index) method to refer to the result column by index (0-based)
  int2 y = (int2)rec->get(0)->intValue();
  printf( "\n\t\tUsing Struct::get(int index): y=%d, s='%s'", y, (char *)rec->get(1)->pointer() );

  // Use field descriptors
  printf( "\n\t\tUsing QueryResult::Fields vector:" );
  Vector <Field> * fields = result->fields();
  for ( size_t iField = 0; iField < fields->length; iField++ )
  {
    Field * f = fields->at(iField);
    ValueRef ref(f->get(rec));
    switch ( iField )
    {
      case 0:
        printf( "\n\t\t\tField %s=%d", f->name()->cstr(), (int)f->get(rec)->intValue() );
        break;

      case 1:
        printf( "\n\t\t\tField %s='%s'", f->name()->cstr(), ref.as<String>()->cstr() );
    }
  }

  // Use the DataSource.extract() method. NB: The structure used must have same number
  // of fields as specified in the select statement.
  struct {
    int y;
    char * s;
  } row;
  result->extract( rec, &row, sizeof( row ) );
  printf( "\n\t\tUsing DataSource::extract(): y=%d, s='%s'\n", row.y, row.s );
}

