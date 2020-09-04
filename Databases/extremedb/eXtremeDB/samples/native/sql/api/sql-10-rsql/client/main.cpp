/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "sqlcln.h"
#include "rsqldb.h"

char sample_descr[] = 
"Sample 'rsql_client' demonstrates the client-side implementation in a\n"
"client-server application.\n";

const char * db_name = "rsqldb";
const int SERVER_PORT = 5001;
char const* SERVER_HOST = "localhost";

using namespace McoSql;

// Forward declaration
void showValue( RemoteSqlEngine *engine, char * head, char * key );

int main( int argc, char ** argv )
{
  RemoteSqlEngine engine;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  engine.open( SERVER_HOST, SERVER_PORT );

  char* key = "#123";
  showValue( &engine, "Initial values for record with ", key );
  int rc = engine.executeStatement( "UPDATE aRecord set value='Client-side value' where id=%s", key );
  if ( SQL_OK == rc || NO_MORE_ELEMENTS == rc ) {
    showValue( &engine, "After update, ", key );
  }

  if ( argc == 1 ) sample_pause("\n\nPress any key to continue . . . ");

  engine.close();

  if ( argc == 1 ) sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

void showValue( RemoteSqlEngine *engine, char * head, char * key )
{
  QueryResult result(engine->executeQuery( "SELECT value FROM aRecord where id=%s", key ) );
  Cursor* cursor = result->records();
  Record* rec = cursor->next();
  char *v;

  result->extract(rec, &v, sizeof(v));
  printf("\n\t%s key=%s: value='%s'\n", head, key, v );
}
