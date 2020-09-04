import com.mcobject.extremedb.*;

@Persistent // Class will be stored in eXtremeDB database
class Obj
{
    @Indexable(type=Database.IndexType.BTree, unique=false) // Declare non-unique tree index by "value" field
    int value;
}

public class BTree
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    public static void main(String[] args) throws Exception
    { 
        Obj obj;    
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        Cursor<Obj> cursor;
        int values[] = { 1,2,2,3,3,3,4,4,5,5,5 };
        int search_value = 3;

        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{Obj.class};        
        // Get config parameters from the command line
        for (String s : args) {
	    if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        System.out.println("Sample 'BTree' demonstrates search operations with tree indexes.\n");

        // Open and connect to database 
        db.open("tree-db", params, DATABASE_SIZE);
        db.generateMcoFile("tree-db.mco"); // Generate database schema file

        Connection con = new Connection(db);
        // Fill database
        for (int val : values) {
            con.startTransaction(Database.TransactionType.ReadWrite);        
            obj = new Obj();
            obj.value = val;
            con.insert(obj);
            con.commitTransaction();
        }

        // Show all objects 
        con.startTransaction(Database.TransactionType.ReadOnly);        
        cursor = new Cursor<Obj>(con, Obj.class, "value");
        System.out.print("All objects             : ");
        for (Obj o : cursor) {
            System.out.print("(" + o.value + ") ");
        }

        // Show objects with value > search_value
        System.out.print("\nObjects with value >  " + search_value + " : ");
        if (cursor.search(Cursor.Operation.GreaterThan, search_value)) {
            for (Obj o : cursor) {
                System.out.print("(" + o.value + ") ");
            }
        }
        
        // Show objects with value >= search_value 
        System.out.print("\nObjects with value >= " + search_value + " : ");
        if (cursor.search(Cursor.Operation.GreaterOrEquals, search_value)) {
            for (Obj o : cursor) {
                System.out.print("(" + o.value + ") ");
            }
        }

        // Show objects with value == search_value 
        System.out.print("\nObjects with value == " + search_value + " : ");
        if (cursor.search(Cursor.Operation.Equals, search_value)) {
            for (Obj o : cursor) {
                if (o.value != search_value) {
                    break; // Stop before the object with non-equal value */
                }
                System.out.print("(" + o.value + ") ");
            }
        }

        // Show objects with value == search_value 
        System.out.print("\nObjects with value == " + search_value + " (stricted search) : ");
        if (cursor.search(Cursor.Operation.StrictEquals, search_value)) {
            for (Obj o : cursor) {
                System.out.print("(" + o.value + ") ");
            }
        }

        // Show objects with value <= search_value 
        System.out.print("\nObjects with value <= " + search_value + " : ");
        if (cursor.search(Cursor.Operation.LessOrEquals, search_value)) {
            while (cursor.hasPrev()) {
                System.out.print("(" + cursor.prev().value + ") ");
            }
        }

        // Show objects with value < search_value 
        System.out.print("\nObjects with value <  " + search_value + " : ");
        if (cursor.search(Cursor.Operation.LessThan, search_value)) {
            while (cursor.hasPrev()) {
                System.out.print("(" + cursor.prev().value + ") ");
            }
        }
        System.out.println();
        
        cursor.close();
        //  Close the transaction 
        con.rollbackTransaction();
        
        // Disconnect and close database, stop runtime 
        con.disconnect();
        db.close();
    }
}
        
