/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include "mcosql.h"
#include "xsqldb.h"
#include "sqlcln.h"
#include "sqlsrv.h"
#include <string.h>
#include <time.h>
#include <locale.h>
#include <commonSQL.h>

#ifdef _ECOS
    #include "unistd.h"
#endif

#ifdef _VXWORKS
#define	FILEPREFIX	"/"
#else
#define	FILEPREFIX	""
#endif

#ifdef _INTEGRITY
char * strdup( const char *src ) {
    unsigned int len = strlen(src);
    char * ret = (char*)malloc(len+1);
    memcpy( ret, src, len+1 );
    return ret;
}
#endif

using namespace McoSql;

size_t const DB_PAGE_SIZE = 256;

#if defined(_VXWORKS) && defined(_RTP)
    #define    PROMPT    "XSQL>\n"
#else
    #define    PROMPT    "XSQL>"
#endif

size_t const DATABASE_SIZE_MB = 4;
void* const DB_MAP_ADDRESS = (void*)0;//x20000000;
int const DEFAULT_PORT = 5001;

const size_t DISK_CACHE_SIZE_MB = 64;
const size_t MB = 1024*1024;
const size_t MAX_TABLES = 100;
const size_t MAX_INDEXES = 1000;
const size_t MAX_DICTIONARY_SIZE = 1024*1024;
const size_t MAX_NODES = 1024;
const size_t MAX_CONNECT_ATTEMPTS = 10;

MCO_RET sniffer_callback(mco_db_h db, void* context, mco_trans_counter_t trans_no)
{
    return MCO_S_DEAD_CONNECTION;
}

#ifdef _VXWORKS
char * strdup( const char * s ) {
	unsigned int len = strlen(s);
	char * r = (char*)malloc( len + 1 );
	return strcpy( r, s );
}
#endif

