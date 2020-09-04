import exdb
import sys

is_debug = False
is_disk = False
tmgr = 'mvcc'

db_name = "iotdevdb"

DEVICE_DATABASE_SIZE = 5*1024*1024


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


def main():
    # Initialize common and IoT runtime
    exdb.init_runtime(disk=is_disk, tmgr=tmgr, shm=False, iot=True, debug=is_debug)

    # Create database
    with open('schema_server.mco') as fp:
        schema = fp.read()
    dbdict = exdb.load_dictionary(schema, persistent=is_disk, debug=is_debug)

    db = exdb.open_database(dbname=db_name, dictionary=dbdict, is_disk=is_disk, db_segment_size=DEVICE_DATABASE_SIZE)

    with db.connect() as con:
        comm = exdb.IoTCommunicator()
        repl = exdb.IoTReplicator(con=con, comm=comm)

        print "Start listening on the port 15000"
        comm.listen("15000")

        insert_config(con)

        raw_input("Press Enter to stop server...")
        print_sensors(con)

        repl.stop()
        comm.stop()
        repl.destroy()
        comm.destroy()

    db.close()
    return 0


if __name__ == '__main__':
    sys.exit(main())
