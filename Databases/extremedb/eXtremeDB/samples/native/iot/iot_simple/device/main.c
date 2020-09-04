#include <common.h>
#include <mcoiot.h>
#include "iotdevice.h"

const char db_name[] = "iotdevdb";
#define DEVICE_DATABASE_SIZE    5*1024*1024

void InsertSensors(mco_db_h db)
{
    Sensor obj;
    mco_trans_h t;
    int i;

    for (i = 0; i < 10; ++i) {
        CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
        CHECK(Sensor_new(t, &obj));
        CHECK(Sensor_ts_put(&obj, 1000 + i));
        CHECK(Sensor_sensorId_put(&obj, i % 2));
        CHECK(Sensor_value_put(&obj, 1.414 * i))
        CHECK(mco_trans_commit(t));
    }
}

void PrintConfig(mco_db_h db)
{
    Config obj;
    mco_trans_h t;
    mco_cursor_t csr;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t));
    rc = Config_idx_index_cursor(t, &csr);
    if (rc == MCO_S_OK) {
        printf("%10s %20s\n---------------------------------\n", "ParamID", "Value");
        for (rc = mco_cursor_first(t, &csr); rc == MCO_S_OK; rc = mco_cursor_next(t, &csr)) {
            Config_fixed cobj;
            CHECK(Config_from_cursor(t, &csr, &obj));
            CHECK(Config_fixed_get(&obj, &cobj));
            printf("%10u %20f\n", cobj.param_id, cobj.value);
        }
    }
    CHECK(mco_trans_commit(t));
}


int main(int argc, char *argv[])
{
    mco_db_params_t db_params;
    mco_device_t dev;
    mco_db_h db;
    mco_iot_replicator_params_t repl_params;
    mco_iot_comm_params_t comm_params;
    mco_iot_replicator_h repl;
    mco_iot_comm_h comm;
    const char *conn_string = (argc > 1) ? argv[1] : "127.0.0.1:15000";

    /* Initialize common and IoT runtime */
    mco_error_set_handler(&sample_errhandler);
    mco_runtime_start();
    mco_iot_init();

    /* Create database */
    dev.type       = MCO_MEMORY_CONV;
    dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;
    dev.size       = DEVICE_DATABASE_SIZE;
    dev.dev.conv.ptr = (void*)malloc(dev.size);

    mco_db_params_init (&db_params);
    db_params.db_max_connections = 5;
    if (argc > 2) {
        db_params.iot_agent_id = atoi(argv[2]); /* Override agent_id in the schema */
    }
    CHECK(mco_db_open_dev(db_name, iotdevice_get_dictionary(), &dev, 1, &db_params));
    CHECK(mco_db_connect(db_name, &db));

    mco_iot_comm_params_init(&comm_params);
    CHECK(mco_iot_comm_create(&comm_params, &comm));
    mco_iot_replicator_params_init(&repl_params);
    CHECK(mco_iot_replicator_create(db, comm, &repl_params, &repl));

    CHECK(mco_iot_replicator_connect(repl, conn_string, 2*1000, 0));

    InsertSensors(db);

    CHECK(mco_iot_replicator_sync(repl, MCO_IOT_SERVER_AGENT_ID, MCO_IOT_SYNC_PULL | MCO_IOT_SYNC_PUSH | MCO_IOT_SYNC_WAIT));

    PrintConfig(db);

    CHECK(mco_iot_replicator_stop(repl));
    CHECK(mco_iot_comm_stop(comm));
    CHECK(mco_iot_replicator_destroy(repl));
    CHECK(mco_iot_comm_destroy(comm));
    CHECK(mco_db_disconnect(db));
    CHECK(mco_db_close(db_name));
    free(dev.dev.conv.ptr);
    mco_runtime_stop();
    return 0;
}
