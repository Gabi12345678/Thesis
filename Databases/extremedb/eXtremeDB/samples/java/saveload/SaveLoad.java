import com.mcobject.extremedb.*;

@Persistent(list=true) // Store class in eXtremeDB database, declare list index
class Record
{
    int key;
}

public class SaveLoad
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    static final int N_RECORDS = 1000;
    static final String DBIMAGE_FILENAME = "db.img";

    public static void main(String[] args) throws Exception
    { 
		// Display program description
		System.out.println("Sample 'Saveload' demonstrates saving and loading \na binary database image to/from disk.\n");

        int i;
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        
        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{Record.class};        
        for (String s : args) {
	    if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        System.out.println("\tCreate new database");
        // Open & connect to database
        db.open("saveload-db", params, DATABASE_SIZE);

        Connection con = new Connection(db);

        // Fill database 
        System.out.println("\tInsert "+ N_RECORDS +" records ");
        for (i = 0; i < N_RECORDS; i++) {
            con.startTransaction(Database.TransactionType.ReadWrite);        
            Record rec = new Record();
            rec.key = i;
            con.insert(rec);
            con.commitTransaction();
        }

        // Save database image to file
        System.out.println("\tSave database...");
        con.saveSnapshot(DBIMAGE_FILENAME);
        con.disconnect();
        System.out.println("\tClose database\n");
        db.close();

        /* Save database image to file */
        System.out.println("\tLoad database...");
        db = new Database();
        params.databaseSnapshotFilePath = DBIMAGE_FILENAME; // Set database image filename
        db.open("saveload-db", params, DATABASE_SIZE); // Database.open will load database from the image

        con = new Connection(db);
        con.startTransaction(Database.TransactionType.ReadOnly);        
        System.out.println("\tLoaded " + con.count(Record.class) + " records");
        con.commitTransaction();
                
        con.disconnect();
        db.close();
    }
}
    
