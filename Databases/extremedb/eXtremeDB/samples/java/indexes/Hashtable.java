import com.mcobject.extremedb.*;

@Persistent // Class will be stored in eXtremeDB database
class Record
{
    @Indexable(type=Database.IndexType.Hashtable, unique=true, initSize=10000) // Declare unique hash index by "iIdx" field
    int iIdx;

    @Indexable(type=Database.IndexType.Hashtable, unique=false, initSize=10000) // Declare non-unique hash index by "iSeries" field
    int iSeries;
}

public class Hashtable
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    static final int SERIES_SIZE = 5;


    public static void main(String[] args) throws Exception
    { 
        int i, nRecs = 7;
        int findValue = 179;
        Cursor<Record> cursor;
        Record rec = null;
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        
        // Get config parameters from the command line
        for (String s : args) {
	    if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{Record.class};        

        System.out.println("Sample 'Hashtable' demonstrates operations on \nunique and non-unique hash indexes.\n");

        db.open("hash-db", params, DATABASE_SIZE);
        db.generateMcoFile("hash-db.mco"); // Generate database schema file

        Connection con = new Connection(db);

        // Fill database 
        System.out.println("\n\tInsert 1000 records ");
        for (i = 0; i < 1000; i++) {
            con.startTransaction(Database.TransactionType.ReadWrite);        
            rec = new Record();
            // Put unique values for index, non-unique values for series 
            rec.iIdx = i;
            rec.iSeries = (i / SERIES_SIZE) + 1;
            con.insert(rec); // Insert object to eXtremeDB database
            con.commitTransaction();
        }

        // Find a specific value in unique index 
        System.out.println("\n\n\tFind record with index == " + findValue);

        // Open a read-only transaction 
        con.startTransaction(Database.TransactionType.ReadOnly);        
        // Open the cursor 
        cursor = new Cursor<Record>(con, Record.class, "iIdx");
        // Get found record - or not
        rec = cursor.find(findValue);
        if ( rec != null )  {
            System.out.println("\tIndex " + rec.iIdx + " Series " + rec.iSeries);
        } else {
            System.out.println("\tnot found");
        }
        // Close the transaction 
        cursor.close();
        con.rollbackTransaction();

        // Show all records with specified value in non-unique index 
        System.out.println("\n\n\tSearch for records with iSeries == " + findValue);

        // Open a transaction 
        con.startTransaction(Database.TransactionType.ReadOnly);        
        cursor = new Cursor<Record>(con, Record.class, "iSeries");
        // Search for records with specified value for iSeries */
        if (cursor.search(Cursor.Operation.Equals, findValue)) {
            // Show all records in cursor 
            rec = cursor.first();
            for (i = 0; i < nRecs && rec != null; ++i) {
                System.out.println("\tIndex " + rec.iIdx + " Series " + rec.iSeries);
                rec = cursor.next();            
            }
        } else {
            System.out.println("\tno records found.");
        }
        cursor.close();
        // Close the transaction
        con.rollbackTransaction();
        
        con.disconnect();
        db.close();
    }
}
        
