import com.mcobject.extremedb.*;

@Persistent // Class will be stored in eXtremeDB database
class TextObject
{
    String text;

    @Indexable(type=Database.IndexType.BTree, thick=true) // Declare inverse index for "keywords" field
    String[] keywords;

    public String toString() {
        return text;
    }

    TextObject(String txt) { 
        text = txt;
        keywords = txt.split(" ");
    }        
}

public class FullText
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    public static void main(String[] args) throws Exception
    { 
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        Cursor<TextObject> cursor;
        String values[] = {
            "one two three one",
            "abc xyz cba zyx",
            "one more abc",
            "two three",
            "xyz",
            "cba abc one"
        };

        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{TextObject.class};        
        // Get config parameters from the command line
        for (String s : args) {
            if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        System.out.println("Sample 'FullText' demonstrates search operations with full text indexes.\n");

        // Open and connect to database 
        db.open("fulltext-db", params, DATABASE_SIZE);
        db.generateMcoFile("fulltext-db.mco"); // Generate database schema file

        Connection con = new Connection(db);
        // Fill database
        for (String val : values) {
            con.startTransaction(Database.TransactionType.ReadWrite);        
            con.insert(new TextObject(val));
            con.commitTransaction();
        }

        con.startTransaction(Database.TransactionType.ReadOnly);        
        cursor = new Cursor<TextObject>(con, TextObject.class, "keywords");

        System.out.println("Perform AND full text search: ");
        if (cursor.search(Cursor.Operation.ContainsAll, (Object)new String[]{"abc", "one"})) {
            for (TextObject o : cursor) {
                System.out.println(o);
            }
        }
        
        System.out.println("Perform OR full text search: ");
        if (cursor.search(Cursor.Operation.ContainsAny, (Object)new String[]{"abc", "one"})) {
            for (TextObject o : cursor) {
                System.out.println(o);
            }
        }
        
        cursor.close();
        //  Close the transaction 
        con.rollbackTransaction();
        
        // Disconnect and close database, stop runtime 
        con.disconnect();
        db.close();
    }
}
        
