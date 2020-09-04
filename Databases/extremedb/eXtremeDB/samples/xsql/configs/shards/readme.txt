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
