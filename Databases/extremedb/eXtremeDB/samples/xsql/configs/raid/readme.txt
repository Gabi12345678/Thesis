raid

This configuration demonstrates a persistent database server using a
RAID-0 disk system for storage. The database is stored in files, 
which may be located on different physical devices.

To start server with interactive console, type:
  cd raid
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;
