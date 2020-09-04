The subdirectories indicated below contain a collection of sample xSQL 
configuration files that demonstrate various eXtremeDB configurations:

im

This is the simplest possible configuration involving an in-memory (IM) 
database with a static dictionary defined in the xsql.cfg file. 

To start the server with an interactive console, type:

  cd im
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;
-----------------------------------------------------------------------

disk

This configuration demonstrates a simple persistent database server.

To start server with interactive console, type:
  cd disk
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;
-----------------------------------------------------------------------

raid

This configuration demonstrates a persistent database server using a
RAID-0 disk system for storage. The database is stored in files, 
which may be located on different physical devices.

To start server with interactive console, type:
  cd raid
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;
-----------------------------------------------------------------------

multifile

This configuration demonstrates a persistent database server using 
several disk files. The server will insert data in first file until 
its limit is reached, then to second file.

To start server with interactive console, type:
  cd multifile
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;
-----------------------------------------------------------------------

HA

This configuration demonstrates a High Availability configuration. It
includes master and replica nodes, both using an in-memory database. 
A static schema is declared in the config file.

To start this sample open 2 console windows:

In console 1 type:
  cd HA
  ../../../../target/bin/xsql -c xsql_master.cfg -i

In console 2 type:
  cd HA
  ../../../../target/bin/xsql -c xsql_replica.cfg -i


eXtremeDB will start the master and insert records into it. When 
the replica is attached, a copy of master database will be downloaded 
to it. After that it is possible to perform select operations from both 
master and replica consoles:
  XSQL>select * from Employee; 
-----------------------------------------------------------------------

cluster

This configuration demonstrates a simple cluster configuration with 3 
nodes using in-memory databases. The static schema is defined in the
config files.

To start this sample open 3 console windows:

In console 1 type:
  cd cluster
  ../../../../target/bin/xsql -c node1.cfg -i

In console 2 type:
  cd cluster
  ../../../../target/bin/xsql -c node2.cfg -i

In console 3 type:
  cd cluster
  ../../../../target/bin/xsql -c node3.cfg -i

Now the cluster is loaded and changes may be applied to any node. 
For example:

In console 1 type:
  XSQL>insert into Employee(...);

In console 2 type: 
  XSQL>update Employee(...) set ... where ...;

In console 3 type:
  XSQL>select * from Employee; 
-----------------------------------------------------------------------

TL

This configuration demonstrates an in-memory database with transaction 
logging (TL) enabled. The log is written to a file; it is possible to 
make snapshots of the current database state.

To start the server with an interactive console, type:
  cd TL
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;

To make a TL snapshot, type:
  XSQL>select tl_save('tl_snapshot.img');
-----------------------------------------------------------------------

perfmon

This configuration demonstrates a simple persistent database with the
performance monitor enabled.

To start the server with an interactive console, type:
  cd perfmon
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;

Now open a web browser and navigate to <Server IP>:8082 to see 
the performance monitor web interface.
-----------------------------------------------------------------------

shards

This configuration demonstrates a simple distributed in-memory database 
working with 2 shards; each shard holds only part of the database.

To start the servers open 2 console windows:

In console 1 type:
  cd shards
  ../../../../target/bin/xsql -c shard1.cfg -i

In console2 type:
  cd shards
  ../../../../target/bin/xsql -c shard2.cfg -i

To start the xSQL client open one more console window and type:
  cd shards
  ../../../../target/bin/xsql -c client.cfg -i

Now execute the query 'select * from Employee' in both servers and
the client. Note that the shards have only part of database.
-----------------------------------------------------------------------

