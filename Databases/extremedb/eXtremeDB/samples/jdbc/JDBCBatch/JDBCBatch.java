import java.sql.*;

/**
 * This examples bulk operations in the JDBC driver, and shows how even
 * the such techniques can be implemented.
 * Most usual for JDBC API way is using of prepared statement batch
 * operations (addBatch / executeBatch).
 * It is also possible to pass prepared statement properties as a set of
 * arrays of those properties elements. Such arrays represent a batch of
 * prepared statement arguments. So theirs sizes must be equal one to another.
 * Arrays could be passed using setObject or setArray methods of class
 * PreparedStatement.
 * 
 * To  run  this  example you  need  to  specify the SQL server utility:
 * "$MCO_ROOT/target/bin/xsql -c $MCO_ROOT/samples/jdbc/JDBCBatch/xsql.cfg"
 */

public class JDBCBatch
{
    Connection con; // The connection to the database

    void initialize() throws SQLException {
        Statement stmt = con.createStatement();
        stmt.executeUpdate("create table if not exists foo(pk integer primary key, val bigint)");
        stmt.close();
    }

    void select() throws SQLException {
        Statement stmt = con.createStatement();
        ResultSet result = stmt.executeQuery("select * from foo");
        while (result.next()) {
            System.out.println(result.getInt(1) + "\t" + result.getLong(2) + "\t");
        }
        stmt.close();
        System.out.println("--------------");
    }

    // Usual JDBC batch operations
    void insertBatch1() throws SQLException {
        PreparedStatement stmt = con.prepareStatement("insert into foo values (?,?)");
        for (int i = 100; i <= 500; i += 100) {
            stmt.setInt(1, i);
            stmt.setLong(2, i*10);
            stmt.addBatch();
        }
        stmt.executeBatch();
        stmt.close();
    }

    // Arrays of prepared statement properties could be added via setObject method
    // Note that sizes of arrays mean size of a batch and must be equal one to another!
    void insertBatch2() throws SQLException {
        PreparedStatement stmt = con.prepareStatement("insert into foo values (?,?)");
        stmt.setObject(1, new int[]{1,2,3,4,5});
        stmt.setObject(2, new long[]{10,20,30,40,50});
        stmt.execute();
        stmt.close();
    }

    // Arrays of prepared statement properties could be added via setArray method
    // Note that sizes of arrays mean size of a batch and must be equal one to another!
    void insertBatch3() throws SQLException {
        PreparedStatement stmt = con.prepareStatement("insert into foo values (?,?)");
        stmt.setArray(1, con.createArrayOf("integer", new Object[]{10,20,30,40,50}));
        stmt.setArray(2, con.createArrayOf("bigint", new Object[]{100,200,300,400,500}));
        stmt.execute();
        stmt.close();
    }

    public JDBCBatch(String url) throws ClassNotFoundException, SQLException
    {
        String user = null;
        String password = null;

        // Load the driver
        Class.forName("com.mcobject.jdbc.McoDriver");

        // Connect to the database
        System.out.println("Connecting to Database URL = " + url);
        con = DriverManager.getConnection(url, user, password);

        System.out.println("Connected...");

        initialize();
        insertBatch1();
        insertBatch2();
        insertBatch3();
        select();

        // Finally close the database
        System.out.println("Now closing the connection");
        con.close();
    }
  
    /**
     * This little lot starts the sample
     */
    public static void main(String[] args) throws Exception 
    {
        new JDBCBatch("jdbc:extremedb:localhost");
    }
}
