/*******************************************************************
 *                                                                 *
 *  tpc_log.cpp                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
/*
 *  This is a sample implementation of the Transaction Processing Performance
 *  Council Benchmark B coded in Java and ANSI SQL2. This sample additionnaly
 *  run Transaction Logging.
 */

#include <common.h>
#include "mcosmp.h"
#include "mcosql.h"
#include "tpcdb.h"
#include "mcotime.h"
#include "mcolog.h"

/* redefine default stack size if needed */
#undef DEFAULT_STACK_SIZE
#define DEFAULT_STACK_SIZE 64*1024

using namespace McoSql;

/* tpc bm b scaling rules */
const int tps = 1; /* the tps scaling factor: here it is 1 */
const int nbranches = 1; /* number of branches in 1 tps db       */
const int ntellers = 10; /* number of tellers in  1 tps db       */

const size_t DB_PAGE_SIZE = 256;

void *const MAP_ADDRESS = DATABASE_MAP_ADDRESS;

McoMultithreadedSqlEngine *engine;

#ifdef DISK_DATABASE

//   #ifdef _VXWORKS
//     #define     FILE_PREFIX     "/"
//   #else
//     #define     FILE_PREFIX
//   #endif

  #ifdef RESTRICTED_TEST_ENVIRONMENT
    const int naccounts = 1000; /* number of accounts in 1 tps db       */
    size_t const MEMORY_DATABASE_SIZE = 8 * 1024 * 1024;
    size_t const DISK_CACHE_SIZE = 6 * 1024 * 1024;
    volatile int numclients = 5;
    volatile int n_clients = 5;
    int n_txn_per_client = 1000;
  #else
    const int naccounts = 2000; /* number of accounts in 1 tps db       */
    size_t const MEMORY_DATABASE_SIZE = 16 * 1024 * 1024;
    size_t const DISK_CACHE_SIZE = 16 * 1024 * 1024;
    volatile int numclients = 5;
    volatile int n_clients = 5;
    int n_txn_per_client = 5000;
  #endif

#else

  #ifdef RESTRICTED_TEST_ENVIRONMENT
    const int naccounts = 1000; /* number of accounts in 1 tps db       */
    size_t const MEMORY_DATABASE_SIZE = 16 * 1024 * 1024;
    volatile int numclients = 5;
    volatile int n_clients = 5;
    int n_txn_per_client = 1000;
  #else
    const int naccounts = 10000; /* number of accounts in 1 tps db       */
    size_t const MEMORY_DATABASE_SIZE = 64 * 1024 * 1024;
    volatile int numclients = 5;
    volatile int n_clients = 5;
    int n_txn_per_client = 10000;
  #endif

#endif

volatile int numreps = 1;

int transaction_count = 0;
time_t start_time = 0;
size_t min_memory = MEMORY_DATABASE_SIZE;
size_t max_memory = 0;
int transaction_conflicts = 0;

mco_mutex_t *cs;

void errhandler(int errcode)
{
    printf("error code %d *\n", errcode);
    puts("Press any key to exit");
    getchar();
    dbg_exit( - 1);
}

int parse_cmd(int argc, char *argv[])
{
    int flag = 1;
    char *p;

    for (int i = 1; i < argc;)
    {
        p = argv[i++];
        if (*p++ == (char)'-')
        {
            if (strcmp(p, "clients") == 0)
            {
                if ((i < argc) && (argv[i][0] != '-'))
                {
                    p = argv[i];
                    i++;
                    numclients = atoi(p);
                }
                flag = 1;
            }
            else if (strcmp(p, "tpc") == 0)
            {
                if ((i < argc) && (argv[i][0] != '-'))
                {
                    p = argv[i];
                    i++;
                    n_txn_per_client = atoi(p);
                }
                flag = 1;
            }
            else if (strcmp(p, "repeats") == 0)
            {
                if ((i < argc) && (argv[i][0] != '-'))
                {
                    p = argv[i];
                    i++;
                    numreps = atoi(p);
                }
                flag = 1;
            } else { 
                flag = 0;
            }
        }

    }
    return flag;
}

