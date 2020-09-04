import exdb
import sys

is_debug = False
is_disk = False
tmgr = 'mvcc'

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
    print "%10s %20s\n---------------------------------" % ("ParamID", "Value")

    for cobj in csr:
        print "%10u %20f" % (cobj.param_id, cobj.value)
    csr.close()
    con.commit()


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

    # Initialize common and IoT runtime
    # Initialize common and IoT runtime
    exdb.init_runtime(disk=is_disk, tmgr=tmgr, shm=False, iot=True, debug=is_debug)

    # Create database
    with open('schema_device.mco') as fp:
        schema = fp.read()
    dbdict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)

    db = exdb.open_database(dbname=db_name, dictionary=dbdict, is_disk=is_disk,
                            db_segment_size=DEVICE_DATABASE_SIZE,
                            iot_agent_id=agent_id)

    with db.connect() as con:
        repl = exdb.IoTReplicator(con)

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