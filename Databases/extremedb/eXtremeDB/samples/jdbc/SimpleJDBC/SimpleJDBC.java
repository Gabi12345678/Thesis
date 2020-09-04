import java.sql.*;

/**
 * This examples the basic components of the JDBC driver, and shows
 * how even the simplest of queries can be implemented.
 * 
 * To  run  this  example you  need  to  specify the SQL server utility:
 * "$MCO_ROOT/target/bin/xsql -c $MCO_ROOT/samples/jdbc/SimpleJDBC/xsql.cfg"
 */

public class SimpleJDBC
{
    Connection con; // The connection to the database

    static final int nRecords = 10;

    void insert() throws SQLException {
        con.setAutoCommit(false);
        PreparedStatement stmt = con.prepareStatement("insert into S (sid,sname,status) values (?,?,?)");
        for (int i = 0; i < nRecords; i++) {
            stmt.setString(1, "S" + i);
            stmt.setString(2, "SN" + i);
            stmt.setInt(3, 1);
            stmt.execute();
        }
        stmt.close();
        con.commit();
    }

    void select() throws SQLException {
        con.setAutoCommit(true);
        PreparedStatement pstmt = con.prepareStatement("select sid,sname,status from S where sid=?");
        System.out.println("SID\tSNAME\tStatus");
        for (int i = 0; i < nRecords; i++) {
            pstmt.setString(1, "S" + i);
            ResultSet result = pstmt.executeQuery();
            while (result.next()) {
                System.out.println(result.getString(1) + "\t" + result.getString(2) + "\t" + result.getInt(3));
            }
        }
        pstmt.close();
        System.out.println("--------------");
    }

    void update() throws SQLException {
        PreparedStatement pstmt = con.prepareStatement("update S set status=? where sid=?");
        for (int i = 0; i < nRecords; i++) {
            pstmt.setInt(1, i);
            pstmt.setString(2, "S" + i);
            pstmt.execute();
        }
        pstmt.close();

        System.out.println("SID\tStatus\n");
        Statement stmt = con.createStatement();
        ResultSet result = stmt.executeQuery("select sid,status from S order by sid desc");
        while (result.next()) {
            System.out.println(result.getString(1) + "\t" + result.getInt(2));
        }
        stmt.close();
    }

    void delete() throws SQLException {
        Statement stmt = con.createStatement();
        int nDeleted = stmt.executeUpdate("delete from S");
        System.out.println("Remove " + nDeleted + " records");
    }

    public SimpleJDBC(String url) throws ClassNotFoundException, SQLException
    {
        String user = null;
        String password = null;

        // Load the driver
        Class.forName("com.mcobject.jdbc.McoDriver");

        // Connect to the database
        System.out.println("Connecting to Database URL = " + url);
        con = DriverManager.getConnection(url, user, password);
        
        System.out.println("Connected...");

        insert();

        select();

        update();

        delete();

        // Finally close the database
        System.out.println("Now closing the connection");
        con.close();
    }
  
    /**
     * This little lot starts the sample
     */
    public static void main(String[] args) throws Exception 
    {
        new SimpleJDBC("jdbc:extremedb:localhost");
    }
}