#if 1

    void reportDone()
    {
        cs->lock(cs);
        n_clients--;
        if (n_clients <= 0)
        {
            time_t end_time = sample_msec();
            time_t completion_time = end_time - start_time;
            printf("* Benchmark finished *\n");
            printf("\n* Benchmark Report *");
            printf("--------------------\n\n");
            printf("Time to execute %d transactions: %d milliseconds, %d conflicts.\n", transaction_count, (int)completion_time, transaction_conflicts);
            printf("Max/Min memory usage: %ld/%ld bytes\n", (long)max_memory, (long)min_memory);
            /*            printf("starttime %d endtime %d, %d transactions in %d msec\n",start_time, end_time, transaction_count, completion_time); */
            printf("Transaction rate: %f txn/msec.\n", (double)transaction_count / completion_time);
            transaction_count = 0;
            transaction_conflicts = 0;
        }
        cs->unlock(cs);
    }

    void incrementTransactionCount(int n_conflicts)
    {
        cs->lock(cs);
        transaction_count++;
        transaction_conflicts += n_conflicts;
        cs->unlock(cs);
    }


    char *makeString(char ch, int n)
    {
        char *arr = new char[n + 1];
        arr[n] = '\0';
        while (--n >= 0)
        {
            arr[n] = ch;
        }
        return arr;
    }

    char const *branchesFiller = makeString(' ', (100-16) / 2);
    char const *tellersFiller = makeString(' ', (100-20) / 2);
    char const *accountsFiller = makeString(' ', (100-20) / 2);
    char const *historyFiller = makeString(' ', (50-32) / 2);

    /*
     *  createDatabase() - Creates and Initializes a scaled database.    
     */
    void createDatabase()
    {
        int i;
        /* prime database using TPC BM B scaling rules.
         *  Note that for each branch and teller:
         *      branch_id = teller_id  / ntellers
         *      branch_id = account_id / naccounts
         */
        for (i = 0; i < nbranches *tps; i++)
        {
            engine->executeStatement("INSERT INTO branches(Bid,Bbalance,filler) VALUES (%i,0,%s)", i, branchesFiller);
        }
        for (i = 0; i < ntellers *tps; i++)
        {
            engine->executeStatement("INSERT INTO tellers(Tid,Bid,Tbalance,filler) VALUES (%i,%i,0,%s)", i, i / ntellers,
                tellersFiller);
        }
        for (i = 0; i < naccounts *tps; i++)
        {
            engine->executeStatement("INSERT INTO accounts(Aid,Bid,Abalance,filler) VALUES (%i,%i,0,%s)", i, i / naccounts,
                accountsFiller);
        }
    } /* end of CreateDatabase    */


    int getRandomInt(int n)
    {
        return (int)((double)rand() / RAND_MAX * n);
    }

    void clientThread(struct tag_sample_task* p)
    {
        McoSqlSession session(engine);
        int bid, tid, aid, delta;
        int accountsPerBranch = naccounts/nbranches;

        session.setTransactionPolicy(MCO_COMMIT_BUFFERED);

        PreparedStatement stmt[5];
        session.prepare(stmt[0], "UPDATE accounts SET Abalance=Abalance+%*i WHERE Aid=%*i", &delta, &aid);
        session.prepare(stmt[1], "SELECT Abalance FROM  accounts WHERE Aid=%*i", &aid);
        session.prepare(stmt[2], "UPDATE tellers SET Tbalance=Tbalance+%*i WHERE Tid=%*i", &delta, &tid);
        session.prepare(stmt[3], "UPDATE branches SET Bbalance=Bbalance+%*i WHERE Bid=%*i", &delta, &bid);
        session.prepare(stmt[4], 
            "INSERT INTO account_history(Tid, Bid, Aid, delta, htime, filler) VALUES (%*i,%*i,%*i,%*i,now,%s)", &tid, &bid,
            &aid, &delta, historyFiller);

        for (int i = n_txn_per_client; --i >= 0;)
        {
            int n_conflicts = 0;
            bid = getRandomInt(nbranches*tps);
            tid = getRandomInt(ntellers*tps);
            if (getRandomInt(100) < 85) {
                aid = getRandomInt(accountsPerBranch) + bid * accountsPerBranch;
            } else {
                aid = getRandomInt(naccounts*tps);
            }
            delta = getRandomInt(1000);

#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
            try {
                session.executePreparedStatement(stmt[0]);
            } catch (TransactionConflict&) {
                n_conflicts += 1;
            }
#else
            session.executePreparedStatement(stmt[0]);
#endif
            DataSource *result = session.executePreparedQuery(stmt[1]);
            Cursor *cursor = result->records();
            int aBalance = 0;
            #ifdef _VXWORKS
                sample_sleep(0);
            #endif 

            while (cursor->hasNext())
            {
                aBalance = (int)cursor->next()->get(0)->intValue();
            }

            result->release();
#if !defined (MCO_CFG_USE_EXCEPTIONS) || MCO_CFG_USE_EXCEPTIONS != 0
            try {
                session.executePreparedStatement(stmt[2]);
            } catch (TransactionConflict&) {
                n_conflicts += 1;
            }
            try {
                session.executePreparedStatement(stmt[3]);
            } catch (TransactionConflict&) {
                n_conflicts += 1;
            }
            try {
                session.executePreparedStatement(stmt[4]);
            } catch (TransactionConflict&) {
                n_conflicts += 1;
            }
#else
            session.executePreparedStatement(stmt[2]);
            session.executePreparedStatement(stmt[3]);
            session.executePreparedStatement(stmt[4]);
#endif
            incrementTransactionCount(n_conflicts);
            #ifdef _VXWORKS
                sample_sleep(0);
            #endif 
        }
        reportDone();
    }


    void startBenchmark()
    {
        int i, clients = n_clients;

        printf("Initializing dataset...\n");
        createDatabase();
        printf("done.\n\n");

        printf("* Starting Benchmark Run *\n");

        sample_sleep(30); /* for vxworks, delay for output */

        start_time = sample_msec();

        sample_task_t *threads = new sample_task_t[n_clients];

        for (i = 0; i < clients; i++)
        {
            sample_start_task(&threads[i], clientThread, NULL);
            #ifdef _VXWORKS
                sample_sleep(50);
            #endif 
        }

        while (true)
        {
            bool stop = false;
            cs->lock(cs);
            stop = (n_clients == 0);
            cs->unlock(cs);

            if (stop)
                break;

            size_t used = engine->getAllocator()->allocated;
            if (used < min_memory)
            {
                min_memory = used;
            }
            else if (used > max_memory)
            {
                max_memory = used;
            }
            sample_sleep(100);
        }

        for (i = 0; i < clients; i++)
        {
            sample_join_task(&threads[i]);
        }

    }

