multifile

This configuration demonstrates a persistent database server using 
several disk files. The server will insert data in first file until 
its limit is reached, then to second file.

To start server with interactive console, type:
  cd multifile
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;
