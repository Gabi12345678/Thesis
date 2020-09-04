import exdb
import sys

is_debug = False
is_disk = False
tmgr = 'mursiw'

db_name = "iotdevdb"
DEVICE_DATABASE_SIZE = 5*1024*1024


def insert_sensors(con):
    for i in xrange(10):
        con.startTransaction()
        obj = con.new("Sensor")
        obj.ts = 1000 + i
        obj.sensorId = i % 2
        obj.value = 1.414 * i
        con.commit()


def print_config(con):
    con.startTransaction()
    csr = con.cursor('Config', 'idx')
    print "%10s %20s\n%s" % ("ParamID", "Value", '-'*80)

    for cobj in csr:
        print "%10s %20s" % (cobj.param_id, cobj.value)

    csr.close()
    con.commit()


def iot_on_connect(iotc):
    print "Agent %s connected. Address %s:%s" %( iotc.agent_id, iotc.stat['address'], iotc.stat['port'])
    return exdb.IoT.CALLBACK_OK


def iot_on_receive(iotc):
    print "DATA received from agent %s" % iotc.agent_id
    return exdb.IoT.CALLBACK_OK


def iot_on_ack(iotc, ack):
    print "ACK received from agent %llu : %llu->%llu, ts=%llu, errcode=%d (%s)" % (iotc.agent_id,
           ack['sender_agent_id'], ack['receiver_agent_id'], ack['timestamp'], ack['error_code'], ack['error_str'])
    return exdb.IoT.CALLBACK_OK


def iot_on_disconnect(iotc):
    print "Agent %s disconnected. Bytes sent %s, recevied %s" % (iotc.agent_id,
           iotc.stat['sent_bytes'], iotc.stat['recv_bytes'])
    return exdb.IoT.CALLBACK_OK


def iot_on_destroy(iotc):
    print "Agent connection %s destroyed" % iotc.agent_id
    return exdb.IoT.CALLBACK_OK


def main():
    if len(sys.argv) > 1:
        conn_string = sys.argv[1]
    else:
        conn_string = "127.0.0.1:15000"

    if len(sys.argv) > 2:
        agent_id = int(sys.argv[2])
    else:
        print 'Warning: using default agent id'
        agent_id = 1000

    # Initialize common and IoT runtime */
    exdb.init_runtime(disk=is_disk, tmgr=tmgr, shm=False, iot=True, debug=is_debug)

    # Create database
    with open('schema_device.mco') as fp:
        schema = fp.read()
    dbdict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)

    db = exdb.open_database(dbname=db_name, dictionary=dbdict, is_disk=is_disk,
                            db_segment_size=DEVICE_DATABASE_SIZE, db_max_connections=5, iot_agent_id=agent_id)

    with db.connect() as con:
        comm = exdb.IoTCommunicator(on_connect=iot_on_connect, on_destroy=iot_on_destroy)

        repl = exdb.IoTReplicator(con, comm=comm)
        comm.on_recieve = iot_on_receive
        comm.on_ack = iot_on_ack
        comm.on_disconnect = iot_on_disconnect

        repl.connect(conn_string, 2*1000)

        insert_sensors(con)
        repl.sync(exdb.IoT.SERVER_AGENT_ID, exdb.IoT.SYNC_PUSH | exdb.IoT.SYNC_PULL | exdb.IoT.SYNC_WAIT)

        print_config(con)

        repl.stop()
        repl.destroy()
    db.close()
    return 0


if __name__ == '__main__':
    sys.exit(main())