TL

This configuration demonstrates an in-memory database with statically
defined dictionary and with transaction logging (TL) enabled. The log
is written to a file while previous state of database is stored in a
DB snapshot file.

At each start xsql utility loads a DB snapshot specified by configuration
parameter 'snapshot_path' if this file exists. Then apply a log file specified by
parameter 'log_path' if it exists too. Then xsql saves a new snapshot file and
start logging. All further operations with a database made by user are stored
in a log file.

As result at each run the xsql utility creates a pair of files 'snapshot' and
'log' those could be loaded at next start to restore previous state of a database.

It is additionally possible to make snapshots of the current database state manually.
In this case log file will be truncated and logging restarted since the latest snapshot.

To start the server first time with an interactive console, type:
  cd TL_DDL
  ../../../../target/bin/xsql -c xsql.cfg -f init.sql -i
  ...
  XSQL>select * from Employee;

To start the server next times with an interactive console, type:
  cd TL_DDL
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;
A database content will be restored from files 'imdb.img' and 'imdb.log' at next start.

To make a TL snapshot, type:
  XSQL>select tl_save('imdb.img');

