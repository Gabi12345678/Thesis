import exdb
import sys

is_debug = False
is_disk = False
tmgr = 'mursiw'

db_name = "iotdevdb"
DEVICE_DATABASE_SIZE  =  5*1024*1024


def insert_config(con):

    con.startTransaction(exdb.Transaction.MCO_READ_WRITE)

    obj = con.new('Config')
    obj.agent_id = 1000 # This record will be replicated to device with ID 1000
    obj.param_id = 1
    obj.value = 3.1415

    obj = con.new('Config')
    obj.agent_id = exdb.IoT.ALL_AGENTS # This record will be replicated to all devices
    obj.param_id = 2
    obj.value = 2.7182

    con.commit()


def print_sensors(con):
    con.startTransaction()
    csr = con.cursor('Sensor', 'idx')

    print "%10s %10s %10s %10s\n---------------------------------------------" %\
          ("AgentID", "SensorID", "Timestamp", "Value")

    try:
        for obj in csr:
            print "%10s %10s %10s %10s" % (obj.agent_id, obj.sensorId, obj.ts, obj.value)
    except Exception as e:
        print e.message
    finally:
        csr.close()
        con.commit()


def iot_on_connect(iotc):
    print "Agent %s connected. Address %s:%d" % (iotc.agent_id, iotc.stat['address'], iotc.stat['port'])
    return exdb.IoT.CALLBACK_OK


def iot_on_receive(iotc):
    print "DATA received from agent %s" % iotc.agent_id
    return exdb.IoT.CALLBACK_OK


def iot_on_ack(iotc, ack):
    print "ACK received from agent %s : %s->%s, ts=%s, errcode=%s (%s)" % (iotc.agent_id,
           ack['sender_agent_id'], ack['receiver_agent_id'], ack['timestamp'], ack['error_code'], ack['error_str'])
    return exdb.IoT.CALLBACK_OK


def iot_on_disconnect(iotc):
    print "Agent %s disconnected. Bytes sent %s, recevied %s" % (
        iotc.agent_id,
        iotc.stat['sent_bytes'], iotc.stat['recv_bytes'])

    return exdb.IoT.CALLBACK_OK


def iot_on_destroy(iotc):
    print "Agent connection %s destroyed" % iotc.agent_id
    return exdb.IoT.CALLBACK_OK


def main():
    # Initialize common and IoT runtime
    exdb.init_runtime(disk=is_disk, tmgr=tmgr, shm=False, iot=True, debug=is_debug)

    # Create database
    with open('schema_server.mco') as fp:
        schema = fp.read()
    dbdict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)

    db = exdb.open_database(dbname=db_name, dictionary=dbdict, is_disk=is_disk, db_segment_size=DEVICE_DATABASE_SIZE)

    with db.connect() as con:

        comm = exdb.IoTCommunicator(on_connect = iot_on_connect, on_recieve = iot_on_receive, on_ack = iot_on_ack)

        repl = exdb.IoTReplicator(con, comm)

        comm.on_disconnect = iot_on_disconnect
        comm.on_destroy = iot_on_destroy

        print "Start listening on the port 15000"
        comm.listen("15000")

        insert_config(con)

        raw_input("Press Enter to stop server...");

        print_sensors(con)

        repl.stop()
        comm.stop()
        repl.destroy()
        comm.destroy()

    db.close()
    return 0

if __name__ == '__main__':
    sys.exit(main())