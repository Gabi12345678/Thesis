/*******************************************************************
 *                                                                 *
 *  tpcodbc.cpp                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
/* 
 *  This is a sample implementation of the Transaction Processing Performance 
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#ifdef _WIN32
    #include <windows.h>
#endif 
#include "sql.h"
#include "sqlext.h"

const int tps = 1; /* the tps scaling factor: here it is 1 */
const int nbranches = 1; /* number of branches in 1 tps db       */
const int ntellers = 10; /* number of tellers in  1 tps db       */
const int naccounts = 100000; /* number of accounts in 1 tps db       */

int getRandomInt(int n)
{
    return (int)((double)rand() / RAND_MAX * n);
}

static void check(SQLRETURN rc)
{
    if (rc != SQL_SUCCESS && rc != SQL_NO_DATA)
    {
        fprintf(stderr, "ODBC error %d\n", rc);
        abort();
    }
}

#define CHECK(x) check(x)

void startBenchmark(char* database, int n_transactions)
{
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLHSTMT hStmt1, hStmt2, hStmt3, hStmt4, hStmt5;
    SQLRETURN rc;
    int bid, tid, aid, delta;
    int accountsPerBranch = naccounts/nbranches;
    int aBalance;
    int i;
    char* historyFiller = "         ";
    time_t start, completion_time;

    start = time(NULL);

    CHECK(SQLAllocEnv(&hEnv));
    CHECK(SQLAllocConnect(hEnv, &hDbc));
    rc = SQLConnect(hDbc, database, 0 /* specify a DB name nature of the second argument */, NULL, 0, NULL, 0);
    if (rc != SQL_SUCCESS)
    {
        printf("Can't open database '%s', please check that sample 'tpc' has started with option '-clients 1'\n", database);
        exit(-1);
    }

    CHECK(SQLAllocStmt(hDbc, &hStmt1));
    CHECK(SQLPrepare(hStmt1, "UPDATE accounts SET Abalance=Abalance+? WHERE Aid=?", SQL_NTS));
    CHECK(SQLBindParameter(hStmt1, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &delta, 0, NULL));
    CHECK(SQLBindParameter(hStmt1, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &aid, 0, NULL));

    CHECK(SQLAllocStmt(hDbc, &hStmt2));
    CHECK(SQLPrepare(hStmt2, "SELECT Abalance FROM  accounts WHERE Aid=?", SQL_NTS));
    CHECK(SQLBindParameter(hStmt2, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &aid, 0, NULL));
    CHECK(SQLBindCol(hStmt2, 1, SQL_C_LONG, &aBalance, 0, NULL));

    CHECK(SQLAllocStmt(hDbc, &hStmt3));
    CHECK(SQLPrepare(hStmt3, "UPDATE tellers SET Tbalance=Tbalance+? WHERE Tid=?", SQL_NTS));
    CHECK(SQLBindParameter(hStmt3, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &delta, 0, NULL));
    CHECK(SQLBindParameter(hStmt3, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &tid, 0, NULL));


    CHECK(SQLAllocStmt(hDbc, &hStmt4));
    CHECK(SQLPrepare(hStmt4, "UPDATE branches SET Bbalance=Bbalance+? WHERE Bid=?", SQL_NTS));
    CHECK(SQLBindParameter(hStmt4, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &delta, 0, NULL));
    CHECK(SQLBindParameter(hStmt4, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bid, 0, NULL));


    CHECK(SQLAllocStmt(hDbc, &hStmt5));
    CHECK(SQLPrepare(hStmt5, "INSERT INTO account_history(Tid, Bid, Aid, delta, htime, filler) VALUES (?,?,?,?,now,?)",
          SQL_NTS));
    CHECK(SQLBindParameter(hStmt5, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &tid, 0, NULL));
    CHECK(SQLBindParameter(hStmt5, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &bid, 0, NULL));
    CHECK(SQLBindParameter(hStmt5, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &aid, 0, NULL));
    CHECK(SQLBindParameter(hStmt5, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &delta, 0, NULL));
    CHECK(SQLBindParameter(hStmt5, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, historyFiller, 0, NULL));

    for (i = n_transactions; --i >= 0;)
    {
        bid = getRandomInt(nbranches*tps);
        tid = getRandomInt(ntellers*tps);
        if (getRandomInt(100) < 85) {
            aid = getRandomInt(accountsPerBranch) + bid * accountsPerBranch;
        } else {
            aid = getRandomInt(naccounts*tps);
        }
        delta = getRandomInt(1000);
 
        CHECK(SQLExecute(hStmt1));
        CHECK(SQLExecute(hStmt2));
        CHECK(SQLFetch(hStmt2));
        CHECK(SQLCloseCursor(hStmt2));
        CHECK(SQLExecute(hStmt3));
        CHECK(SQLExecute(hStmt4));
        CHECK(SQLExecute(hStmt5));
    }

    CHECK(SQLFreeStmt(hStmt1, SQL_DROP));
    CHECK(SQLFreeStmt(hStmt2, SQL_DROP));
    CHECK(SQLFreeStmt(hStmt3, SQL_DROP));
    CHECK(SQLFreeStmt(hStmt4, SQL_DROP));
    CHECK(SQLFreeStmt(hStmt5, SQL_DROP));
    CHECK(SQLDisconnect(hDbc));
    CHECK(SQLFreeConnect(hDbc));
    CHECK(SQLFreeEnv(hEnv));

    completion_time = time(NULL) - start;

    printf("* Benchmark finished *\n");
    printf("\n* Benchmark Report *");
    printf("--------------------\n\n");
    printf("Time to execute %d transactions: %ld seconds.\n", n_transactions, (long)completion_time);
    printf("Transaction rate: %f txn/sec.\n", (double)n_transactions / completion_time);
}


int main(int argc, char* argv[])
{
    int n_transactions = 10000;
    char* database = "tpcdb";
    int i;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-tpc") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                n_transactions = atoi(argv[i]);
            }
        }
        else if (strcmp(argv[i], "-db") == 0)
        {
            if (i + 1 < argc)
            {
                i++;
                database = argv[i];
            }
        }
        else
        {
            printf("usage: tpcodbc [-tpc N] [-db NAME[:PORT]]\n");
            printf("\t-tpc\ttransactions per client\n");
            printf("\t-db\teXtremeDB database name to which ODBC driver will attach\n");
            return 1;
        }
    }
    printf("*********************************************************\n");
    printf("* TPC-B ODBC Bench v1.0                                 *\n");
    printf("*********************************************************\n");
    printf("Number of transactions: %d\n", n_transactions);

    startBenchmark(database, n_transactions);

    return 0;
}