#endif 

void run(int pass)
{
    #ifdef DISK_DATABASE
        McoSqlOpenParameters params;
        params.databaseName = "tpcdb";
        params.dictionary = tpcdb_get_dictionary();
        params.mainMemoryDatabaseSize = MEMORY_DATABASE_SIZE;
        params.mainMemoryPageSize = DB_PAGE_SIZE;
        params.mainMemoryDatabaseAddress = MAP_ADDRESS;
        params.diskDatabaseFile = FILE_PREFIX"tpcdb.dbs";
        params.diskDatabaseLogFile = FILE_PREFIX"tpcdb.log";
        params.diskCacheSize = DISK_CACHE_SIZE;
        engine->open(params);
        engine->setTransactionPolicy(MCO_COMMIT_BUFFERED);
    #else 
        engine->open("tpcdb", tpcdb_get_dictionary(), MEMORY_DATABASE_SIZE, DB_PAGE_SIZE, MAP_ADDRESS);
    #endif 

    /* Obtain database connection handle for using with database C API */
    mco_db_h con = engine->getHandle();

    /* setup and run transaction logging */
    mco_TL_start_data_t tl_parms;
    tl_parms.flags = 0;
    tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;

    char tl_log[64];
    sprintf(tl_log, "tpc_log_%d_tl.log", pass);

    CHECK(mco_translog_start( con, tl_log, &tl_parms ));

    startBenchmark();

    /* stop transaction logging */
    CHECK(mco_translog_stop( con ));

    engine->close();
}

int main(int argc, char *argv[])
{

    if (!parse_cmd(argc, argv))
    {
        printf("usage: tpc [-tpc n] [-clients n]\n");
        printf("\t-tpc\ttransactions per client\n");
        printf("\t-clients\tnumber of simultaneous clients\n");
        printf("\t-repeats\tnumber of test repeats\n");
        return 1;
    }

#ifdef DISK_DATABASE
    unlink(FILE_PREFIX"tpcdb.dbs");
    unlink(FILE_PREFIX"tpcdb.log");
#endif

    printf("*********************************************************\n");
    printf("* TPC-B Bench v1.0                                      *\n");
    printf("*********************************************************\n");
    printf("Number of clients: %d\n", numclients);
    printf("Number of transactions per client: %d\n", n_txn_per_client);
    printf("Number of repeats: %d\n", numreps);

    sample_sleep(100); /* for vxworks, delay for output */

    /* Initialize Transactio Logging */
    mco_translog_init();

    engine = new McoMultithreadedSqlEngine();
    #ifdef EXTREMEDB_LICENSE_KEY
    engine->license(EXTREMEDB_LICENSE_KEY);
    #endif

    cs = mco_create_mutex();

    for (int j = 0; j < numreps; j++)
    {
        n_clients = numclients;
        run(j);
    }

    delete engine;

    if (numclients > 1)
    {
        #if (defined(_VXWORKS) && defined(_RTP))
            printf("\nProgram complete\n");
        #else
            printf("\nPress Enter to exit");
            getchar();
        #endif
    }

    cs->destroy(cs);

    return 0;
}
