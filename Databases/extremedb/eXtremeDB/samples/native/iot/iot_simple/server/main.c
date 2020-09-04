#include <common.h>
#include <mcoiot.h>
#include "iotserver.h"

const char db_name[] = "iotdevdb";
#define DEVICE_DATABASE_SIZE    5*1024*1024


void InsertConfig(mco_db_h db)
{
    Config obj;
    mco_trans_h t;

    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));

    CHECK(Config_new(t, &obj));
    CHECK(Config_agent_id_put(&obj, 1000)); /* This record will be replicated to device with ID 1000 */
    CHECK(Config_param_id_put(&obj, 1));
    CHECK(Config_value_put(&obj, 3.1415));

    CHECK(Config_new(t, &obj));
    CHECK(Config_agent_id_put(&obj, MCO_IOT_ALL_AGENTS)); /* This record will be replicated to all devices */
    CHECK(Config_param_id_put(&obj, 2));
    CHECK(Config_value_put(&obj, 2.7182));

    CHECK(mco_trans_commit(t));
}

void PrintSensors(mco_db_h db)
{
    Sensor obj;
    mco_trans_h t;
    mco_cursor_t csr;
    MCO_RET rc;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t));
    rc = Sensor_idx_index_cursor(t, &csr);
    if (rc == MCO_S_OK) {
        printf("%10s %10s %10s %10s\n---------------------------------------------\n", "AgentID", "SensorID", "Timestamp", "Value");
        for (rc = mco_cursor_first(t, &csr); rc == MCO_S_OK; rc = mco_cursor_next(t, &csr)) {
            Sensor_fixed cobj;
            CHECK(Sensor_from_cursor(t, &csr, &obj));
            CHECK(Sensor_fixed_get(&obj, &cobj));
            printf("%10llu %10d %10llu %10f\n", cobj.agent_id, cobj.sensorId, cobj.ts, cobj.value);
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
    CHECK(mco_db_open_dev(db_name, iotserver_get_dictionary(), &dev, 1, &db_params));
    CHECK(mco_db_connect(db_name, &db));

    mco_iot_comm_params_init(&comm_params);
    CHECK(mco_iot_comm_create(&comm_params, &comm));
    mco_iot_replicator_params_init(&repl_params);
    CHECK(mco_iot_replicator_create(db, comm, &repl_params, &repl));

    printf("Start listening on the port 15000\n");
    CHECK(mco_iot_comm_listen(comm, "15000", 0));

    InsertConfig(db);

    sample_pause("Press Enter to stop server...");

    PrintSensors(db);

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
