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
