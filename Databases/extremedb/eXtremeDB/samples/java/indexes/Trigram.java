import com.mcobject.extremedb.*;

@Persistent // class will be stored in eXtremeDB database
class TrigrmObj
{
    @Indexable(type=Database.IndexType.Trigram)
    String carid;
}

public class Trigram
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    public static void main(String[] args) throws Exception
    { 
        TrigrmObj obj;    
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        Cursor<TrigrmObj> cursor;
        String[] carid = { " 77a474ko", " 77u768cc", " 69a314pi", "177x278ee" };
        String[] search_pattern = { "768", " 77", "4pi", "8cc", "7u7", " 77a474ko" };

        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{TrigrmObj.class};
        // get config parameters from the command line
        for (String s : args) {
            if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        System.out.println("Sample 'Trigram' demonstrates search operations with tree indexes");

        // open and connect to database 
        db.open("trigram-db", params, DATABASE_SIZE);
        db.generateMcoFile("trigram-db.mco"); // generate database schema file

        Connection con = new Connection(db);
        // fill database
        for (String id : carid) {
            con.startTransaction(Database.TransactionType.ReadWrite);
            obj = new TrigrmObj();
            obj.carid = id;
            con.insert(obj);
            con.commitTransaction();
        }

        // do look up
        for (String ptrn : search_pattern) {
            con.startTransaction(Database.TransactionType.ReadOnly);
            cursor = new Cursor<TrigrmObj>(con, TrigrmObj.class, "carid");

            System.out.println("\nObjects with pattern (" + ptrn + "):");
            if (cursor.search(Cursor.Operation.Contains, ptrn)) {
                for (TrigrmObj o : cursor) {
                    System.out.println("\t(" + o.carid + ") ");
                }
            }
            cursor.close();

            //  close the transaction 
            con.rollbackTransaction();

            System.out.println();
        }

        // disconnect and close database, stop runtime 
        con.disconnect();
        db.close();
    }
}

