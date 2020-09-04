import com.mcobject.extremedb.*;

import java.io.Console;
public class RemoteSql
{
    public static void main(String[] args)
    {
        String host = "localhost";
        int port = 5001;
        int maxAttempts = 10;
        int config = Database.MCO_CFG_SQL_SUPPORT;
        boolean arg_host = false;
        boolean arg_port = false;
        boolean arg_attempts = false;
        int rows, columns;
        boolean test_failed = false;

        // Get config parameters from the command line
        for (String s : args) {
            if (arg_host) {
                host = s;
                arg_host = false;
                continue;
            } else if (arg_port) {
                port = Integer.parseInt(s);
                arg_port = false;
                continue;
            } else if (arg_attempts) {
                maxAttempts = Integer.parseInt(s);
                arg_attempts = false;
                continue;
            } else {
                if ("debug".equals(s))  config |= Database.MCO_CFG_DEBUG_LIBRARY;
                if ("host".equals(s))  arg_host = true;
                if ("port".equals(s))  arg_port = true;
                if ("attempts".equals(s))  arg_attempts = true;
            }
        }

        Database db = new Database(config);
        SqlConnection con = new SqlRemoteConnection(host, port, maxAttempts);

        con.executeStatement("create table MyTable (pk int, value string)");
        con.executeStatement("insert into MyTable (pk,value) values (?,?)", 2012, "Good bye");
        con.executeStatement("insert into MyTable (pk,value) values (?,?)", 2013, "Hello");

        SqlResultSet result = con.executeQuery("select pk,value from MyTable where pk>=?", 2013);
        for (String column : result.getColumnNames()) { 
            System.out.print(column + ", ");
        }
        System.out.println();
        for (SqlTuple tuple : result) {
            System.out.println(tuple.getInt(0) + ", " + tuple.get("value"));
        }
        con.disconnect();
        System.out.println("Disconnect from the database");
    }
}
