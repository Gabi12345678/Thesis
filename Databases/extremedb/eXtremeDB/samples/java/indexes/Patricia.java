import com.mcobject.extremedb.*;
import java.io.Console;

@Persistent // Class will be stored in eXtremeDB database
class AreaCode
{
    @Indexable(type=Database.IndexType.Patricia) // Declare patricia index by "areaCode" field
    String areaCode;

    @Dimension(4)    
    String strAreaCode;

    public AreaCode(String areaCode, String strAreaCode) { 
        this.areaCode = areaCode;
        this.strAreaCode = strAreaCode;
    }

    AreaCode() {} // Default constructor needed for eXtremeDB
    public String toString() { return "\t<" + areaCode + "," + strAreaCode + ">"; }
}

public class Patricia
{
    static final int PAGE_SIZE = 256;
    static final int DATABASE_SIZE = 16*1024*1024;

    // Fill database
    static void createDatabase(Connection con) {
        con.startTransaction(Database.TransactionType.ReadWrite);        
        con.insert(new AreaCode("01", "ATT")); // Create AreaCode object and insert it into the eXtremeDB database
        con.insert(new AreaCode("020", "BCC"));
        con.insert(new AreaCode("025", "TNT"));
        con.insert(new AreaCode("03", "ANC"));
        con.insert(new AreaCode("0355", "NCC"));
        con.insert(new AreaCode("0355", "UDC"));
        con.insert(new AreaCode("045", "WTC"));
        con.insert(new AreaCode("0454", "FCC"));
        con.insert(new AreaCode("05", "ABB"));
        con.insert(new AreaCode("051", "DDA"));
        con.insert(new AreaCode("051", "FIY"));
        con.insert(new AreaCode("0513", "ASD"));
        con.insert(new AreaCode("0515", "ASH"));
        con.insert(new AreaCode("05183", "ASW"));
        con.insert(new AreaCode("0525", "ASP"));
        con.insert(new AreaCode("06", "YUI"));
        con.insert(new AreaCode("064", "KEW"));
        con.insert(new AreaCode("066", "WER"));
        con.insert(new AreaCode("0668", "VBR"));
        con.insert(new AreaCode("0668", "MPP"));
        con.insert(new AreaCode("0668", "FCA"));
        con.insert(new AreaCode("06689", "FCU"));
        con.insert(new AreaCode("0699", "FCH"));
        con.insert(new AreaCode("06683", "FCL"));
        con.insert(new AreaCode("06685", "FCD"));
        con.insert(new AreaCode("06687", "FCA"));
        con.insert(new AreaCode("07", "MNM"));
        con.insert(new AreaCode("07", "MGM"));
        con.insert(new AreaCode("079", "MSM"));
        con.insert(new AreaCode("0795", "EAE"));
        con.insert(new AreaCode("081", "YUI"));
        con.insert(new AreaCode("084", "WWW"));
        con.insert(new AreaCode("0849", "OLA"));
        con.insert(new AreaCode("09", "GEA"));
        con.insert(new AreaCode("099", "YTR"));
        con.insert(new AreaCode("099", "IUI"));
        con.insert(new AreaCode("0988", "JHJ"));
        con.insert(new AreaCode("0988", "DFD"));
        con.commitTransaction();
    }

    // List all objects in the database
    static void doListing(Connection con) {
        System.out.println("\tPress Enter to view the list of AreaCodes:");
        if (System.console() != null) {
            System.console().readLine();
        }

        con.startTransaction(Database.TransactionType.ReadOnly);        

        Cursor<AreaCode> cursor = new Cursor<AreaCode>(con, AreaCode.class, "areaCode");
        for (AreaCode ac : cursor) {
            System.out.println(ac.toString());
        }
        cursor.close();
        con.commitTransaction();
    }

    // Search database
    static void doSearch(Connection con, Cursor.Operation op, String strAreaCode)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);        

        Cursor<AreaCode> cursor = new Cursor<AreaCode> (con, AreaCode.class, "areaCode");
        if (cursor.search(op, strAreaCode)) {
            System.out.println("\tFound " + op + " for key " + strAreaCode);
            do {
                System.out.println(cursor.next().toString());
            } while (cursor.search(Cursor.Operation.NextMatch, strAreaCode));
        } else {
          System.out.println("\t" + op + " not found for key " + strAreaCode);
        }
        cursor.close();
        con.commitTransaction();
    }
    
    static void doGoogle(Connection con) {
        //////////////////////////////////////////////////////////////////////////
        // Exact search

        System.out.println("\n\t//////////////////////////////////////");
        System.out.println("\tPress Enter to view the Exact Match function demonstrations:");
        if (System.console() != null) {
            System.console().readLine();
        }

        doSearch(con, Cursor.Operation.ExactMatch, "01");
        doSearch(con, Cursor.Operation.ExactMatch, "055");
        doSearch(con, Cursor.Operation.ExactMatch, "5");
        doSearch(con, Cursor.Operation.ExactMatch, "05");
        doSearch(con, Cursor.Operation.ExactMatch, "06");
        doSearch(con, Cursor.Operation.ExactMatch, "07");

        //////////////////////////////////////////////////////////////////////////
        // prefixSearch

        System.out.println("\n\t//////////////////////////////////////");
        System.out.println("\tPress Enter to view the Prefix Match function demonstrations:");
        if (System.console() != null) {
            System.console().readLine();
        }

        doSearch(con, Cursor.Operation.PrefixMatch, "0190");
        doSearch(con, Cursor.Operation.PrefixMatch, "055");
        doSearch(con, Cursor.Operation.PrefixMatch, "589");
        doSearch(con, Cursor.Operation.PrefixMatch, "051");
        doSearch(con, Cursor.Operation.PrefixMatch, "0511");
        doSearch(con, Cursor.Operation.PrefixMatch, "05189");
        doSearch(con, Cursor.Operation.PrefixMatch, "0668889");
        doSearch(con, Cursor.Operation.PrefixMatch, "0698889");
        doSearch(con, Cursor.Operation.PrefixMatch, "071111111");

        //////////////////////////////////////////////////////////////////////////
        // bestSearch

        System.out.println("\n\t//////////////////////////////////////");
        System.out.println("\tPress Enter to view the Best Match function demonstrations:");
        if (System.console() != null) {
            System.console().readLine();
        }

        doSearch(con, Cursor.Operation.BestMatch, "0190");
        doSearch(con, Cursor.Operation.BestMatch, "055");
        doSearch(con, Cursor.Operation.BestMatch, "589");
        doSearch(con, Cursor.Operation.BestMatch, "051");
        doSearch(con, Cursor.Operation.BestMatch, "0511");
        doSearch(con, Cursor.Operation.BestMatch, "05189");
        doSearch(con, Cursor.Operation.BestMatch, "0668889");
        doSearch(con, Cursor.Operation.BestMatch, "0698889");
        doSearch(con, Cursor.Operation.BestMatch, "071111111");
    }
    
    public static void main(String[] args) throws Exception
    { 
        Database db = new Database();
        Database.Parameters params = new Database.Parameters();
        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{AreaCode.class};        
        // get config parameters from the command line
        for (String s : args) {
	    if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        System.out.println("Sample 'Patricia' demonstrates eXtremeDB patricia tree match functions\nusing a string key.\n");

        db.open("ptree-db", params, DATABASE_SIZE);
        db.generateMcoFile("ptree-db.mco"); // Generate database schema file
        Connection con = new Connection(db);
        
        createDatabase(con);
        doListing(con);
        doGoogle(con);

        con.disconnect();
        db.close();
    }
}
        
