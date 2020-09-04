import com.mcobject.extremedb.*;

// Demonstarates basic insert, read, update and delete operations.

@Persistent // Class Record will be stored in eXtremeDB database
class Record
{
    @Indexable(unique=true) // Create unique (tree) eXtremeDB index by "id" field
    int id; 
    
    public String str;
}

public class Operations
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;
    static final int OBJECT_NUM = 5;

    // Print out database content 
    static void printDatabase(Connection con, String header) {
        System.out.println(header);

        // Open read-only transaction
        con.startTransaction(Database.TransactionType.ReadOnly);
        // Open cursor by "id" index
        Cursor<Record> cursor = new Cursor<Record>(con, Record.class, "id");
        for (Record rec : cursor) { // print out all objects
            System.out.println("id=" + rec.id + ", str=\"" + rec.str + "\"");
        }
        cursor.close(); // Close cursor
        con.rollbackTransaction(); // End transaction
        System.out.println();
    }

    public static void main(String[] args)
    { 
		// Display program description
		System.out.println("Sample 'Operations' demonstrates basic insert, update and delete operations.\n");

        Record rec;
        Cursor<Record> cursor;
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[] { Record.class };
        for (String s : args) {
	    if (s.startsWith("license:")) params.license_key = s.substring(8);
        }
        db.open("operations-db", params, DATABASE_SIZE);

        Connection con = new Connection(db);

        ////////////////////////////////////////////////////// 
        // Insert objects 
        ////////////////////////////////////////////////////// 
        for (int i = 0; i < OBJECT_NUM; ++i) {
            con.startTransaction(Database.TransactionType.ReadWrite); // start RW transaction
            rec = new Record(); // create Java object
            // fill data
            rec.id = i;
            rec.str = "String " + i;
        
            con.insert(rec); // Insert object into eXtremeDB database
            con.commitTransaction(); // Commit changes
        }
        
        printDatabase(con, "Database content after insert :");
        
        ////////////////////////////////////////////////////// 
        // Update
        ////////////////////////////////////////////////////// 
        con.startTransaction(Database.TransactionType.ReadWrite);
        // Perform simple index search: locate Record by id
        cursor = new Cursor<Record>(con, Record.class, "id");
        // Find record to update 
        rec = cursor.find(2);
        // Update object
        rec.str = "Updated string";
        cursor.update(); // Update current object (pointed by the cursor) in eXtremeDB database
        cursor.close(); // Release cursor
        con.commitTransaction(); // commit changes

        printDatabase(con, "Database content after updating record with id == 2 :");

        ////////////////////////////////////////////////////// 
        // Delete
        ////////////////////////////////////////////////////// 
        con.startTransaction(Database.TransactionType.ReadWrite);
        // Perform simple index search: locate Record by id
        cursor = new Cursor<Record>(con, Record.class, "id");
        // Find record with id == 3
        rec = cursor.find(3);
        cursor.remove(); // Remove current object (pointed by cursor) from eXtremeDB database
        cursor.close(); // Release cursor
        con.commitTransaction(); // Commit changes

        printDatabase(con, "Database content after removing record with id == 3 :");

        ////////////////////////////////////////////////////// 
        // Delete all
        ////////////////////////////////////////////////////// 
        con.startTransaction(Database.TransactionType.ReadWrite);
        con.removeAll(Record.class);
        con.commitTransaction();

        printDatabase(con, "Database content after removing all records :");

        // Cleanup
        con.disconnect();
        db.close();
    }
}
    
