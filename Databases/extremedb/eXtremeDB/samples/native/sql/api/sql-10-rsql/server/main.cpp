/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcosql.h>
#include <commonSQL.h>
#include "sqlsrv.h"
#include "rsqldb.h"

char sample_descr[] = 
"Sample 'rsql_server' demonstrates the server-side implementation in a\n"
"client-server application\n";

const char * db_name = "rsqldb";
const int SERVER_PORT = 5001;

using namespace McoSql;

volatile bool run = true;
const char* sessionStateStr[4] = { "wait", "active", "done", "canceled" };

GET_DICTIONARY(rsqldb);

static void session_info_handler( const SqlServer::session_info_t *info, void *context )
{
    char ip[MCO_NET_INETADDR_STRING_LEN];
    mco_net_ntoa(&info->peer_addr, ip);
    printf("%s'%s' peer ADDRESS %s PORT %d", (char *)context,
                        sessionStateStr[info->state],
                        ip, info->peer_port);
}

static void session_event_handler( const SqlServer::session_info_t *info, void *context )
{
    SqlServer* srv = (SqlServer *)context;
    char ip[MCO_NET_INETADDR_STRING_LEN];
    mco_net_ntoa(&info->peer_addr, ip);
    printf("Event: '%s' peer ADDRESS %s PORT %d\n",
                        sessionStateStr[info->state],
                        ip, info->peer_port);

    printf("Sessions list");
    srv->getSessionsInfo(session_info_handler, (void *)": ");
    printf("\n\n");
}

int main( int argc, char ** argv )
{
  McoMultithreadedSqlEngine engine;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  printf("\n\tInitializing the eXtremeSQL server engine...");

  #ifdef EXTREMEDB_LICENSE_KEY
  engine.license(EXTREMEDB_LICENSE_KEY);
  #endif
  engine.open( db_name, rsqldb_get_dictionary(), DATABASE_SIZE, MEMORY_PAGE_SIZE,  MAP_ADDRESS); 

  // Insert a record into the database
  engine.executeStatement("INSERT INTO aRecord (id, value) VALUES ('#123', 'server-side value')");

  SqlServer server(&engine, SERVER_PORT);
  server.regSessionEvent(session_event_handler, (void *)&server);
  server.start();

  sample_pause("\n\tServer started: press ENTER to terminate.\n\n");

  server.stop();
  engine.close();

  sample_os_shutdown();
  return 0;
}
