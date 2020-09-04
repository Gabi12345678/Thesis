im

This is the simplest possible configuration involving an in-memory (IM) 
database with a static dictionary defined in the xsql.cfg file. 

To start the server with an interactive console, type:

  cd im
  ../../../../target/bin/xsql -c xsql.cfg -i
  ...
  XSQL>select * from Employee;
