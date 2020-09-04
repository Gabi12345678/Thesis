import com.mcobject.extremedb.*;

// Demonstrates basic TTL functionality.

@Persistent // class RecordTtlCount will be stored in eXtremeDB database
@TTL(maxCount=5) // delete extra objects when object count reaches 5
class RecordTtlCount
{
    @Indexable(unique=true) // create unique (tree) eXtremeDB index by "id" field
    int id; 
}

@Persistent // class RecordTtlTime will be stored in eXtremeDB database
@TTL(maxTime=750000) // delete objects older than 750 ms
class RecordTtlTime
{
    @Indexable(unique=true) // create unique (tree) eXtremeDB index by "id" field
    int id; 
}

@Persistent // class RecordTtlMixed will be stored in eXtremeDB database
@TTL(maxCount=5, maxTime=250000) // mixed TTL policy
class RecordTtlMixed
{
    @Indexable(unique=true) // create unique (tree) eXtremeDB index by "id" field
    int id; 
}

public class Ttl
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;
    static final int OBJECT_NUM = 10;

    // print out database content 
    static void printDatabase(Connection con, String header) {
        System.out.println(header);

        // open read-only transaction
        con.startTransaction(Database.TransactionType.ReadOnly);

        System.out.println("RecordTtlCount objects:");
        Cursor<RecordTtlCount> cursorTtlCount = new Cursor<RecordTtlCount>(con, RecordTtlCount.class, "id");
        for (RecordTtlCount rec : cursorTtlCount) {
            System.out.println("id=" + rec.id);
        }
        cursorTtlCount.close();
        System.out.println();

        System.out.println("RecordTtlTime objects:");
        Cursor<RecordTtlTime> cursorTtlTime = new Cursor<RecordTtlTime>(con, RecordTtlTime.class, "id");
        for (RecordTtlTime rec : cursorTtlTime) {
            System.out.println("id=" + rec.id);
        }
        cursorTtlTime.close();
        System.out.println();

        System.out.println("RecordTtlMixed objects:");
        Cursor<RecordTtlMixed> cursorTtlMixed = new Cursor<RecordTtlMixed>(con, RecordTtlMixed.class, "id");
        for (RecordTtlMixed rec : cursorTtlMixed) {
            System.out.println("id=" + rec.id);
        }
        cursorTtlMixed.close();
        System.out.println();

        con.rollbackTransaction(); // end transaction
    }

    public static void main(String[] args)
    { 
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[] { RecordTtlCount.class, RecordTtlTime.class, RecordTtlMixed.class };
        for (String s : args) {
            if (s.startsWith("license:")) params.license_key = s.substring(8);
        }
        db.open("ttl-db", params, DATABASE_SIZE);

        Connection con = new Connection(db);

        System.out.println("Inserting objects");

        ////////////////////////////////////////////////////// 
        // Insert objects 
        ////////////////////////////////////////////////////// 
        for (int i = 0; i < OBJECT_NUM; ++i) {
            con.startTransaction(Database.TransactionType.ReadWrite); // start RW transaction
            RecordTtlCount recTtlCount = new RecordTtlCount();
            recTtlCount.id = i;

            RecordTtlTime recTtlTime = new RecordTtlTime();
            recTtlTime.id = i;

            RecordTtlMixed recTtlMixed = new RecordTtlMixed();
            recTtlMixed.id = i;

            // insert objects into eXtremeDB database
            con.insert(recTtlCount);
            con.insert(recTtlTime);
            con.insert(recTtlMixed);
            con.checkpointTransaction(); // update indices before TTL policies will be applied
            con.commitTransaction(); // commit changes

            // sleep 100 ms to demonstrate TTL maxTime policy
            try { Thread.sleep(100); } catch (Exception e) {};
        }

        printDatabase(con, "Database content after insert :");

        // cleanup
        con.disconnect();
        db.close();
    }
}
    
