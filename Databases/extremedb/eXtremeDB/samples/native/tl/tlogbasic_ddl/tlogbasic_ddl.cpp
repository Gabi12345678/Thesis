/*******************************************************************
 *                                                                 *
 *  tlogbasic_ddl.cpp                                              *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#include <common.h>
#include "mcosmp.h"
#include "mcosql.h"
#include "mcotime.h"
#include "mcolog.h"

using namespace McoSql;
const char *snapshot_file = "basicdb.img";
const char *log_file = "basicdb.log";

int main(int argc, char *argv[])
{
    McoSqlEngine engine;

    mco_translog_init();
    
    // Open DB and try to load a previously saved DB snapshot
    engine.open("basicdb", 0 /* no static dictionary */, 16 * 1024 * 1024, 256,
                    0, 0, McoSqlOpenParameters::DEFAULT_OPEN_FLAGS, snapshot_file, 100, 1000, 64 * 1024);

    FILE *f = fopen(log_file, "rb");
    if (f) {
        // Not a first run. Apply previously logged data
        fclose(f);
        f = 0;
        CHECK(mco_translog_apply(engine.getHandle(), log_file, MCO_TRANSLOG_ALL_LABELS));
    } else if ((f = fopen(snapshot_file, "rb")) == 0) {
        // First run. Create table
        engine.executeStatement("create table a (id int primary key, value string)");
        engine.executeStatement("create table b (id int primary key, value string)");
    }

    if (f) fclose(f);

    // Save current DB state to the snapshot
    engine.save(snapshot_file);

    // Start logging
    mco_TL_start_data_t tl_parms;
    tl_parms.flags = MCO_TRANSLOG_CRC;
    tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;

    CHECK(mco_translog_start(engine.getHandle(), log_file, &tl_parms));

    // Utilize database
    int count = 0;
    {
        QueryResult result(engine.executeQuery("select count(*) from a"));
        Cursor* cursor = result->records();
        if (cursor->hasNext())
            count = (int)cursor->next()->get(0)->intValue();
    }

    // Execute DDL operation
    if (count % 2 == 0)
        engine.executeStatement("alter table b rename to c");
    else
        engine.executeStatement("alter table c rename to b");

    {
        char buff[16];
        sprintf(buff, "val_%d", count);
        engine.executeStatement("insert into a (id, value) values (%i, %s)", count, buff);
    }

    printf("List of DB records:\n");
    {
        QueryResult result(engine.executeQuery("select id, value from a"));
        Cursor* cursor = result->records();
        while (cursor->hasNext()) {
            Record* rec = cursor->next();
            ValueRef ref(rec->get(1));
            printf("\tid: %d, value: %s\n", (int)rec->get(0)->intValue(), ref.as<String>()->cstr());
        }
    }

    // Close the database
    CHECK(mco_translog_stop(engine.getHandle()));
    engine.close();
    return 0;
}