int main(int argc, char* argv[])
{
    SqlEngine* engine = NULL;
    SqlServer* server = NULL;
    int i = 2;
    bool daemon = false;
    char* nodes[MAX_NODES];
    sample_os_initialize(DEFAULT);
    if (argc < 2)
    {
        mco_runtime_info_t info;

        mco_get_runtime_info(&info);

        fprintf(stderr, "api-sql-12-interactive: eXtremeSQL %d.%d (build %d) interactive SQL Utility\n\n"
                "Usage: api-sql-12-interactive (DATABASE_NAME | @HOST)[:PORT | :*] [-path DATA_FILE_PATH] [-page MEM_PAGE_SIZE] [-connections MAX_CONNECTIONS] [-log LOG_FILE_PATH] [-size SIZE] [-cache SIZE] [-redundancy N] [-locale LOC] [-time] [-recovery] [-daemon] [-trace] [-local] [-interactive] [-login NAME] [-password PASSWD] {SQL_FILE}\n",
                info.mco_version_major, info.mco_version_minor, info.mco_build_number);
        sample_os_shutdown();
        return 1;
    }
    char* databaseName = argv[1];
    char* databaseFilePath = NULL;
    char* databaseLogPath = NULL;
    char* portPart = strchr(databaseName, ':');
    int   port = DEFAULT_PORT;
    int pageSize = DB_PAGE_SIZE;
    int databaseSize =  DATABASE_SIZE_MB;
#ifdef DISK_DATABASE
    int cacheSize = DISK_CACHE_SIZE_MB;
    MCO_COMMIT_POLICY policy = MCO_COMMIT_SYNC_FLUSH;
#endif
    int nThreads = 8;
    bool  measureTime = false;
    bool  recovery = false;
    bool interactive = false;
    bool trace = false;
    bool localDomain = false;
    int redundancy = 1;
    int maxConnections = 100;
    int bufSize = 64*1024;
    size_t nNodes = 0;
	char const* login = NULL;
	char const* password = NULL;

    if (*databaseName == '@') { 
        nNodes = 1;
        nodes[0] = databaseName + 1;
    }

    if (portPart != NULL)
    {
        *portPart = '\0';
        if (portPart[1] != '*')
        {
            if (sscanf(portPart + 1, "%d", &port) != 1)
            {
                fprintf(stderr, "Invalid port number\n");
                sample_os_shutdown();
                return 1;
            }
        }
    }
    while (i < argc && (*argv[i] == '-' || *argv[i] == '@')) {
        if (argv[i][0] == '@') {
            if (nNodes == MAX_NODES) {
                fprintf(stderr, "Too much nodes\n");
                return 1;
            }
            nodes[nNodes++] = &argv[i][1];
        } else if (strcmp(argv[i], "-recovery") == 0) {
            recovery = true;
        } else if (strcmp(argv[i], "-local") == 0) {
            localDomain = true;
        } else if (i+1 < argc && strcmp(argv[i], "-path") == 0) {
            databaseFilePath = strdup(argv[++i]);
        } else if (i+1 < argc && strcmp(argv[i], "-log") == 0) {
            databaseLogPath = strdup(argv[++i]);
        } else if (i+1 < argc && strcmp(argv[i], "-size") == 0) {
            databaseSize = atoi(argv[++i]);
        } else if (i+1 < argc && strcmp(argv[i], "-redundancy") == 0) {
            redundancy = atoi(argv[++i]);
        } else if (i+1 < argc && strcmp(argv[i], "-cache") == 0) {
#ifdef DISK_DATABASE
            cacheSize = atoi(argv[++i]);
#endif
        } else if (i+1 < argc && strcmp(argv[i], "-page") == 0) {
            pageSize = atoi(argv[++i]);
        } else if (i+1 < argc && strcmp(argv[i], "-buffer") == 0) {
            bufSize = atoi(argv[++i]);
        } else if (i+1 < argc && strcmp(argv[i], "-threads") == 0) {
            nThreads = atoi(argv[++i]);
        } else if (i+1 < argc && strcmp(argv[i], "-locale") == 0) {
            if (setlocale(LC_CTYPE, argv[++i]) == NULL) {
                fprintf(stderr, "Unsupported locale %s\n", argv[i]);
                return 1;
            }
        } else if (i+1 < argc && strcmp(argv[i], "-connections") == 0) {
            maxConnections = atoi(argv[++i]);;
        } else if (strcmp(argv[i], "-time") == 0) {
            measureTime = true;
        } else if (strcmp(argv[i], "-daemon") == 0) {
            daemon = true;
        } else if (strcmp(argv[i], "-trace") == 0) {
            trace = true;
        } else if (i+1 < argc && strcmp(argv[i], "-login") == 0) {
            login = argv[++i];
        } else if (i+1 < argc && strcmp(argv[i], "-password") == 0) {
            password = argv[++i];
        } else if (strcmp(argv[i], "-interactive") == 0) {
            interactive = true;
        } else if (i+1 < argc && strcmp(argv[i], "-policy") == 0) {
            i += 1;
#ifdef DISK_DATABASE
            if (strcmp(argv[i], "sync") == 0) {
                policy = MCO_COMMIT_SYNC_FLUSH;
            } else if (strcmp(argv[i], "buffered") == 0) {
                policy = MCO_COMMIT_BUFFERED;
            } else if (strcmp(argv[i], "delayed") == 0) {
                policy = MCO_COMMIT_DELAYED;
            } else if (strcmp(argv[i], "nosync") == 0) {
                policy = MCO_COMMIT_NO_SYNC;
            } else {
                fprintf(stderr, "Unknown policy %s\n", argv[i]);
                sample_os_shutdown();
                return 1;
            }
#endif
        } else {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            sample_os_shutdown();
            return 1;
        }
        i += 1;
    }
    if (nNodes > 1) {
        // distributed SQL
        if (nNodes == MAX_NODES) {
            fprintf(stderr, "Too much nodes\n");
            return 1;
        }
        if (portPart != NULL) { 
            *portPart = ':';
        }
        DistributedSqlEngine* client = new DistributedSqlEngine();
        int badNode = -1;
        if (!client->open(nodes, nNodes, redundancy, DistributedSqlEngine::SQL_REPLICATION, MAX_CONNECT_ATTEMPTS, &badNode, localDomain)) {
            fprintf(stderr, "Failed to connect to node %d\n", badNode);
            return 1;
        }
		if (login != NULL) {
			if (!client->authenticate(login, password)) { 
				fprintf(stderr, "Authentication failure\n");
				return 1;
			}
		}
		engine = client;
    } else if (nNodes != 0) {
        // remote client
        portPart = strrchr(nodes[0], ':');
        if (portPart != NULL) {
            *portPart = '\0';
            if (portPart[1] != '*')
            {
                if (sscanf(portPart + 1, "%d", &port) != 1)
                {
                    fprintf(stderr, "Invalid port number\n");
                    sample_os_shutdown();
                    return 1;
                }
            }
        }
        RemoteSqlEngine* client = new RemoteSqlEngine();
        if (!client->open(nodes[0], port, MAX_CONNECT_ATTEMPTS, localDomain)) {
            fprintf(stderr, "Failed to connect to server\n");
            return 1;
        }
		if (login != NULL) {
			if (!client->authenticate(login, password)) { 
				fprintf(stderr, "Authentication failure\n");
				return 1;
			}
		}
        engine = client;
    }
    if (*databaseName != '@')
    {
        McoSqlOpenParameters params;
        McoMultithreadedSqlEngine* sqlEngine = new McoMultithreadedSqlEngine();
        #ifdef EXTREMEDB_LICENSE_KEY
        sqlEngine->license(EXTREMEDB_LICENSE_KEY);
        #endif

        #ifdef DISK_DATABASE
        if (databaseFilePath == NULL) {
            databaseFilePath = new char[strlen(databaseName) + 8];
            sprintf(databaseFilePath, "%s%s.dbs", FILEPREFIX, databaseName);
        }
        if (databaseLogPath == NULL) {
            databaseLogPath = new char[strlen(databaseName) + 8];
            sprintf(databaseLogPath, "%s%s.log", FILEPREFIX, databaseName);
        }
        params.diskDatabaseFile = databaseFilePath;
        params.diskDatabaseLogFile = databaseLogPath;
        params.diskCacheSize = cacheSize*MB;
        params.defaultCommitPolicy = policy;
        #endif

        params.databaseName = databaseName;
        params.dictionary = xsqldb_get_dictionary();
        params.mainMemoryDatabaseSize = databaseSize*MB;
        params.mainMemoryPageSize = pageSize;
        params.mainMemoryDatabaseAddress = DB_MAP_ADDRESS;
        params.diskPageSize = pageSize*8;
        params.maxConnections = maxConnections;
        params.maxClasses = MAX_TABLES;
        params.maxIndexes = MAX_INDEXES;
        params.maxDictionarySize = MAX_DICTIONARY_SIZE;
        params.remoteEngine = engine;

        sqlEngine->open(params);
        if (recovery) {
            MCO_RET rc = mco_db_sniffer(sqlEngine->getHandle(), sniffer_callback, MCO_SNIFFER_INSPECT_ACTIVE_CONNECTIONS);
            if (rc != MCO_S_OK) {
                fprintf(stderr, "Database recovery error: %d\n", rc);
            }
        }

        if (portPart != NULL)
        {
            server = new SqlServer(sqlEngine, port, bufSize, nThreads, 5, NULL, localDomain, NULL, login != NULL);
            server->start();
        }
        engine = sqlEngine;
    } 
    #undef main
    engine->trace(trace);
    bool batch = false;
    if (i < argc)
    {
        time_t start_time = time(NULL);
        while (i < argc)
        {
            FILE* f = fopen(argv[i], "r");
            if (f == NULL)
            {
                fprintf(stderr, "Failed to open file '%s'\n", argv[i]);
            }
            else
            {
                engine->main(NULL, f, stdout, stderr);
            }
            i += 1;
        }
        if (measureTime) {
            printf("Elapsed time: %d seconds\n", (int)(time(NULL) - start_time));
        }
        batch = !interactive;
    }
    if (server != NULL || !batch) {
        if (daemon)
        {
            #if defined(_WIN32) || defined(_INTEGRITY) || defined(_VXWORKS)
            sample_pause("\nPress any key to close database");
            #else
            pause();
            #endif
        }
        else
        {
            engine->main(PROMPT, stdin, stdout, stderr);
        }
    }
    if (server != NULL)
    {
        server->stop();
    }
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
    try
    {
        engine->close();
    }
    catch (McoSqlException const &x)
    {
        fprintf(stderr, "ERROR: %s\n", x.getMessage()->cstr());
    }
#else
    engine->close();
#endif
    delete server;
    delete engine;
    delete[] databaseFilePath;
    delete[] databaseLogPath;
    //printf("\nProgram complete\n");

    //printf("\n\nPress any key to continue . . . ");
    //getchar();

    sample_os_shutdown();
    return 0;
}

