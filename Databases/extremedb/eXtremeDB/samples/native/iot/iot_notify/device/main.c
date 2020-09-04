#include <common.h>
#include <mcoiot.h>
#include "iotdevice.h"

const char db_name[] = "iotdevdb";
#define DEVICE_DATABASE_SIZE    5*1024*1024

void InsertSensors(mco_db_h db,  mco_datetime start_ts)
{
    Sensor obj;
    mco_trans_h t;
    mco_datetime ts;

    for (ts = start_ts; ts < start_ts + 5; ++ts) {
        CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t));
        CHECK(Sensor_new(t, &obj));
        CHECK(Sensor_ts_put(&obj, ts));
        CHECK(Sensor_sensorId_put(&obj, ts % 2));
        CHECK(Sensor_value_put(&obj, 10 + 0.0414 * ((ts + sample_msec()) % 119)));
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
        printf("\n%10s %20s\n---------------------------------\n", "ParamID", "Value");
        for (rc = mco_cursor_first(t, &csr); rc == MCO_S_OK; rc = mco_cursor_next(t, &csr)) {
            Config_fixed cobj;
            CHECK(Config_from_cursor(t, &csr, &obj));
            CHECK(Config_fixed_get(&obj, &cobj));
            printf("%10u %20f\n", cobj.param_id, cobj.value);
        }
        printf("\n");
    }
    CHECK(mco_trans_commit(t));
}

void PrintSensors(mco_db_h db)
{
    Sensor obj;
    mco_trans_h t;
    mco_cursor_t csr;
    MCO_RET rc;
    int n_recs = 0;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t));
    rc = Sensor_idx_index_cursor(t, &csr);
    if (rc == MCO_S_OK) {
        printf("\n%10s %10s %10s\n-----------------------------------\n", "SensorID", "Timestamp", "Value");
        for (rc = mco_cursor_first(t, &csr); rc == MCO_S_OK; rc = mco_cursor_next(t, &csr)) {
            Sensor_fixed cobj;
            CHECK(Sensor_from_cursor(t, &csr, &obj));
            CHECK(Sensor_fixed_get(&obj, &cobj));
            printf("%10d %10llu %10f\n", cobj.sensorId, cobj.ts, cobj.value);
            ++n_recs;
        }
    }
    printf("Number of Sensor records : %d\n", n_recs);
    CHECK(mco_trans_commit(t));
}


int iot_on_connect(mco_iot_connection_h iotc, void *context)
{
    mco_iot_connection_stat_t iotc_stat;
    char address[MCO_NET_INETADDR_STRING_LEN];

    mco_iot_conn_get_stat(iotc, &iotc_stat);
    mco_net_ntoa(&iotc_stat.inetaddr, address);

    printf("Agent %llu connected. Address %s:%d\n", mco_iot_conn_get_agent_id(iotc), address, iotc_stat.port);
    return IOT_CALLBACK_OK;
}

int iot_on_receive(mco_iot_connection_h iotc, mco_iot_read_stream_t *stream, void *context)
{
    printf("DATA received from agent %llu\n", mco_iot_conn_get_agent_id(iotc));
    return IOT_CALLBACK_OK;
}

int iot_on_ack(mco_iot_connection_h iotc, const iot_ack_t *ack, void *context)
{
    printf("ACK  received from agent %llu : %llu:%llu->%llu, ts=%llu, seq=%llu, errcode=%d (%s)\n", mco_iot_conn_get_agent_id(iotc),
           ack->sender_agent_id, ack->sender_db_id, ack->receiver_agent_id, ack->timestamp, ack->seq, ack->error_code, mco_strerror(ack->error_code));
    return IOT_CALLBACK_OK;
}

int iot_on_disconnect(mco_iot_connection_h iotc, void *context)
{
    mco_iot_connection_stat_t iotc_stat;

    mco_iot_conn_get_stat(iotc, &iotc_stat);
    printf("Agent %llu disconnected. Bytes sent %llu, recevied %llu\n", mco_iot_conn_get_agent_id(iotc),
           iotc_stat.sent_bytes, iotc_stat.recv_bytes);
    return IOT_CALLBACK_OK;
}

int iot_on_destroy(mco_iot_connection_h iotc, void *context)
{
    printf("Agent connection %llu destroyed\n", mco_iot_conn_get_agent_id(iotc));
    return IOT_CALLBACK_OK;
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
    const char *conn_string = (argc > 1) ? argv[1] : "127.0.0.1:15001";
    iot_comm_callback_t iot_callbacks = {&iot_on_connect, &iot_on_receive, &iot_on_ack, &iot_on_disconnect, &iot_on_destroy};

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
    CHECK(mco_iot_comm_register_callback(comm, &iot_callbacks, comm));

    CHECK(mco_iot_replicator_connect(repl, conn_string, 2*1000, 0));

    InsertSensors(db, 1000);
    CHECK(mco_iot_replicator_sync(repl, MCO_IOT_SERVER_AGENT_ID, MCO_IOT_SYNC_PULL | MCO_IOT_SYNC_PUSH | MCO_IOT_SYNC_WAIT));
    CHECK(mco_iot_replicator_clear(repl, MCO_IOT_SERVER_AGENT_ID));

    InsertSensors(db, 2000);
    CHECK(mco_iot_replicator_sync(repl, MCO_IOT_SERVER_AGENT_ID, MCO_IOT_SYNC_PULL | MCO_IOT_SYNC_PUSH | MCO_IOT_SYNC_WAIT));
    CHECK(mco_iot_replicator_clear(repl, MCO_IOT_SERVER_AGENT_ID));

    InsertSensors(db, 3000);
    CHECK(mco_iot_replicator_sync(repl, MCO_IOT_SERVER_AGENT_ID, MCO_IOT_SYNC_PULL | MCO_IOT_SYNC_PUSH | MCO_IOT_SYNC_WAIT));

    PrintConfig(db);
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
